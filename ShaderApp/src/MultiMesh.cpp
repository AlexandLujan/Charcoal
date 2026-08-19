#include "pch.h"
#include <afxdlgs.h>
#include <afxwin.h>
#include <CommandList.h>
#include <DirectXMath.h>
#include <dx12lib.h>
#include <fstream>
#include <iostream>
#include <Material.h>
#include <memory>
#include <Mesh.h>
#include <Scene.h>
#include <SceneNode.h>
#include <string>
#include <vector>
#include "afxdialogex.h"
#include "ColorControl.h"
#include "d3dx12.h"
#include "OBJ_3D_Model.h"
#include "Object.h"
#include "ShaderAppDlg.h"

using namespace std;
using namespace DirectX;
using namespace dx12lib;

//	VertexCollection = std::vector< VertexPositionNormalTangentBitangentTexture >;
//	IndexCollection = std::vector<UINT16>;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CShaderAppDlg* parent;

shared_ptr<SceneNode> CShaderAppDlg::CreateSceneMultipleMesh(string objectName, shared_ptr<CommandList> commandList)
{
	XMMATRIX identity = DirectX::XMMatrixIdentity();
	VertexCollection vertices;
	IndexCollection indices;
	auto findVertex = [&vertices](XMFLOAT3 position, XMFLOAT3 norm, XMFLOAT3 tx, XMFLOAT3 tan) -> uint32_t
		{
			for (int i = 0; i < (int)vertices.size(); i++)
			{
				if ((vertices[i].Position.x == position.x) and
					(vertices[i].Position.y == position.y) and
					(vertices[i].Position.z == position.z))
				{
					return i;
				}
			}
			uint32_t index = (uint32_t)vertices.size();
			VertexPositionNormalTangentBitangentTexture v0(position, norm, tx, tan);
			vertices.push_back(v0);
			return index;
		};
	shared_ptr<Object> obj_ptr = make_shared<MultiMeshObject>();
	obj_ptr = ObjectList[objectName];
	if (obj_ptr == nullptr) return nullptr; // continue;
	if (obj_ptr->Iam() != "MULTIPLE MESH") return nullptr; //continue;
	shared_ptr<MultiMeshObject> m = std::static_pointer_cast<MultiMeshObject>(obj_ptr);
	auto node = std::make_shared<SceneNode>();
	node->SetName(objectName); // name);

	for (auto& [key, mesh] : m->MeshList())
	{
		XMFLOAT4 meshColor = { 99,99,99,99 };
		string meshName = mesh->Name();
		for (size_t i = 0; i < (mesh->groupStartIndex.size() - 1); i++)
		{
			XMFLOAT4 mins = { 999999,999999,999999,0 };
			XMFLOAT4 maxs = { -999999,-999999,-999999,0 };
			UINT start = mesh->groupStartIndex[i];
			UINT end = mesh->groupStartIndex[i + 1];
			for (UINT here = start; here < end; here++)
			{
				auto& idx = mesh->VertexIndex()[here];
				UINT i0 = idx.v_index.x;
				UINT i1 = idx.v_index.y;
				UINT i2 = idx.v_index.z;
				VertexType v0 = mesh->Vertices()[i0];
				VertexType v1 = mesh->Vertices()[i1];
				VertexType v2 = mesh->Vertices()[i2];
				v0 = TranslateXMFLOAT4(v0, mesh->World(), identity, identity);
				v1 = TranslateXMFLOAT4(v1, mesh->World(), identity, identity);
				v2 = TranslateXMFLOAT4(v2, mesh->World(), identity, identity);

//				v0.first.x /= 100000.0f;
//				v0.first.y /= 100000.0f;
//				v0.first.z /= 100000.0f;
//				v1.first.x /= 100000.0f;
//				v1.first.y /= 100000.0f;
//				v1.first.z /= 100000.0f;
//				v2.first.x /= 100000.0f;
//				v2.first.y /= 100000.0f;
//				v2.first.z /= 100000.0f;

				mins.x = min(mins.x, min(v0.first.x, min(v1.first.x, v2.first.x)));
				maxs.x = max(maxs.x, max(v0.first.x, max(v1.first.x, v2.first.x)));
				mins.y = min(mins.y, min(v0.first.y, min(v1.first.y, v2.first.y)));
				maxs.y = max(maxs.y, max(v0.first.y, max(v1.first.y, v2.first.y)));
				mins.z = min(mins.z, min(v0.first.z, min(v1.first.z, v2.first.z)));
				maxs.z = max(maxs.z, max(v0.first.z, max(v1.first.z, v2.first.z)));

				XMFLOAT3 N = { 0,0,0 }; // Normal(v0.first, v1.first, v2.first); // return the normal
				XMFLOAT3 T = { 0,0,0 }; // Tangent(v0.first, v1.first); // return the normal

				XMFLOAT3 vpnt0(v0.first.x, v0.first.y, v0.first.z);
				XMFLOAT3 vpnt1(v1.first.x, v1.first.y, v1.first.z);
				XMFLOAT3 vpnt2(v2.first.x, v2.first.y, v2.first.z);
				i0 = findVertex(vpnt0, N, XMFLOAT3(0, 0, 0), T);
				i1 = findVertex(vpnt1, N, XMFLOAT3(0, 0, 0), T);
				i2 = findVertex(vpnt2, N, XMFLOAT3(0, 0, 0), T);
				indices.push_back(i0);
				indices.push_back(i1);
				indices.push_back(i2);
				if (meshColor.x == 99)
				{
					meshColor = v0.second;
				}
			}

			auto vertexBuffer = commandList->CopyVertexBuffer(vertices);
			auto indexBuffer = commandList->CopyIndexBuffer(indices);
			auto _mesh = make_shared<Mesh>(meshName);
			auto mat = Material::findMaterial("Zero");
			mat->Emissive = meshColor;
			mat->Reflectance = XMFLOAT4(0.7f, 0.5f, 0.8f, 1.0f);
			mat->Ambient = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);
			mat->Diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
			mat->Opacity = 1.0f;
			auto material = std::make_shared<Material>(*mat);

			if (abs(mins.z - maxs.z) <= 0.1f)
			{
				mins.z -= 0.1f;
				maxs.z += 0.1f;
			}
			auto v_mins = XMLoadFloat4(&mins);
			auto v_maxs = XMLoadFloat4(&maxs);
			BoundingBox aabb;
			BoundingBox::CreateFromPoints(aabb, v_mins, v_maxs);
			_mesh->SetAABB(aabb);
			_mesh->SetVertexBuffer(0, vertexBuffer);
			_mesh->SetIndexBuffer(indexBuffer);
			_mesh->SetMaterial(material);
			node->AddMesh(_mesh);
			vertices.clear();
			vertexBuffer.reset();
			indices.clear();
			indexBuffer.reset();
		}
	}

	return node;
}

extern std::string RandomColor();

RoundTunnel::RoundTunnel(std::string tunnel_name, float diameter, float length, float ringWidth, float tilesPerSegment) :
	MultiMeshObject(tunnel_name), diameter(diameter), length(length), ringWidth(ringWidth), tilesPerRing(tilesPerSegment)
{
	UINT numberOfRings = (UINT)floor(length / ringWidth); // how many rings from front to back
	UINT tileHeight = (UINT)floor((XM_2PI * diameter) / (float)tilesPerRing);
	float TileStepSizeInRadians = (XM_2PI / tilesPerRing);
	int id = 0;
	for (int i = 0; i < numberOfRings; i++)
	{
		float z = i * ringWidth;
		for (int j = 0; j < tilesPerRing; j++)
		{
			float angle = (j * TileStepSizeInRadians);
			XMFLOAT3 location = XMFLOAT3((diameter / 2.0f) * cosf(angle), (diameter / 2.0f) * sinf(angle), z);
			std::shared_ptr<MeshObject> segment = CreateBox(i, j, .002f, tileHeight * 0.50f, ringWidth * 0.9f, angle, location);
			meshList[segment->Name()] = segment;
		}
	}
	UpdateWorld();
}

shared_ptr<MeshObject> MultiMeshObject::CreateBox(int ring, int id, float width, float height, float depth, float rollAngle, XMFLOAT3 CenterPoint)
{
	XMMATRIX identity = DirectX::XMMatrixIdentity();
	XMMATRIX matRotateZ = DirectX::XMMatrixRotationZ(rollAngle);
	XMMATRIX matTranslate = DirectX::XMMatrixTranslation(CenterPoint.x, CenterPoint.y, CenterPoint.z);
//	XMMATRIX worldMatrix = matTranslate * matRotateZ;
	XMMATRIX worldMatrix = matRotateZ * matTranslate;
//	std::mt19937 gen{ static_cast<unsigned int>(std::time(0)) }; // Seed with time
	XMFLOAT4 boxColor = FindColor(RandomColor());
	float dWidth = width / 2.0f;
	float dHeight = height / 2.0f;
	float dDepth = depth / 2.0f;
	VertexType ruf = make_pair(XMFLOAT4(dWidth, dHeight, dDepth, 1), boxColor);
	VertexType rlf = make_pair(XMFLOAT4(dWidth, -dHeight, dDepth, 1), boxColor);
	VertexType luf = make_pair(XMFLOAT4(-dWidth, dHeight, dDepth, 1), boxColor);
	VertexType llf = make_pair(XMFLOAT4(-dWidth, -dHeight, dDepth, 1), boxColor);
	VertexType rur = make_pair(XMFLOAT4(dWidth, dHeight, -dDepth, 1), boxColor);
	VertexType rlr = make_pair(XMFLOAT4(dWidth, -dHeight, -dDepth, 1), boxColor);
	VertexType lur = make_pair(XMFLOAT4(-dWidth, dHeight, -dDepth, 1), boxColor);
	VertexType llr = make_pair(XMFLOAT4(-dWidth, -dHeight, -dDepth, 1), boxColor);
	vector<VertexType> vertexList = { ruf,rlf,luf,llf,rur,rlr,lur,llr };
	XMUINT3  i00 = { 3,2,0 };
	XMUINT3  i01 = { 1,3,0 };

	XMUINT3  i02 = { 7,5,4 };
	XMUINT3  i03 = { 6,7,4 };

	XMUINT3  i04 = { 6,4,0 };
	XMUINT3  i05 = { 2,6,0 };

	XMUINT3  i06 = { 7,5,1 };
	XMUINT3  i07 = { 3,7,1 };

	XMUINT3  i08 = { 3,2,6 };
	XMUINT3  i09 = { 7,3,6 };

	XMUINT3  i10 = { 4,1,0 };
	XMUINT3  i11 = { 4,5,0 };
	std::vector<vertex_index> vi = {
					vertex_index(i00,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i01,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i02,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i03,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i04,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i05,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i06,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i07,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i08,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i09,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i10,No_Index_XMUINT3,No_Index_XMUINT3),
					vertex_index(i11,No_Index_XMUINT3,No_Index_XMUINT3) };
	vertexList = TranslateXMFLOAT4(vertexList, worldMatrix, identity, identity);
	shared_ptr<MeshObject> box = make_shared<MeshObject>("tile_" + std::to_string(ring) + "_" + to_string(id), boxColor);
	box->Vertices() = vertexList;
	box->VertexIndex() = vi;
	box->groupStartIndex = { 0,(UINT)vi.size() };
	return box;
}

TriangleTunnel::TriangleTunnel(std::string tunnel_name, float base, float height, float length, float segmentWidth) :
	MultiMeshObject(tunnel_name), base(base), height(height), length(length), segmentWidth(segmentWidth)
{
	UpdateWorld();
}
