#include "stdafx.h"
#include <afx.h>
#include <algorithm>
#include <cstdint>
#include <d3d12.h>
#include <DirectXCollision.h>
#include <DirectXCollision.inl>
#include <DirectXMath.h>
#include <dxgiformat.h>
#include <exception>
#include <list>
#include <memory>
#include <string>
#include <string.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include <windows.h>
#include <wrl/client.h>
#include "Actor.h"
#include "d3d12.h"
#include "d3dx12.h"
#include "DXImage.h"
#include "DXImageHelper.h"
#include "Light.h"
#include "Material.h"
#include "Model.h"
#include "Object.h"
#include "struct.h"
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

extern unordered_map<std::string, std::shared_ptr<Material>> MaterialsMap;


namespace
{
	const D3D12_INPUT_ELEMENT_DESC c_elementDescs[Attribute::Count] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "USES",     0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
//		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
//		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
	};
	const uint32_t c_sizeMap[] =
	{
		12, // Position
		12, // Normal
		8,  // TexCoord
//		12, // Tangent
//		12, // Bitangent
		4, // COLOR
		4,  // USES
	};
}

void Model::DebugMesh(std::string meshName)
{
	CString st;
	st.Format(_T("%s\r\nVertexCount: %d\r\n Unique Indices Count: %d\r\n"),
			  meshName.c_str(),
			  m_vertice_buffer.size(),
			  m_unique_vertex_buffer.size());
	OutputDebugString(st);
	OutputDebugString(_T("VERTEX LIST"));
	for (auto& v : m_vertice_buffer)
	{
		st.Format(_T("%11.6f %11.6f, %11.6\r\n"), v.Position.x, v.Position.y, v.Position.z);
		OutputDebugString(st);
	}
	OutputDebugString(_T("UNIQUE INDEX LIST"));
	for (auto p : m_unique_vertex_buffer)
	{
		st.Format(_T("%6d\r\n"), p);
		OutputDebugString(st);
	}

}

//struct MaterialBuffer
//{
//	XMFLOAT4 Emissive;
//	XMFLOAT4 Specular;
//	XMFLOAT4 Reflectance;
//	FLOAT SpecularPower;
//	FLOAT Transparency;
//	FLOAT Ambient;
//	FLOAT IndexOfRefraction;
//};
void DumpMaterialList(string name);

UINT Model::LoadFromObjectList() // map<string, shared_ptr<Actor>> actors)
{
	for (auto& obj : DXImage::ObjectList)
	{
		GenerateObjectMeshlets(obj.second);
	}
	// load the resulting materials 
	int material_index = 0;
	int light_index = 0;
//	DumpMaterialList("object list 1");
//	for (auto it = MaterialsMap.begin(); it != MaterialsMap.end(); ++it)
	CString st;
	for (auto& [name, mat] : MaterialsMap)
	{
//		const auto& [name, mat] = *it;
//		if (mat == nullptr)
//			continue;
		MaterialData material;
		material.Emissive = mat->Emissive();
		material.Specular = mat->Specular();
		material.Reflectance = mat->Reflectance();
		material.SpecularPower = mat->SpecularPower();
		material.Transparency = mat->Opacity();
		material.IndexOfRefraction = mat->IndexOfRefraction();
		material.Intensity = mat->Intensity();
		if (material.Transparency < 1.0f)
		{
//			st.Format(_T("object: %s Transparent: %6.4f\r\n"), name.c_str(), material.Transparency);
//			OutputDebugString(st);
		}

		m_material_buffer.push_back(material);
		material_index++;
	}

	UpdateActors(0);
	return S_OK;
}

void Model::UpdateActors(UINT64 FrameNumber)
{
	m_solidinstance_buffer.clear();
	m_transparentinstance_buffer.clear();
	m_light_buffer.clear();
	for (auto& [name, details] : DXImage::ActorList)
	{
		details->Update(FrameNumber, false);
		m_solidinstance_buffer.insert(m_solidinstance_buffer.end(), details->GetInstanceData().begin(), details->GetInstanceData().end());
	}
	for (auto& [name, details] : DXImage::ActorList)
	{
		details->Update(FrameNumber, true);
		m_transparentinstance_buffer.insert(m_transparentinstance_buffer.end(), details->GetInstanceData().begin(), details->GetInstanceData().end());
	}
	// update any changes in lighting
	int light_index = 0;
	{
		for (auto& [name, light] : DXImage::LightList)
		{
			LightData buffer;
			buffer.positionW = light->PositionWS;
			buffer.positionV = light->PositionVS;
			buffer.directionW = { 0,0,0,0 };
			buffer.directionV = { 0,0,0,0 };
			buffer.color = light->color;
			buffer.intensity = 20.0f;
			buffer.range = 100.0f;
			buffer.status = (UINT)(light->status) + GENERIC_LIGHT;

			m_light_buffer.push_back(buffer);
			light_index++;
		}
		for (auto& [name, light] : DXImage::PointLightList)
		{
			LightData buffer;
			buffer.positionW = light->PositionWS;
			buffer.positionV = light->PositionVS;
			buffer.directionW = { 0,0,0,0 };
			buffer.directionV = { 0,0,0,0 };
			buffer.color = light->color;
			buffer.intensity = 20.0f;
			buffer.range = 100.0f;
			buffer.status = (UINT)(light->status) + POINT_LIGHT;
			m_light_buffer.push_back(buffer);
			light_index++;
		}
		for (auto& [name, light] : DXImage::SpotLightList)
		{
			LightData buffer;
			buffer.positionW = light->PositionWS;
			buffer.positionV = light->PositionVS;
			buffer.directionW = light->DirectionWS;
			buffer.directionV = light->DirectionVS;
			buffer.color = light->color;
			buffer.intensity = 20.0f;
			buffer.range = 100.0f;
			buffer.status = (UINT)(light->status) + SPOT_LIGHT;
			m_light_buffer.push_back(buffer);
			light_index++;
		}
		for (auto& [name, light] : DXImage::DirectionalLightList)
		{
			LightData buffer;
			buffer.positionW = light->PositionWS;
			buffer.positionV = light->PositionVS;
			buffer.directionW = light->DirectionWS;
			buffer.directionV = light->DirectionVS;
			buffer.color = light->color;
			buffer.intensity = 20.0f;
			buffer.range = 100.0f;
			buffer.status = (UINT)(light->status) + DIRECTIONAL_LIGHT;
			m_light_buffer.push_back(buffer);
			light_index++;
		}
	}
	if (solidInstanceUpload != nullptr)
	{
		uint8_t* memory = nullptr;
		solidInstanceUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_solidinstance_buffer.data(), m_solidinstance_buffer.size() * sizeof(InstanceData));
		solidInstanceUpload->Unmap(0, nullptr);
	}
	if (transparentInstanceUpload != nullptr)
	{
		uint8_t* memory = nullptr;
		transparentInstanceUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_transparentinstance_buffer.data(), m_transparentinstance_buffer.size() * sizeof(InstanceData));
		transparentInstanceUpload->Unmap(0, nullptr);
	}

	if (lightDataUpload != nullptr)
	{
		uint8_t* memory = nullptr;
		lightDataUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_light_buffer.data(), m_light_buffer.size() * sizeof(LightData));
		lightDataUpload->Unmap(0, nullptr);
	}

}

void Model::GenerateObjectMeshlets(std::shared_ptr<Object> obj)
{
	if (obj == nullptr) return;
	// Implementation for generating meshlets for a given object
	for (auto& [name, component] : obj->GetMeshObjectList())
	{
//		DumpMaterialList(name + "(1)");
		MeshStatistics meshletEntry = CreateMeshlets(component);
//		DumpMaterialList(name + "(2)");
		if (meshletEntry.meshCount != 0)
		{
			obj->MeshDirectory()[name] = meshletEntry;
		}
	}
	obj->Clear(); // remove mesh definitions
}

const int maxPrimitivesPerMeshlet = 64;

UINT Model::findVertex(XMFLOAT3 position, XMFLOAT3 norm, XMFLOAT2 texture, UINT materialIndex)
{
	for (auto it = m_vertice_buffer.rbegin(); it != m_vertice_buffer.rend(); ++it)
	{
		auto& v = *it;
		if ((v.Position.x == position.x) and
			(v.Position.y == position.y) and
			(v.Position.z == position.z))
		{
		// if the vertex already exists, we need to average the normals
			XMFLOAT3 existingNormal = { v.Normal.x * (float)v.uses,
										v.Normal.y * (float)v.uses,
										v.Normal.z * (float)v.uses };
			v.uses++;

			v.Normal = { (existingNormal.x + norm.x) / (float)v.uses,
						 (existingNormal.y + norm.y) / (float)v.uses,
						 (existingNormal.z + norm.z) / (float)v.uses };
			UINT indx = (UINT)distance(it, m_vertice_buffer.rend()) - 1;
			return indx;
		}
	}
	uint32_t index = (uint32_t)m_vertice_buffer.size();
	VertexPositionNormalTextureMaterial v0(position, norm, texture, materialIndex);
	m_vertice_buffer.push_back(v0);
	return index;
};

INT Model::findIndex(INT index, INT startIndex)
{
	auto it = std::find(m_unique_vertex_buffer.begin() + startIndex, m_unique_vertex_buffer.end(), index);
	if (it != m_unique_vertex_buffer.end())
	{
		return (INT)distance(m_unique_vertex_buffer.begin() + startIndex, it) + 1; // going to subtract this later
	}
	INT idx = (INT)(m_unique_vertex_buffer.size() - startIndex) + 1; // going to subtract this later - used to make 0 into negative
	m_unique_vertex_buffer.push_back(index);
	return -idx;
};

extern unordered_map<std::string, std::shared_ptr<Material>> MaterialsMap;

MeshStatistics Model::CreateMeshlets(shared_ptr<MeshObject> m_object)
{
	string meshName = m_object->groupName;
	string materialName = m_object->MaterialName();
	UINT meshletOffset = (UINT)m_meshlet_buffer.size();
	UINT meshletSize = 0;
	Meshlet meshlet;
	MeshStatistics stats = { meshletOffset,0,false,{9999999,9999999,9999999},{-9999999,-9999999,-9999999} };
	shared_ptr<Material> mat = MaterialsMap[materialName];
	stats.IsTransparent = mat->IsTransparent();
	CString st;
	st.Format(_T("object: %s Material: %s\r\n"), meshName.c_str(), materialName.c_str());
	OutputDebugString(st);
	UINT materialIndex = Material::FindMaterialIndex(materialName);

	for (int i = 0; i < (int)m_object->groupStartIndex.size() - 1; i++)
	{
		uint32_t indx = m_object->groupStartIndex[i];
		uint32_t nxtindx = m_object->groupStartIndex[i + 1];
		if ((nxtindx - indx) > maxPrimitivesPerMeshlet)
		{
			m_object->groupStartIndex.insert(m_object->groupStartIndex.begin() + i + 1, indx + maxPrimitivesPerMeshlet);
			i = 0; // restart the loop to check the new groupings
		}
	}
	for (size_t i = 0; i < (m_object->groupStartIndex.size() - 1); i++)
	{
		UINT start = m_object->groupStartIndex[i];
		UINT end = m_object->groupStartIndex[i + 1];
		// reset the meshlet for each group
		// set starting offsets for vertices and indices in the meshlet
		meshlet = { 0,(UINT)m_unique_vertex_buffer.size(),0,(UINT)m_primitiveindex_buffer.size() };
		meshletSize++;
		vector<XMFLOAT3> meshBoundingVertices = {};
		for (UINT here = start; here < end; here++)
		{
			auto& idx = m_object->VertexIndex()[here];
			XMFLOAT3 v0 = m_object->Vertices()[idx[0].x];
			XMFLOAT3 v1 = m_object->Vertices()[idx[1].x];
			XMFLOAT3 v2 = m_object->Vertices()[idx[2].x];
			meshBoundingVertices.push_back(v0);
			meshBoundingVertices.push_back(v1);
			meshBoundingVertices.push_back(v2);

			stats.meshMin.x = min(stats.meshMin.x, min(v0.x, min(v1.x, v2.x)));
			stats.meshMin.y = min(stats.meshMin.y, min(v0.y, min(v1.y, v2.y)));
			stats.meshMin.z = min(stats.meshMin.z, min(v0.z, min(v1.z, v2.z)));

			stats.meshMax.x = max(stats.meshMax.x, max(v0.x, max(v1.x, v2.x)));
			stats.meshMax.y = max(stats.meshMax.y, max(v0.y, max(v1.y, v2.y)));
			stats.meshMax.z = max(stats.meshMax.z, max(v0.z, max(v1.z, v2.z)));

			XMFLOAT2 t0 = m_object->TextureList()[idx[0].y];
			XMFLOAT2 t1 = m_object->TextureList()[idx[1].y];
			XMFLOAT2 t2 = m_object->TextureList()[idx[2].y];
			XMFLOAT3 n0 = m_object->NormalList()[idx[0].z];
			XMFLOAT3 n1 = m_object->NormalList()[idx[1].z];
			XMFLOAT3 n2 = m_object->NormalList()[idx[2].z];
			UINT i0 = findVertex(v0, n0, t0, materialIndex);
			UINT i1 = findVertex(v1, n1, t1, materialIndex);
			UINT i2 = findVertex(v2, n2, t2, materialIndex);
			// these represent the indices of the vertices in the vertex buffer, 
			// so we can use them to create the index buffer for the meshlet
			INT mi0 = findIndex(i0, meshlet.VertOffset); // VertexOffset set when meshlet initialized
			INT mi1 = findIndex(i1, meshlet.VertOffset);
			INT mi2 = findIndex(i2, meshlet.VertOffset);
			if (mi0 < 0)
			{
				// if the index was not found, it will be negative, so we need to convert it to a positive index
				mi0 = -mi0;
				meshlet.VertCount++; // index added and needs to be counted
			}
			if (mi1 < 0)
			{
				// if the index was not found, it will be negative, so we need to convert it to a positive index
				mi1 = -mi1;
				meshlet.VertCount++; // index added and needs to be counted
			}
			if (mi2 < 0)
			{
				// if the index was not found, it will be negative, so we need to convert it to a positive index
				mi2 = -mi2;
				meshlet.VertCount++; // index added and needs to be counted
			}
			m_primitiveindex_buffer.push_back(PackedTriangle((UINT)mi0 - 1, (UINT)mi1 - 1, (UINT)mi2 - 1));
			meshlet.PrimCount++;
		}
		BoundingSphere BoundingSphere;
		BoundingSphere::CreateFromPoints(BoundingSphere, meshBoundingVertices.size(), meshBoundingVertices.data(), sizeof(XMFLOAT3));
		CullData cullData = { BoundingSphere.Center, BoundingSphere.Radius, (UINT)m_meshlet_buffer.size() };
		meshBoundingVertices.clear();
		m_culling_buffer.push_back(cullData);
		m_meshlet_buffer.push_back(meshlet);
	}
	stats.meshCount = meshletSize;
	return stats;
}


HRESULT Model::UploadGpuResources(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList)
{

		// Create committed D3D resources of proper sizes
	auto meshInfoDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(MeshInfo));
	auto vertexDesc = CD3DX12_RESOURCE_DESC::Buffer(m_vertice_buffer.size() * VertexStrides);
	auto meshletDesc = CD3DX12_RESOURCE_DESC::Buffer(m_meshlet_buffer.size() * sizeof(Meshlet));
	auto vertexIndexDesc = CD3DX12_RESOURCE_DESC::Buffer(m_unique_vertex_buffer.size() * sizeof(UINT));
	auto primitiveDesc = CD3DX12_RESOURCE_DESC::Buffer(m_primitiveindex_buffer.size() * sizeof(PackedTriangle));
	auto materialDesc = CD3DX12_RESOURCE_DESC::Buffer(m_material_buffer.size() * sizeof(MaterialData));
	auto cullDataDesc = CD3DX12_RESOURCE_DESC::Buffer(m_culling_buffer.size() * sizeof(CullData));
	lightDataDesc = CD3DX12_RESOURCE_DESC::Buffer(m_light_buffer.size() * sizeof(LightData));
	solidIinstanceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_solidinstance_buffer.size() * sizeof(InstanceData));

	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &meshInfoDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&MeshInfoResource)));
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&VertexResource)));
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&MeshletResource)));
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &vertexIndexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&UniqueVertexIndexResource)));
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &primitiveDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&PrimitiveIndexResource)));
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &materialDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&MaterialResource)));
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &cullDataDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&CullDataResource)));
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &lightDataDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&LightDataResource)));
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &solidIinstanceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&SolidInstanceResource)));
	if (m_transparentinstance_buffer.size() > 0)
	{
		transparentIinstanceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_transparentinstance_buffer.size() * sizeof(InstanceData));
		ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &transparentIinstanceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&TransparentInstanceResource)));
	}

	// Create upload resources
	ComPtr<ID3D12Resource>              meshInfoUpload;
	ComPtr<ID3D12Resource>				vertexUpload;
	ComPtr<ID3D12Resource>              meshletUpload;
	ComPtr<ID3D12Resource>              uniqueVertexIndexUpload;
	ComPtr<ID3D12Resource>              primitiveIndexUpload;
	ComPtr<ID3D12Resource>              materialUpload;
//		ComPtr<ID3D12Resource>              indexUpload;
	ComPtr<ID3D12Resource>              cullDataUpload;
//	ComPtr<ID3D12Resource>              lightDataUpload;
//		ComPtr<ID3D12Resource>              instanceUpload;

	auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexUpload)));
	ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&meshletUpload)));
	ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vertexIndexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uniqueVertexIndexUpload)));
	ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &primitiveDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&primitiveIndexUpload)));
	ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &meshInfoDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&meshInfoUpload)));
	ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &materialDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&materialUpload)));
	ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &cullDataDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cullDataUpload)));
	ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &lightDataDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&lightDataUpload)));
	ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &solidIinstanceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&solidInstanceUpload)));
	{
		MeshInfo info = {};
		info.IndexSize = sizeof(UINT);
		info.MeshletCount = static_cast<uint32_t>(m_meshlet_buffer.size());

		uint8_t* memory = nullptr;
		meshInfoUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, &info, sizeof(MeshInfo));
		meshInfoUpload->Unmap(0, nullptr);
	}

	{
		uint8_t* memory = nullptr;
		vertexUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_vertice_buffer.data(), m_vertice_buffer.size() * VertexStrides);
		vertexUpload->Unmap(0, nullptr);
	}

	{
		uint8_t* memory = nullptr;
		meshletUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_meshlet_buffer.data(), m_meshlet_buffer.size() * sizeof(Meshlet)); // m.Meshlets[0]));
		meshletUpload->Unmap(0, nullptr);
	}

	{
		uint8_t* memory = nullptr;
		uniqueVertexIndexUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_unique_vertex_buffer.data(), m_unique_vertex_buffer.size() * sizeof(UINT));
		uniqueVertexIndexUpload->Unmap(0, nullptr);
	}

	{
		uint8_t* memory = nullptr;
		primitiveIndexUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_primitiveindex_buffer.data(), m_primitiveindex_buffer.size() * sizeof(PackedTriangle));
		primitiveIndexUpload->Unmap(0, nullptr);
	}

	{
		uint8_t* memory = nullptr;
		materialUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_material_buffer.data(), m_material_buffer.size() * sizeof(MaterialProperties));
		materialUpload->Unmap(0, nullptr);
	}

	{
		uint8_t* memory = nullptr;
		cullDataUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_culling_buffer.data(), m_culling_buffer.size() * sizeof(CullData));
		cullDataUpload->Unmap(0, nullptr);
	}

	{
		uint8_t* memory = nullptr;
		lightDataUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_light_buffer.data(), m_light_buffer.size() * sizeof(LightData));
		lightDataUpload->Unmap(0, nullptr);
	}

	{
		uint8_t* memory = nullptr;
		solidInstanceUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
		std::memcpy(memory, m_solidinstance_buffer.data(), m_solidinstance_buffer.size() * sizeof(InstanceData));
		solidInstanceUpload->Unmap(0, nullptr);
	}
	if (m_transparentinstance_buffer.size() > 0)
	{
		ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &transparentIinstanceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&transparentInstanceUpload)));

		if (transparentInstanceUpload != nullptr)
		{
			uint8_t* memory = nullptr;
			transparentInstanceUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
			std::memcpy(memory, m_transparentinstance_buffer.data(), m_transparentinstance_buffer.size() * sizeof(InstanceData));
			transparentInstanceUpload->Unmap(0, nullptr);
		}
	}

	// Populate our command list
	cmdList->Reset(cmdAlloc, nullptr);

	D3D12_RESOURCE_BARRIER postCopyBarriers[9];

	cmdList->CopyResource(VertexResource.Get(), vertexUpload.Get());
	postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(VertexResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdList->CopyResource(MeshletResource.Get(), meshletUpload.Get());
	postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(MeshletResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdList->CopyResource(UniqueVertexIndexResource.Get(), uniqueVertexIndexUpload.Get());
	postCopyBarriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(UniqueVertexIndexResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdList->CopyResource(PrimitiveIndexResource.Get(), primitiveIndexUpload.Get());
	postCopyBarriers[3] = CD3DX12_RESOURCE_BARRIER::Transition(PrimitiveIndexResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdList->CopyResource(MeshInfoResource.Get(), meshInfoUpload.Get());
	postCopyBarriers[4] = CD3DX12_RESOURCE_BARRIER::Transition(MeshInfoResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	cmdList->CopyResource(MaterialResource.Get(), materialUpload.Get());
	postCopyBarriers[5] = CD3DX12_RESOURCE_BARRIER::Transition(MaterialResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	cmdList->CopyResource(CullDataResource.Get(), cullDataUpload.Get());
	postCopyBarriers[6] = CD3DX12_RESOURCE_BARRIER::Transition(CullDataResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	cmdList->CopyResource(LightDataResource.Get(), lightDataUpload.Get());
	postCopyBarriers[7] = CD3DX12_RESOURCE_BARRIER::Transition(LightDataResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	cmdList->CopyResource(SolidInstanceResource.Get(), solidInstanceUpload.Get());
	postCopyBarriers[8] = CD3DX12_RESOURCE_BARRIER::Transition(SolidInstanceResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


	cmdList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);

	ThrowIfFailed(cmdList->Close());

	ID3D12CommandList* ppCommandLists[] = { cmdList };
	cmdQueue->ExecuteCommandLists(1, ppCommandLists);

	// Create our sync fence
	ComPtr<ID3D12Fence> fence;
	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	cmdQueue->Signal(fence.Get(), 1);

	// Wait for GPU
	if (fence->GetCompletedValue() != 1)
	{
		HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		fence->SetEventOnCompletion(1, event);

		WaitForSingleObjectEx(event, INFINITE, false);
		CloseHandle(event);
	}

	return S_OK;
}





























#if 0

HRESULT Model::LoadFromFile(const wchar_t* filename)
{
	std::ifstream stream(filename, std::ios::binary);
	if (!stream.is_open())
	{
		return E_INVALIDARG;
	}

	std::vector<MeshHeader> meshes;
	std::vector<BufferView> bufferViews;
	std::vector<Accessor> accessors;

	FileHeader header;
	stream.read(reinterpret_cast<char*>(&header), sizeof(header));

	if (header.Prolog != c_prolog)
	{
		return E_FAIL; // Incorrect file format.
	}

	if (header.Version != CURRENT_FILE_VERSION)
	{
		return E_FAIL; // Version mismatch between export and import serialization code.
	}

	// Read mesh metdata
	meshes.resize(header.MeshCount);
	stream.read(reinterpret_cast<char*>(meshes.data()), meshes.size() * sizeof(meshes[0]));

	accessors.resize(header.AccessorCount);
	stream.read(reinterpret_cast<char*>(accessors.data()), accessors.size() * sizeof(accessors[0]));

	bufferViews.resize(header.BufferViewCount);
	stream.read(reinterpret_cast<char*>(bufferViews.data()), bufferViews.size() * sizeof(bufferViews[0]));

	m_buffer.resize(header.BufferSize);
	stream.read(reinterpret_cast<char*>(m_buffer.data()), header.BufferSize);

	char eofbyte;
	stream.read(&eofbyte, 1); // Read last byte to hit the eof bit

	assert(stream.eof()); // There's a problem if we didn't completely consume the file contents.

	stream.close();

	// Populate mesh data from binary data and metadata.
	m_meshes.resize(meshes.size());
	for (uint32_t i = 0; i < static_cast<uint32_t>(meshes.size()); ++i)
	{
		auto& meshView = meshes[i];
		auto& mesh = m_meshes[i];

		// Index data
		{
			Accessor& accessor = accessors[meshView.Indices];
			BufferView& bufferView = bufferViews[accessor.BufferView];

			mesh.IndexSize = accessor.Size;
			mesh.IndexCount = accessor.Count;

			mesh.Indices = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
		}

		// Index Subset data
		{
			Accessor& accessor = accessors[meshView.IndexSubsets];
			BufferView& bufferView = bufferViews[accessor.BufferView];

			mesh.IndexSubsets = MakeSpan(reinterpret_cast<Subset*>(m_buffer.data() + bufferView.Offset), accessor.Count);
		}

		// Vertex data & layout metadata

		// Determine the number of unique Buffer Views associated with the vertex attributes & copy vertex buffers.
		std::vector<uint32_t> vbMap;

		mesh.LayoutDesc.pInputElementDescs = mesh.LayoutElems;
		mesh.LayoutDesc.NumElements = 0;

		for (uint32_t j = 0; j < Attribute::Count; ++j)
		{
			if (meshView.Attributes[j] == -1)
				continue;

			Accessor& accessor = accessors[meshView.Attributes[j]];

			auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);
			if (it != vbMap.end())
			{
				continue; // Already added - continue.
			}

			// New buffer view encountered; add to list and copy vertex data
			vbMap.push_back(accessor.BufferView);
			BufferView& bufferView = bufferViews[accessor.BufferView];

			Span<uint8_t> verts = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);

			mesh.VertexStrides.push_back(accessor.Stride);
			mesh.Vertices.push_back(verts);
			mesh.VertexCount = static_cast<uint32_t>(verts.size()) / accessor.Stride;
		}

		 // Populate the vertex buffer metadata from accessors.
		for (uint32_t j = 0; j < Attribute::Count; ++j)
		{
			if (meshView.Attributes[j] == -1)
				continue;

			Accessor& accessor = accessors[meshView.Attributes[j]];

			// Determine which vertex buffer index holds this attribute's data
			auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);

			D3D12_INPUT_ELEMENT_DESC desc = c_elementDescs[j];
			desc.InputSlot = static_cast<uint32_t>(std::distance(vbMap.begin(), it));

			mesh.LayoutElems[mesh.LayoutDesc.NumElements++] = desc;
		}

		// Meshlet data
		{
			Accessor& accessor = accessors[meshView.Meshlets];
			BufferView& bufferView = bufferViews[accessor.BufferView];

			mesh.Meshlets = MakeSpan(reinterpret_cast<Meshlet*>(m_buffer.data() + bufferView.Offset), accessor.Count);
		}

		// Meshlet Subset data
		{
			Accessor& accessor = accessors[meshView.MeshletSubsets];
			BufferView& bufferView = bufferViews[accessor.BufferView];

			mesh.MeshletSubsets = MakeSpan(reinterpret_cast<Subset*>(m_buffer.data() + bufferView.Offset), accessor.Count);
		}

		// Unique Vertex Index data
		{
			Accessor& accessor = accessors[meshView.UniqueVertexIndices];
			BufferView& bufferView = bufferViews[accessor.BufferView];

			mesh.UniqueVertexIndices = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
		}

		// Primitive Index data
		{
			Accessor& accessor = accessors[meshView.PrimitiveIndices];
			BufferView& bufferView = bufferViews[accessor.BufferView];

			mesh.PrimitiveIndices = MakeSpan(reinterpret_cast<PackedTriangle*>(m_buffer.data() + bufferView.Offset), accessor.Count);
		}

		// Cull data
		{
			Accessor& accessor = accessors[meshView.CullData];
			BufferView& bufferView = bufferViews[accessor.BufferView];

			mesh.CullingData = MakeSpan(reinterpret_cast<CullData*>(m_buffer.data() + bufferView.Offset), accessor.Count);
		}
	}

   // Build bounding spheres for each mesh
	for (uint32_t i = 0; i < static_cast<uint32_t>(m_meshes.size()); ++i)
	{
		auto& m = m_meshes[i];

		uint32_t vbIndexPos = 0;

		// Find the index of the vertex buffer of the position attribute
		for (uint32_t j = 1; j < m.LayoutDesc.NumElements; ++j)
		{
			auto& desc = m.LayoutElems[j];
			if (strcmp(desc.SemanticName, "POSITION") == 0)
			{
				vbIndexPos = j;
				break;
			}
		}

		// Find the byte offset of the position attribute with its vertex buffer
		uint32_t positionOffset = 0;

		for (uint32_t j = 0; j < m.LayoutDesc.NumElements; ++j)
		{
			auto& desc = m.LayoutElems[j];
			if (strcmp(desc.SemanticName, "POSITION") == 0)
			{
				break;
			}

			if (desc.InputSlot == vbIndexPos)
			{
				positionOffset += GetFormatSize(m.LayoutElems[j].Format);
			}
		}

		XMFLOAT3* v0 = reinterpret_cast<XMFLOAT3*>(m.Vertices[vbIndexPos].data() + positionOffset);
		uint32_t stride = m.VertexStrides[vbIndexPos];

		BoundingSphere::CreateFromPoints(m.BoundingSphere, m.VertexCount, v0, stride);

		if (i == 0)
		{
			m_boundingSphere = m.BoundingSphere;
		}
		else
		{
			BoundingSphere::CreateMerged(m_boundingSphere, m_boundingSphere, m.BoundingSphere);
		}
	}

	return S_OK;
}







UINT meshletIndex = static_cast<UINT>(m_mesh_buffer.size());
UINT indexIndex = static_cast<UINT>(m_index_buffer.size());
UINT vertexIndex = static_cast<UINT>(m_vertice_buffer.size());
UINT uniqueVertexIndex = static_cast<UINT>(m_unique_vertex_buffer.size());
UINT primitiveindexIndex = static_cast<UINT>(m_primitiveindex_buffer.size());
UINT initialVertexIndex = vertexIndex;
UINT initialPrimitiveIndexIndex = primitiveindexIndex;
UINT primitiveindex_count = 0;
UINT uniqueindex_count = 0;
UINT vertex_count = 0;
UINT index_count = 0;
UINT meshletCount = 0;
vector <uint32_t> unique_vertex_indices = {};
UINT mesh_counter = 0;
for (auto& m : meshworks)
{
	// Create a vector to hold unique vertex indices for the current mesh
	if (m->GetVertexCount() == 0 || m->GetIndexCount() == 0)
	{
		continue; // skip empty meshes
	}
	unique_vertex_indices.clear();
	unique_vertex_indices.assign(m->GetVertexCount(), 0);
	Meshlet meshlet = { m->GetVertexCount(),vertexIndex,m->GetIndexCount(),primitiveindexIndex };
		// plug the numbers into the meshlet structure and the vertex/index buffers
	m_mesh_buffer.push_back(meshlet);
	meshletCount++;
#if 0
	CString st;
	st.Format(_T("%4d Mesh: %s  v = %d  i = %d \r\n"), meshletCount, m->meshName.c_str(), m->GetVertexCount(), m->GetIndexCount());
	OutputDebugStringW(L"-----------------------------------------------\r\n");
	OutputDebugString(st);
#endif

		// expand and load the vertex and index data from the MeshWork object into the Model's Mesh structure
		// save the vertex and index data into the Model's Mesh structure
		// move the vertices
	vertexIndex = static_cast<UINT>(m_vertice_buffer.size());
	m_vertice_buffer.insert(m_vertice_buffer.end(), m->GetVertexBuffer().begin(), m->GetVertexBuffer().end());
	vertex_count += m->GetVertexCount();

	indexIndex = static_cast<UINT>(m_index_buffer.size());
	for (auto& ind : m->GetIndexBuffer())
	{
		m_index_buffer.push_back(ind);
		index_count++;
	}

	// need to compress the indices and add them to the end of the index buffer
	for (auto& idx : m->GetIndexBuffer())
	{
		PackedTriangle prim;
		prim.i0 = idx.x;
		prim.i1 = idx.y;
		prim.i2 = idx.z;
		m_primitiveindex_buffer.push_back(prim);
		primitiveindex_count++;
		unique_vertex_indices.push_back(idx.x);
		unique_vertex_indices.push_back(idx.y);
		unique_vertex_indices.push_back(idx.z);
	}
	primitiveindexIndex = static_cast<UINT>(m_primitiveindex_buffer.size());
	uniqueVertexIndex = static_cast<UINT>(m_unique_vertex_buffer.size());
	m_unique_vertex_buffer.insert(m_unique_vertex_buffer.end(), unique_vertex_indices.begin(), unique_vertex_indices.end());
	unique_vertex_indices.clear();

	mesh.Meshlets = MakeSpan<Meshlet>(m_mesh_buffer.data() + meshletIndex, /*(UINT)sizeof(Meshlet) * */ meshletCount);
	mesh.UniqueVertexIndices = MakeSpan<uint8_t>((uint8_t*)(m_unique_vertex_buffer.data() + uniqueVertexIndex), vertex_count * sizeof(uint32_t));
	mesh.PrimitiveIndices = MakeSpan<PackedTriangle>(m_primitiveindex_buffer.data() + initialPrimitiveIndexIndex, (UINT)m_primitiveindex_buffer.size()); // <<<< may have to set this to the index buffer if we want to use it for culling


	mesh.LayoutDesc.pInputElementDescs = mesh.LayoutElems;
	mesh.LayoutElems[0] = c_elementDescs[0]; //   POSITION
	mesh.LayoutElems[1] = c_elementDescs[1]; //   NORMAL;
	mesh.LayoutElems[2] = c_elementDescs[2]; //   TEXTURE
	mesh.LayoutElems[3] = c_elementDescs[3]; //   COLOR;
	mesh.LayoutElems[4] = c_elementDescs[4]; //   USES;
	mesh.LayoutDesc.NumElements = 5;

	mesh.BoundingSphere = {};
	mesh.MeshletSubsets = MakeSpan<Subset>(nullptr, 0);
	mesh.IndexSubsets = MakeSpan<Subset>(nullptr, 0);
	mesh.CullingData = MakeSpan<CullData>(m_culling_buffer.data(), (UINT)(m_culling_buffer.size()));

	m_meshes.push_back(mesh);

	m_mesh_buffer.clear();
	m_index_buffer.clear();
	m_vertice_buffer.clear();
	m_unique_vertex_buffer.clear();
	m_primitiveindex_buffer.clear();
	unique_vertex_indices.clear();
	meshletIndex = 0;
	indexIndex = 0;
	vertexIndex = 0;
	uniqueVertexIndex = 0;
	primitiveindexIndex = 0;
	initialVertexIndex = 0;
	initialPrimitiveIndexIndex = 0;
	primitiveindex_count = 0;
	uniqueindex_count = 0;
	vertex_count = 0;
	index_count = 0;
}


#endif
/*

void BuildShaderParameters()
{
	mesh.IndexSize = sizeof(UINT);
	mesh.IndexCount = m->GetIndexCount();
	uint32_t stride = sizeof(struct VertexPositionNormalTextureMaterial);
	int material_index = 0;
	for (auto it = MaterialsMap.begin(); it != MaterialsMap.end(); ++it)
	{
		const auto& [name, mat] = *it;
		mesh.materialData.push_back(mat->MaterialProperties());
#if 0
		CString st;
		st.Format(_T("%3d Material %s\r\n\tEmissive: %6.4f,%6.4f,%6.4f\r\n\tDiffuse: %6.4f,%6.4f,%6.4f\r\n\tAmbient: %6.4f,%6.4f,%6.4f\r\n"),
					 material_index, name.c_str(),
					 mat->Emissive().x, mat->Emissive().y, mat->Emissive().z,
					 mat->Diffuse().x, mat->Diffuse().y, mat->Diffuse().z,
					 mat->Ambient().x, mat->Ambient().y, mat->Ambient().z);
		OutputDebugString(st);
		material_index++;
#endif
	}

}


		mesh.UniqueVertexIndices = m->IndexBuffer();
		mesh.Meshlets = m->GetMeshletList();
		mesh.PrimitiveIndices = m->PrimitiveIndexBuffer();


		mesh.LayoutDesc.pInputElementDescs = mesh.LayoutElems;
		mesh.LayoutElems[0] = c_elementDescs[0]; //   POSITION
		mesh.LayoutElems[1] = c_elementDescs[1]; //   NORMAL;
		mesh.LayoutElems[2] = c_elementDescs[2]; //   TEXTURE
		mesh.LayoutElems[3] = c_elementDescs[3]; //   COLOR;
		mesh.LayoutElems[4] = c_elementDescs[4]; //   USES;
		mesh.LayoutDesc.NumElements = 5;



*/
#include <afxstr.h>
#include <tchar.h>
#include <cstringt.h>
#include <iterator>

extern unordered_map<std::string, std::shared_ptr<Material>> MaterialsMap;
#if 0
UINT Model::findVertex(VertexPositionNormalTextureMaterial p)
{
	for (auto it = m_vertice_buffer.rbegin(); it != m_vertice_buffer.rend(); ++it)
	{
		auto& v = *it;
		if ((v.Position.x == p.Position.x) and
			(v.Position.y == p.Position.y) and
			(v.Position.z == p.Position.z)) // and
		{
		// if the vertex already exists, we need to average the normals
			XMFLOAT3 existingNormal = { v.Normal.x * (float)v.uses,
										v.Normal.y * (float)v.uses,
										v.Normal.z * (float)v.uses };
			v.uses++;

			v.Normal = { (existingNormal.x + p.Normal.x) / (float)v.uses,
						 (existingNormal.y + p.Normal.y) / (float)v.uses,
						 (existingNormal.z + p.Normal.z) / (float)v.uses };
			UINT indx = (UINT)distance(it, m_vertice_buffer.rend()) - 1;
			return indx;
		}
	}
	uint32_t index = (uint32_t)m_vertice_buffer.size();
	m_vertice_buffer.push_back(p);
	return index;
};

shared_ptr<MeshletDirectory> Model::LoadFromMeshWorks(shared_ptr<MeshWork> m)
{
	shared_ptr<MeshletDirectory> meshletDirectory = std::make_shared<MeshletDirectory>();
	CString st;
	st.Format(_T("%s\r\n"), m->meshName.c_str());
	OutputDebugString(st);

	// move the vertices to the shader resource buffer
	// return new unique index
	for (auto& unique : m->IndexBuffer())
	{
		VertexPositionNormalTextureMaterial v = m->GetVertexBuffer()[unique];
		unique = findVertex(v);
	}
	// the vertices are loaded into the shader resource buffer
	// get location of start of unique indices in shader resource buffer
	UINT uniqueIndexOffset = (UINT)m_unique_vertex_buffer.size();
	UINT uniqueIndexCount = m->GetIndexCount();
	m_unique_vertex_buffer.insert(m_unique_vertex_buffer.end(),
								  m->IndexBuffer().data().begin(),
								  m->IndexBuffer().data().end());
	// the unique vertex indices are loaded into the shader resource buffer
	// get location of start of primitive indices in shader resource buffer
	UINT primitiveIndexOffset = (UINT)m_primitiveindex_buffer.size();
	UINT primitiveIndexCount = m->GetPrimitiveIndexCount();
	m_primitiveindex_buffer.insert(m_primitiveindex_buffer.end(),
								   m->PrimitiveIndexBuffer().data().begin(),
								   m->PrimitiveIndexBuffer().data().end());
	// the primitive indices are loaded into the shader resource buffer
	// get location of start of meshlets in shader resource buffer
	UINT meshletStart = (UINT)m_meshlet_buffer.size();
	UINT meshletCount = m->GetMeshletCount();
	m_meshlet_buffer.push_back({ uniqueIndexCount,uniqueIndexOffset,primitiveIndexCount,primitiveIndexOffset });

	Mesh mesh;
//	for (auto& m : meshworks)
	{
		mesh.clear();
		if ((m->GetVertexCount() == 0) || (m->GetIndexCount() == 0)) return nullptr; // skip empty meshes
		UINT meshID = (UINT)m_meshes.size();
		mesh.VertexStrides = (UINT)sizeof(VertexPositionNormalTextureMaterial);
		mesh.Vertices = m->GetVertexBuffer();
		mesh.VertexCount = m->GetVertexCount();

		UINT meshletPosition = (UINT)m_mesh_buffer.size();
		UINT meshletCount = m->GetMeshletCount();
//		DXImage::MeshletDirectory[]


		mesh.IndexSize = sizeof(UINT);
		mesh.IndexCount = m->GetIndexCount();
		mesh.UniqueVertexIndices = m->IndexBuffer();
		mesh.Meshlets = m->GetMeshletList();
		mesh.PrimitiveIndices = m->PrimitiveIndexBuffer();

		const XMFLOAT3* v0 = (const XMFLOAT3*)mesh.Vertices.data();
		uint32_t stride = sizeof(struct VertexPositionNormalTextureMaterial);
		BoundingSphere BoundingSphere;
		BoundingSphere::CreateFromPoints(BoundingSphere, mesh.VertexCount, v0, stride);
		CullData cullData;
		cullData.BoundingSphereCenter = BoundingSphere.Center;
		cullData.BoundingSphereRadius = BoundingSphere.Radius;
		cullData.MeshletID = meshID;
		mesh.CullingData.push_back(cullData);

		int material_index = 0;
		for (auto it = MaterialsMap.begin(); it != MaterialsMap.end(); ++it)
		{
			const auto& [name, mat] = *it;
			mesh.materialData.push_back(mat->MaterialProperties());
#if 0
			CString st;
			st.Format(_T("%3d Material %s\r\n\tEmissive: %6.4f,%6.4f,%6.4f\r\n\tDiffuse: %6.4f,%6.4f,%6.4f\r\n\tAmbient: %6.4f,%6.4f,%6.4f\r\n"),
						 material_index, name.c_str(),
						 mat->Emissive().x, mat->Emissive().y, mat->Emissive().z,
						 mat->Diffuse().x, mat->Diffuse().y, mat->Diffuse().z,
						 mat->Ambient().x, mat->Ambient().y, mat->Ambient().z);
			OutputDebugString(st);
			material_index++;
#endif
		}
		mesh.LayoutDesc.pInputElementDescs = mesh.LayoutElems;
		mesh.LayoutElems[0] = c_elementDescs[0]; //   POSITION
		mesh.LayoutElems[1] = c_elementDescs[1]; //   NORMAL;
		mesh.LayoutElems[2] = c_elementDescs[2]; //   TEXTURE
		mesh.LayoutElems[3] = c_elementDescs[3]; //   COLOR;
		mesh.LayoutElems[4] = c_elementDescs[4]; //   USES;
		mesh.LayoutDesc.NumElements = 5;
		m_meshes.push_back(mesh);
	}

	return meshletDirectory;
}
#endif
