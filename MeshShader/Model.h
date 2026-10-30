//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#pragma once
#include <cstdint>
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>
#include <wrl/client.h>
#include "d3d12.h"
#include "d3dx12.h"
#include "Material.h"
#include "Object.h"
#include "struct.h"

#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define max(a,b)            (((a) > (b)) ? (a) : (b))
struct VertexPositionNormalTextureMaterial;

struct MeshObject; // Forward declaration of MeshObject class

#define SHADER_BUFFER_SIZE 1000000

class Model
{
public:
	Model()
	{
		m_vertice_buffer.reserve(SHADER_BUFFER_SIZE);
		m_unique_vertex_buffer.reserve(SHADER_BUFFER_SIZE);
		m_primitiveindex_buffer.reserve(SHADER_BUFFER_SIZE);
		m_meshlet_buffer.reserve(SHADER_BUFFER_SIZE);
		m_culling_buffer.reserve(SHADER_BUFFER_SIZE);
		m_solidinstance_buffer.reserve(SHADER_BUFFER_SIZE / 100);
		m_material_buffer.reserve(512);
	}
	HRESULT UploadGpuResources(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);
	const DirectX::BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }
private:
	DirectX::BoundingSphere                m_boundingSphere;
	std::vector<uint8_t>                   m_buffer;
public:
	// I added this function to load meshlets from a collection of MeshWork objects
	std::vector<VertexPositionNormalTextureMaterial>		m_vertice_buffer;
	std::vector<uint32_t>									m_unique_vertex_buffer;
	std::vector<PackedTriangle>								m_primitiveindex_buffer;
	std::vector<Meshlet>									m_meshlet_buffer;
	std::vector<CullData>									m_culling_buffer;
	std::vector<MaterialData>								m_material_buffer = {};
	std::vector <InstanceData>								m_solidinstance_buffer = {};
	std::vector <InstanceData>								m_transparentinstance_buffer = {};
	std::vector < LightData>								m_light_buffer = {};
//	std::vector<MeshInfo>									m_mesh_info_buffer;
//	std::vector<XMUINT3>									m_index_buffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>				VertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>              MeshletResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>              UniqueVertexIndexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>              PrimitiveIndexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>              MeshInfoResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>              MaterialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>              CullDataResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>              LightDataResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>              SolidInstanceResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>              TransparentInstanceResource;

	UINT VertexStrides = sizeof(VertexPositionNormalTextureMaterial);
	UINT LoadFromObjectList();
	void GenerateObjectMeshlets(std::shared_ptr<Object> obj);
	MeshStatistics CreateMeshlets(std::shared_ptr<MeshObject> m_object);
	UINT findVertex(XMFLOAT3 position, XMFLOAT3 norm, XMFLOAT2 texture, UINT materialIndex);
	INT findIndex(INT index, INT startIndex);
	void DebugMesh(std::string meshName);
	void UpdateActors(UINT64 FrameNumber);
	CD3DX12_RESOURCE_DESC solidIinstanceDesc;
	ComPtr<ID3D12Resource> solidInstanceUpload;
	CD3DX12_RESOURCE_DESC transparentIinstanceDesc;
	ComPtr<ID3D12Resource> transparentInstanceUpload;
	CD3DX12_RESOURCE_DESC lightDataDesc;
	ComPtr<ID3D12Resource> lightDataUpload;

};
