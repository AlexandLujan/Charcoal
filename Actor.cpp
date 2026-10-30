#include "stdafx.h"
#include <afx.h>
#include <cstdint>
#include <DirectXMath.h>
#include <DirectXMathConvert.inl>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <memory>
#include <utility>
#include <vector>
#include <Windows.h>
#include "Actor.h"
#include "DXImage.h"
#include "Object.h"
#include "struct.h"
#include "Utility.h"

using namespace DirectX;
using namespace std;

Actor::Actor(std::string name, std::string objectName, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, std::string materialname)
	: name(name),
	objectName(objectName),
	position(position),
	pitch(DegreesToRadians(rotation.x)),
	yaw(DegreesToRadians(rotation.y)),
	roll(DegreesToRadians(rotation.z)),
	scaleX(scale.x),
	scaleY(scale.y),
	scaleZ(scale.z)
{
	assembly = DXImage::ObjectList[objectName];
	worldMatrix = DirectX::XMMatrixIdentity();
	UpdateWorld();
//		material = Material::findMaterial(materialname);
//		CreateMeshlets();
}

shared_ptr<Object> Actor::GetAssembly()
{
	return DXImage::ObjectList[objectName];
}
struct InstanceDatax
{

};

void Actor::Update(uint64_t frameNumber, BOOL handleTransparencies = FALSE)
{
	instanceData.clear();
	shared_ptr<Object> obj = GetAssembly();
	if (obj == nullptr) return;
	for (auto& [key, info] : obj->MeshDirectory())
	{
		if (info.IsTransparent != handleTransparencies)
			continue;
		UINT meshOffset = info.meshOffset;
		UINT meshCount = info.meshCount;
		XMMATRIX t = AdjustMesh(frameNumber, key);
		XMMATRIX w = XMMatrixTranspose(t * worldMatrix);
		InstanceData d = { 0, meshOffset, meshCount, scaleX, w };
		instanceData.push_back(d);
	}

}

void Actor::Clear()
{
	ObjectName() = "";
	position = { 0,0,0 };
	pitch = 0.0f;
	yaw = 0.0f;
	roll = 0.0f;
	scaleX = 1.0f;
	scaleY = 1.0f;
	scaleZ = 1.0f;
	worldMatrix = DirectX::XMMatrixIdentity();
}


XMFLOAT3 TranslateToWorld(XMFLOAT3 pt, XMMATRIX& world)
{
	XMFLOAT3 results;
	XMVECTOR v = XMLoadFloat3(&pt);
	v = XMVector3Transform(v, world);
	XMStoreFloat3(&results, v);
	return results;
}

#ifdef DENNIS
#include <set>

struct Triangle
{
	UINT id;
	bool doneFlag = false;
	std::set<UINT> one_vertex_neighbor;
	std::set<UINT> two_vertex_neighbor;
	XMUINT4 i0;
	XMUINT4 i1;
	XMUINT4 i2;
	UINT& I0() { return i0.x; }
	UINT& I1() { return i1.x; }
	UINT& I2() { return i2.x; }

	XMFLOAT3 v0 = { 0,0,0 };
	XMFLOAT3 v1 = { 0,0,0 };
	XMFLOAT3 v2 = { 0,0,0 };
	// x is vertex index
	// y is texture
	// z is normal
	// w is material
	Triangle(UINT idNumber = 0, vector<XMUINT4> idx = { }) : id(idNumber), i0(idx[0]), i1(idx[1]), i2(idx[2])
	{
		if (idx.size() == 3)
		{
			v0 = vertexList[I0()];
			v1 = vertexList[I1()];
			v2 = vertexList[I2()];
		}
		one_vertex_neighbor.clear();
		two_vertex_neighbor.clear();
	}
	bool AddNeighbor(UINT neighborId, int v_index)
	{
		if (neighborId == id) return false; // don't add self as neighbor
		if ((v_index == I0()) or (v_index == I1()) or (v_index == I2()))
		{
			if (one_vertex_neighbor.contains(neighborId))
			{
				// This neighbor is already in the one_vertex_neighbor set, 
				// so we need to move it to the two_vertex_neighbor set.
				one_vertex_neighbor.erase(neighborId);
				two_vertex_neighbor.insert(neighborId);
			}
			else
			{
#if 1
				CString st;
				st.Format(_T("%6d add %6d index %6d matching %6d %6d %6d one visit = %d\r\n"), id, neighborId, v_index, I0(), I1(), I2(), one_vertex_neighbor.size());
				OutputDebugString(st);
#endif
				one_vertex_neighbor.insert(neighborId);
			}
			return true;
		}
		return false;
	}
	void RemoveNeighbor(UINT neighborId)
	{
		if (one_vertex_neighbor.contains(neighborId))
		{
			one_vertex_neighbor.erase(neighborId);
		}
		if (two_vertex_neighbor.contains(neighborId))
		{
			two_vertex_neighbor.erase(neighborId);
		}
	}
	UINT LoadMeshLet(int depth);
};

std::vector<std::shared_ptr<Triangle>>		triangleList;
extern std::vector<XMFLOAT3>				vertexList;
extern std::vector<vector<XMUINT4>>			vertexIndex;
extern std::vector<XMFLOAT2>				textureList;
extern std::vector<XMFLOAT3>				normalList;
#include <afxstr.h>
#include <tchar.h>
#include <cstringt.h>

UINT Triangle::LoadMeshLet(int depth)
{
	if (doneFlag) return 0;
	if (depth > 32) return 0; // prevent infinite recursion in case of a bug
	doneFlag = true;

	// This function will load the triangle and its neighbors into the meshlet lists.
	// find vertex in meshlet vertex buffer for each triangle vertex
	XMFLOAT3 v0 = vertexList[I0()];
	XMFLOAT3 v1 = vertexList[I1()];
	XMFLOAT3 v2 = vertexList[I2()];
//	UINT MeshWork::findVertex(XMFLOAT3 position, XMFLOAT3 norm, XMFLOAT2 texture, UINT materialIndex)

//	UINT m0 = FindVertex(v0);
//	UINT m1 = FindVertex(v1);
//	UINT m2 = FindVertex(v2);
	vector <UINT> neighbors = {};
	// It will also remove the neighbors from the sets as they are added to the meshlet lists.
	if (two_vertex_neighbor.size() > 0)
	{
		for (auto n_id : two_vertex_neighbor)
		{
			neighbors.push_back(n_id);
			shared_ptr<Triangle> neighbor = triangleList[n_id];
			neighbor->RemoveNeighbor(id);
	//		RemoveNeighbor(n_id);
		}
	}
	if (one_vertex_neighbor.size() > 0)
	{
		for (auto n_id : one_vertex_neighbor)
		{
//			neighbors.push_back(n_id);
			shared_ptr<Triangle> neighbor = triangleList[n_id];
			neighbor->RemoveNeighbor(id);
//			RemoveNeighbor(n_id);
		}
	}
	two_vertex_neighbor.clear();
	one_vertex_neighbor.clear();
	if (neighbors.size() > 0)
	{
		for (auto n_id : neighbors)
		{
			shared_ptr<Triangle> neighbor = triangleList[n_id];
			neighbor->LoadMeshLet(depth + 1);
		}
	}
// The recursion will stop when there are no more neighbors to add or when the max number of primitives per meshlet is reached.
	return 0;
}
#endif

#if 0
UINT Actor::CreateMeshlets()
{
	shared_ptr<Object> object = static_cast<shared_ptr<Object>>(DXImage::ObjectList[objectName]);
	TheMesh = object->GenerateObjectMeshlets();
	for (auto& i : vertexIndex)
	{
		shared_ptr<Triangle> t = make_shared<Triangle>((UINT)triangleList.size(), i);
		triangleList.push_back(t);
	}
	// Now we have the triangle list and the world vertices. We can find the neighbors for each triangle.
	for (int i = 0; i < triangleList.size() - 1; i++)
	{
		shared_ptr<Triangle> t1 = triangleList[i];
		for (int j = i + 1; j < triangleList.size(); j++)
		{
			shared_ptr<Triangle> t2 = triangleList[j];
			t1->AddNeighbor(t2->id, t2->I0());
			t1->AddNeighbor(t2->id, t2->I1());
			t1->AddNeighbor(t2->id, t2->I2());
			t2->AddNeighbor(t1->id, t1->I0());
			t2->AddNeighbor(t1->id, t1->I1());
			t2->AddNeighbor(t1->id, t1->I2());
		}
	}
	// all triangles have their neighbors, 
	// so we can create the meshlets by grouping triangles together based on their neighbors 
	// and the max number of primitives per meshlet.
	// starting with the first triangle, put vertices into the meshlet vertex buffer
	// and indices into the meshlet index buffer
	// recurse into the two vertex neighbors and add them to the meshlet lists
	// erase neighbors from sets as we go.
	// when the recursion comes back, go to the next triangle 
	// and repeat until all triangles have been processed.
	// Then divide the vertex buffer into meshlets of 64 primitives,
	shared_ptr<MeshWork> TheMesh = make_shared<MeshWork>();
	for (auto& t : triangleList)
	{
		t->LoadMeshLet(0);
	}
//	TheMesh->meshName = groupName;
//	string materialName = MaterialName();
	Meshlet meshlet;
	UINT meshletSize = 0;
	meshlet = { 0,TheMesh->GetIndexCount(),0,TheMesh->GetPrimitiveIndexCount() };
	for (auto& idx : vertexIndex) //int i = 0; i < (int)groupStartIndex.size() - 1; i++)
	{
		XMFLOAT3 v0 = Vertices()[idx[0].x];
		XMFLOAT3 v1 = Vertices()[idx[1].x];
		XMFLOAT3 v2 = Vertices()[idx[2].x];
		XMFLOAT2 t0 = TextureList()[idx[0].y];
		XMFLOAT2 t1 = TextureList()[idx[1].y];
		XMFLOAT2 t2 = TextureList()[idx[2].y];
		XMFLOAT3 n0 = NormalList()[idx[0].z];
		XMFLOAT3 n1 = NormalList()[idx[1].z];
		XMFLOAT3 n2 = NormalList()[idx[2].z];
		UINT materialIndex = idx[0].w; // Material::FindMaterialIndex(MaterialName());
		UINT i0 = TheMesh->findVertex(v0, n0, t0, materialIndex);
		UINT i1 = TheMesh->findVertex(v1, n1, t1, materialIndex);
		UINT i2 = TheMesh->findVertex(v2, n2, t2, materialIndex);
			// these represent the indices of the vertices in the vertex buffer, 
			// so we can use them to create the index buffer for the meshlet
		INT mi0 = TheMesh->findIndex(i0, meshlet.VertOffset); // VertexOffset set when meshlet initialized
		INT mi1 = TheMesh->findIndex(i1, meshlet.VertOffset);
		INT mi2 = TheMesh->findIndex(i2, meshlet.VertOffset);
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
		TheMesh->PrimitiveIndexBuffer().push_back(PackedTriangle((UINT)mi0 - 1, (UINT)mi1 - 1, (UINT)mi2 - 1));
		meshlet.PrimCount++;
		if (meshlet.PrimCount == 64)
		{
			TheMesh->AddMeshlet(meshlet, groupName);
			meshlet = { 0,TheMesh->GetIndexCount(),0,TheMesh->GetPrimitiveIndexCount() };
			meshletSize++;
		}
	}
	if (meshlet.PrimCount)
	{
		TheMesh->AddMeshlet(meshlet, groupName);
		meshlet = { 0,TheMesh->GetIndexCount(),0,TheMesh->GetPrimitiveIndexCount() };
		meshletSize++;
	}

#if 0
	CString st;
	st.Format(_T("%4d Add Meshlet: %s unique count = %d triangle count = %d\r\n"),
			  meshletSize, groupName.c_str(),
			  meshlet.VertCount, meshlet.PrimCount);
	OutputDebugString(st);
	meshletSize++;
#endif

	DebugMesh(groupName, TheMesh);
	return TheMesh;

}

#endif



















#if 0

for (auto& v : TheMesh)
{
//		v.Position = TranslateToWorld(v.Position, worldMatrix);
}
return 0;
}

//uint32_t Actor::GetInitialVertexCount()
//{
//	shared_ptr<Object> object = DXImage::ObjectList[objectName];
//	return object->GetVertexCount();
//}
//uint32_t Actor::GetInitialIndexCount()
//{
//	shared_ptr<Object> object = DXImage::ObjectList[objectName];
//	return object->GetIndexCount();
//}

/*
std::vector<std::shared_ptr<MeshWork>> Actor::GenerateMeshList(XMMATRIX world)
{
	std::vector<std::shared_ptr<MeshWork>> meshList;
	for (auto& object : objectList)
	{
		std::vector<std::shared_ptr<MeshWork>> objectMeshList = object->GenerateMeshList(world);
		meshList.insert(meshList.end(), objectMeshList.begin(), objectMeshList.end());
	}
	return meshList;
}
*/

#endif