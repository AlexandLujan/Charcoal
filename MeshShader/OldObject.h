#pragma once
#include "stdafx.h"
#include <afxwin.h>
#include <algorithm>
#include <cstdint>
#include <d3d12.h>
#include <DirectXMath.h>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>
#include "Const.h"
#include "Material.h"
#include "Utility.h"

#include "Model.h"

using namespace DirectX;
using namespace Microsoft::WRL;
struct Meshlet;
class MeshWork;

extern MaterialProperties Zero;


class Object
{
protected:
	std::string materialName;
	std::string objectName;
public:
	Object(std::string _name, std::string materialName = "ZERO") : objectName(_name), materialName(materialName)
	{}
	~Object()
	{

	}
	virtual std::string Iam() = 0;
	std::string& MaterialName() { return materialName; }
	virtual uint32_t GetMeshCount() { return 0; }
	virtual uint32_t GetVertexCount() { return 0; }
	virtual uint32_t GetIndexCount() { return 0; }
	std::string& Name() { return objectName; }
//	virtual UINT CreateMeshlets(MeshWork& TheMesh) = 0;
	virtual std::shared_ptr<MeshWork> CreateMeshlets() = 0;
//	virtual std::vector<std::shared_ptr<MeshWork>> CreateMeshlets() = 0;
//	virtual std::vector<std::shared_ptr<MeshWork>> GenerateMeshList(XMMATRIX world) = 0;
	virtual void Clear() = 0;
};
class MeshWork;
class MeshObject : public Object
{
protected:
	// list of vertices. XMFLOAT4 is the position, XMFLOAT3 is float of RGB components
	std::vector<XMFLOAT3>					vertexList = {};
	std::vector<std::vector<XMUINT4>>		vertexIndex = {};
	std::vector<XMFLOAT2>					textureValues = {};
	std::vector<XMFLOAT3>					normalValues = {};
	XMFLOAT3 minVertex = { 999999.0f,999999.0f,999999.0f };
	XMFLOAT3 maxVertex = { -999999.0f,-999999.0f,-999999.0f };
public:
	std::vector<uint32_t>					groupStartIndex = {};
	std::string								groupName = "";
	MeshObject() : Object("", "ZERO") {}
	MeshObject(std::string name, std::string material = "ZERO") : Object(name, material), groupName(name)
	{
		groupStartIndex = { 0 };
	}
	~MeshObject()
	{
		Clear();
	}
	std::string Iam() { return "MESH"; };
	std::vector<XMFLOAT3>& Vertices() { return vertexList; }
	std::vector<std::vector<XMUINT4>>& VertexIndex() { return vertexIndex; }
	std::vector<XMFLOAT2>& TextureList() { return textureValues; }
	std::vector<XMFLOAT3>& NormalList() { return normalValues; }
	std::shared_ptr<MeshWork> CreateMeshlets();
	void GenerateMinMax()
	{
		for (auto& v : vertexList)
		{
			minVertex.x = min(minVertex.x, v.x);
			minVertex.y = min(minVertex.y, v.y);
			minVertex.z = min(minVertex.z, v.z);
			maxVertex.x = max(maxVertex.x, v.x);
			maxVertex.y = max(maxVertex.y, v.y);
			maxVertex.z = max(maxVertex.z, v.z);
		}
	}
//	std::vector<std::shared_ptr<MeshWork>> CreateMeshlets();
//	std::vector<std::shared_ptr<MeshWork>> GenerateMeshList(XMMATRIX world);
	void DebugMesh(std::string meshName, std::shared_ptr<MeshWork> TheMesh);
	void LoadWorldVertices(std::vector<XMFLOAT3>& vertexList, std::vector<std::vector<XMUINT4>>& vertexIndex);
	XMFLOAT3 GetMinVertex() { return minVertex; }
	XMFLOAT3 GetMaxVertex() { return maxVertex; }
	void Clear()
	{
		vertexList.erase(vertexList.begin(), vertexList.end());
		vertexList.clear();
		vertexIndex.erase(vertexIndex.begin(), vertexIndex.end());
		vertexIndex.clear();
		groupStartIndex.erase(groupStartIndex.begin(), groupStartIndex.end());
		groupStartIndex.clear();
		groupName = "";
	}
	virtual uint32_t GetMeshCount() { return (UINT)groupStartIndex.size() - 1; }
	virtual uint32_t GetVertexCount() { return (UINT)vertexList.size(); }
	virtual uint32_t GetIndexCount() { return (UINT)vertexIndex.size(); }
	uint32_t findVertex(XMFLOAT3 position)
	{
		for (int i = 0; i < (int)vertexList.size(); i++)
		{
			if ((vertexList[i].x == position.x) and
				(vertexList[i].y == position.y) and
				(vertexList[i].z == position.z))
			{
				return i;
			}
		}
		uint32_t index = (uint32_t)vertexList.size();
		vertexList.push_back(position);
		return index;
	};
	uint32_t findTexture(XMFLOAT2 texture)
	{
		for (int i = 0; i < (int)textureValues.size(); i++)
		{
			if ((textureValues[i].x == texture.x) and
				(textureValues[i].y == texture.y))
			{
				return i;
			}
		}
		uint32_t index = (uint32_t)textureValues.size();
		textureValues.push_back(texture);
		return index;
	};
	uint32_t findNormal(XMFLOAT3 normal)
	{
		for (int i = 0; i < (int)normalValues.size(); i++)
		{
			if ((normalValues[i].x == normal.x) and
				(normalValues[i].y == normal.y) and
				(normalValues[i].z == normal.z))
			{
				return i;
			}
		}
		uint32_t index = (uint32_t)normalValues.size();
		normalValues.push_back(normal);
		return index;
	};

};

const XMUINT4 No_Index_XMUINT4 = { 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF };

class Assembly : public Object
{
protected:
	std::map<std::string, std::shared_ptr<MeshObject>> MeshObjectList;
public:
	Assembly(std::string name = "UNKNOWN", std::string material = "ZERO") : Object(name, material) {}
	std::map<std::string, std::shared_ptr<MeshObject>>& Components() { return MeshObjectList; }
	std::string Iam() { return "Assembly"; }

	void AddComponent(std::string name, std::shared_ptr<MeshObject> component) { MeshObjectList[name] = component; }
	void RemoveComponent(std::shared_ptr<MeshObject> component)
	{
		auto it = std::find_if(MeshObjectList.begin(), MeshObjectList.end(),
			[&](const std::pair<std::string, std::shared_ptr<MeshObject>>& pair) { return pair.second == component; });
		if (it != MeshObjectList.end())
		{
			MeshObjectList.erase(it);
		}
	}

	std::shared_ptr<MeshWork> CreateMeshlets() { return nullptr; }
	std::vector<std::shared_ptr<MeshWork>> CreateAssemblyMeshlets();
	std::shared_ptr<MeshObject>& GetComponent(std::string name) { return MeshObjectList[name]; }
//	UINT CreateMeshlets(MeshWork& TheMesh);
//	std::vector<std::shared_ptr<MeshWork>> CreateMeshlets();
//	std::vector<std::shared_ptr<MeshWork>> GenerateMeshList(XMMATRIX world);
	std::shared_ptr<MeshObject> CreateBox(int ring, int id, float width, float height, float depth, float rollAngle, XMFLOAT3 CenterPoint);
	std::shared_ptr<MeshObject> CreateBox(float width, float height, XMFLOAT3  angles, XMFLOAT3 CenterPoint);
	uint32_t GetMeshObjectCount() { return (uint32_t)MeshObjectList.size(); }
	uint32_t GetMeshCount()
	{
		uint32_t total = 0;
		for (auto& [key, value] : MeshObjectList)
		{
			total += value->GetMeshCount();
		}
		return total;
	}
	uint32_t GetVertexCount()
	{
		uint32_t total = 0;
		for (auto& [key, value] : MeshObjectList)
		{
			total += value->GetVertexCount();
		}
		return total;
	}
	uint32_t GetIndexCount()
	{
		uint32_t total = 0;
		for (auto& [key, value] : MeshObjectList)
		{
			total += value->GetIndexCount();
		}
		return total;
	}
	void Clear()
	{
		for (auto& [key, value] : MeshObjectList)
		{
			value->Clear();
		}
		MeshObjectList.clear();
	}
};

class Box : public Assembly
{
public:
	Box() : Assembly() {}
	Box(std::string name, float width, float height, float depth, std::map<std::string, std::string> materialTable);
	std::shared_ptr<MeshObject> CreateSide(std::string name, std::string sideName, std::vector<XMFLOAT3> Vertices, UINT normal, std::string materialName);
};

class OBJ_File_Mesh : public Assembly
{
	std::string filename = "";
public:
	OBJ_File_Mesh() : Assembly() {}
	OBJ_File_Mesh(std::string name, std::string directoryname = "", std::string filename = "", float scale = 1, std::string material = "ZERO", VertexExchange reverseYZ = XYZ, ImageJustify justification = ImageJustify::NOTHING)
		: Assembly(name)
	{
		objReader(directoryname, filename, reverseYZ, justification);
		for (auto& m : MeshObjectList)
		{
			m.second->GenerateMinMax();
		}
	}
	int objReader(std::string directoryname, std::string filename, VertexExchange reverseYZ, ImageJustify justification);
	int LoadMTLFile(std::string directoryname, const std::string mtlFileName);
	void JustifyVertices(std::vector<XMFLOAT3>& vertices, XMFLOAT3 minVertex, XMFLOAT3 maxVertex, ImageJustify justification);
};


class Fractal : public Assembly
{
	std::string fractalType;
public:
	Fractal() : Assembly() {}
	Fractal(std::string name = "default", std::string type = "MOUNTAIN", int iter = 3, std::vector<XMFLOAT3> baseCoords = {}, std::vector<XMUINT3> baseIndex = {})
		: Assembly(name), fractalType(type)
	{
		if (type == "MOUNTAIN")	MountainOutOfNothing(baseCoords, baseIndex, iter);
	}
	~Fractal() { Clear(); }
	void MountainOutOfNothing(std::vector<XMFLOAT3> baseCoords, std::vector<XMUINT3> baseIndex, int iter);
};

class CDC;
class CFont;
enum Justification
{
	Left_Justified,
	Center_Justified,
	Right_Justified
};
class TextMesh : public Assembly
{
public:

	TextMesh(std::string name, std::vector<std::string> t = {},
		std::string fontname = "Arial", uint32_t fontFamily = FF_SWISS, uint32_t fontsize = 24,
		std::string material = "ZERO", std::string _hOrigin = "Left", std::string _vOrigin = "Top",
		Justification justify = Justification::Left_Justified);
	~TextMesh() { Clear(); }
	void CreateGlyphOutline(CDC& dc, std::vector<std::string> text, CFont& font, Justification justify, char hOrigin = 'L', char vOrigin = 'T', std::string material = "BLACK");
	void Clear() {}
};

class RoundTunnel : public Assembly
{
	std::string tunnel_name;
	float diameter;
	float length;
	float ringWidth;
	float tilesPerRing;
public:
	RoundTunnel(std::string tunnel_name, float diameter, float length, float ringWidth, float tilesPerSegment);
};

class TriangleTunnel : public Assembly
{
	std::string tunnel_name;
	float base;
	float height;
	float length;
	float segmentWidth;
public:
	TriangleTunnel(std::string tunnel_name, float base, float height, float length, float segmentWidth);
};

class StarShip : public Assembly
{
	std::vector<XMFLOAT3>					vertices;
	std::vector<std::vector<XMUINT3>>		v_index;

	float base_cube;
	float max_height;
	float max_width;
	UINT pin_count;
	std::vector<XMFLOAT3> CreateCube(float side);
	void AssignIndicesToSpireVertices(std::vector<XMFLOAT3> spirePoints);
	std::vector<XMFLOAT3> BuildSpire(int orientation, XMFLOAT3 base, float pin_width, float spire_height, int spire_layer);
	uint32_t findVertex(XMFLOAT3 position);
public:
	StarShip(std::string ship_name = "", std::string material = "ZERO", float cube_size = 25, float height = 200, UINT pin_count = 5);
};
