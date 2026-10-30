#pragma once
#include "stdafx.h"
#include <cstdint>
#include <DirectXMath.h>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <windows.h>
#include "Const.h"
#include "Model.h"
#include "nlohmann/json.hpp"
#include "struct.h"
using json = nlohmann::json;

using namespace DirectX;
using namespace Microsoft::WRL;
typedef std::unordered_map<std::string, MeshStatistics>	MeshletDirectory; // name, meshlet start, meshlet count	

struct MeshObject
{
	std::string								groupName = "";
	std::string								materialName = "";
	std::vector<uint32_t>					groupStartIndex = {};
	// list of vertices. XMFLOAT4 is the position, XMFLOAT3 is float of RGB components
	std::vector<XMFLOAT3>					vertexList = {};
	std::vector<std::vector<XMUINT4>>		vertexIndex = {};
	std::vector<XMFLOAT2>					textureValues = {};
	std::vector<XMFLOAT3>					normalValues = {};
	std::vector<XMFLOAT3>& Vertices() { return vertexList; }
	std::vector<std::vector<XMUINT4>>& VertexIndex() { return vertexIndex; }
	std::vector<XMFLOAT2>& TextureList() { return textureValues; }
	std::vector<XMFLOAT3>& NormalList() { return normalValues; }
	std::string& Name() { return groupName; }
	std::string& MaterialName() { return materialName; }
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

class Object
{
protected:
	std::string objectName;
	std::string materialName;
	MeshletDirectory   meshDirectory;
	std::unordered_map<std::string, std::shared_ptr<MeshObject>> MeshObjectList;
public:
	Object(std::string _name) : objectName(_name)
	{}
	~Object()
	{
		meshDirectory.clear();
		MeshObjectList.clear();
	}
	std::string& Name() { return objectName; }
	void Clear()
	{
		MeshObjectList.clear();
	}
	void AddComponent(std::string name, std::shared_ptr<MeshObject> component) { MeshObjectList[name] = component; }
	//std::vector<std::shared_ptr<MeshWork>> GenerateObjectMeshlets();
	std::shared_ptr<MeshObject> CreateBox(int ring, int id, float width, float height, float depth, float rollAngle, XMFLOAT3 CenterPoint);
	std::shared_ptr<MeshObject> CreateBox(float width, float height, XMFLOAT3  angles, XMFLOAT3 CenterPoint);
	uint32_t GetMeshObjectCount() { return (uint32_t)MeshObjectList.size(); }
	std::shared_ptr<MeshObject>& GetComponent(std::string name) { return MeshObjectList[name]; }
	MeshletDirectory& MeshDirectory() { return meshDirectory; }
	std::unordered_map<std::string, std::shared_ptr<MeshObject>>& GetMeshObjectList() { return MeshObjectList; }
};

class Box : public Object
{
public:
	Box() : Object("a box") {}
	Box(std::string name, float width, float height, float depth, std::map<std::string, std::string> materialTable);
	std::shared_ptr<MeshObject> CreateSide(std::string name, std::string sideName, std::vector<XMFLOAT3> Vertices, UINT normal, std::string materialName);
};

class OBJ_File_Mesh : public Object
{
	std::string filename = "";
	std::vector<std::string> meshNames;
public:
	OBJ_File_Mesh() : Object("an OBJ file mesh") {}
	OBJ_File_Mesh(std::string name, json& meshNames, std::string directoryname = "", std::string filename = "", float scale = 1, std::string material = "ZERO", VertexExchange reverseXYZ = XYZ, ImageJustify justification = ImageJustify::NOTHING, XMUINT3 mirror = { 0,0,0 }, BOOL woundCCW = true)
		: Object(name)
	{
		for (json::const_iterator it = meshNames.cbegin(); it != meshNames.cend(); it++)
		{
			std::string name = *it;
			AddMeshName(name);
		}
		objReader(directoryname, filename, reverseXYZ, justification, mirror, woundCCW);
	}
	void AddMeshName(std::string name) { meshNames.push_back(name); }
	bool UseMeshName(std::string name)
	{
		for (auto n : meshNames)
		{
			if (name == n)
			{
				return true;
			}
		}
		return false;
	}
	int objReader(std::string directoryname, std::string filename, VertexExchange reverseXYZ, ImageJustify justification, XMUINT3 mirror, BOOL woundCCW);
	int LoadMTLFile(std::string directoryname, const std::string mtlFileName);
	void JustifyVertices(std::vector<XMFLOAT3>& vertices, XMFLOAT3 minVertex, XMFLOAT3 maxVertex, ImageJustify justification);
};


class Fractal : public Object
{
	std::string fractalType;
public:
	Fractal() : Object("a fractal") {}
	Fractal(std::string name = "default", std::string type = "MOUNTAIN", int iter = 3, std::vector<XMFLOAT3> baseCoords = {}, std::vector<XMUINT3> baseIndex = {})
		: Object(name), fractalType(type)
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
class TextMesh : public Object
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

class RoundTunnel : public Object
{
	std::string tunnel_name;
	float diameter;
	float length;
	float ringWidth;
	float tilesPerRing;
public:
	RoundTunnel(std::string tunnel_name, float diameter, float length, float ringWidth, float tilesPerSegment);
};

class TriangleTunnel : public Object
{
	std::string tunnel_name;
	float base;
	float height;
	float length;
	float segmentWidth;
public:
	TriangleTunnel(std::string tunnel_name, float base, float height, float length, float segmentWidth);
};

class StarShip : public Object
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
