#pragma once
#include <wrl.h>
using namespace Microsoft::WRL;
#include <dxgidebug.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <vector>
#include <string>
#include "Const.h"
#include "Utility.h"
#include "ColorControl.h"
#include <map>
using namespace DirectX;
//using namespace std;

std::wstring string_to_wstring_mb(const std::string& narrow_str);

class Object
{
protected:
	ColorControl colorControl;
	XMFLOAT4 position;
//	XMFLOAT4 rgb;
//	std::string colorName = ;
	boolean is_visible = true;
	boolean is_dirty = false;
	std::string name;
	float pitch = 0.0f; // Rotate about X Axis radians
	float yaw = 0.0f;   // Rotate about Y Axis radians
	float roll = 0.0f;  // Rotate about Z Axis radians
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float scaleZ = 1.0f;
	XMMATRIX worldMatrix;
public:
	Object(std::string _name, std::string _rgb = "WHITE") : name(_name)//, colorName(_rgb)
	{
		position.x = 0;
		position.y = 0;
		position.z = 0;
		colorControl.SetColor(_rgb);
		worldMatrix = DirectX::XMMatrixIdentity();
	}
	Object(std::string n = "", float x = 0, float y = 0, float z = 0, std::string color = "WHITE") :
		name(n), position(x, y, z, 1.0f) //, colorName(color)
	{
		colorControl.SetColor(color);
	//	rgb = FindColor(colorName);
		worldMatrix = DirectX::XMMatrixIdentity();
	}
	Object(std::string n, XMFLOAT4 p, std::string color = "WHITE") : name(n), position(p)//, colorName(color)
	{
		position.w = 1.0f;
		colorControl.SetColor(color);
	//	rgb = FindColor(colorName);
		worldMatrix = DirectX::XMMatrixIdentity();
	}
	~Object()
	{

	}
	virtual std::string Iam() = 0;
	XMFLOAT4& Position() { return position; }
	XMFLOAT4 Color() { return colorControl.CurrentColor(); }
	std::string& Name() { return name; }
	std::string ColorName() { return colorControl.ColorName(); }
	float& Pitch() { return pitch; }
	float& Yaw() { return yaw; }
	float& Roll() { return roll; }
	float& X() { return position.x; }
	float& Y() { return position.y; }
	float& Z() { return position.z; }
	virtual uint32_t MeshTriangleCount() { return 0; }
	virtual uint32_t MeshVerticeCount() { return 0; }
	virtual bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) = 0;
	virtual void Clear() = 0;
	boolean& IsVisible() { return is_visible; }
	boolean& IsDirty() { return is_dirty; }
	ColorControl& ObjectColorControl() { return colorControl; }

	virtual void MoveTo(float x, float y, float z)
	{
		position.x = x;
		position.y = y;
		position.z = z;
		UpdateWorld();
	}
	virtual void MoveBy(float dx, float dy, float dz)
	{
		position.x += dx;
		position.y += dy;
		position.z += dz;
		UpdateWorld();
	}
	virtual void RotateTo(float _pitch, float _yaw, float _roll) //degrees
	{
		pitch = DegreesToRadians(_pitch);
		yaw = DegreesToRadians(_yaw);
		roll = DegreesToRadians(_roll);
		UpdateWorld();
	}
	virtual void RotateBy(float deltaPitch, float deltaYaw, float deltaRoll)
	{
		pitch += DegreesToRadians(deltaPitch);
		yaw += DegreesToRadians(deltaYaw);
		roll += DegreesToRadians(deltaRoll);
		UpdateWorld();
	}
	virtual void PitchTo(float _pitch)
	{
		pitch = DegreesToRadians(_pitch);
		UpdateWorld();
	}
	virtual void YawTo(float _yaw)
	{
		yaw = DegreesToRadians(_yaw);
		UpdateWorld();
	}
	virtual void RollTo(float _roll)
	{
		roll = DegreesToRadians(_roll);
		UpdateWorld();
	}
	virtual void PitchBy(float _pitch)
	{
		pitch += DegreesToRadians(_pitch);
		UpdateWorld();
	}
	virtual void YawBy(float _yaw)
	{
		yaw += DegreesToRadians(_yaw);
		UpdateWorld();
	}
	virtual void RollBy(float _roll)
	{
		roll += DegreesToRadians(_roll);
		UpdateWorld();
	}
	virtual void ScaleTo(float sX, float sY, float sZ)
	{
		scaleX = sX;
		scaleY = sY;
		scaleZ = sZ;
		UpdateWorld();
	}
	virtual void ScaleBy(float dScaleX, float dScaleY, float dScaleZ)
	{
		scaleX += dScaleX;
		scaleY += dScaleY;
		scaleZ += dScaleZ;
		UpdateWorld();
	}
	virtual void Erase() = 0;
	XMMATRIX& World() { return worldMatrix; }
	void UpdateWorld()
	{
		XMMATRIX matScale = DirectX::XMMatrixScaling(scaleX, scaleY, scaleZ);
		XMMATRIX matRotateX = DirectX::XMMatrixRotationX(pitch);
		XMMATRIX matRotateY = DirectX::XMMatrixRotationY(yaw);
		XMMATRIX matRotateZ = DirectX::XMMatrixRotationZ(roll);
		XMMATRIX matTranslate = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		worldMatrix = matScale * matRotateX * matRotateY * matRotateZ * matTranslate;
	}
};

class gLine : public Object
{
	XMFLOAT4 startPosition;
	XMFLOAT4 endPosition;
public:
	gLine(std::string name, float startX, float startY, float startZ, float endX, float endY, float endZ, std::string color)
		: Object(name, 0, 0, 0, color), startPosition(startX, startY, startZ, 1), endPosition(endX, endY, endZ, 1)
	{
		UpdateWorld();
	}
	~gLine() {}
	virtual std::string Iam() { return "LINE"; };

	XMFLOAT4& Start() { return startPosition; }
	XMFLOAT4& End() { return endPosition; }
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override { return false; }
	void ScaleTo(float scaleX, float scaleY, float scaleZ) override {}
	void ScaleBy(float dScaleX, float dScaleY, float dScaleZ) override {}
	void Erase() override {}
	void Clear() override {}
};
class TemplateRef : public Object
{
	std::string templateName = "";
public:
	TemplateRef() : Object("", "WHITE") {}
	TemplateRef(std::string name, std::string _templateName, std::string color = "WHITE")
		: Object(name, color), templateName(_templateName)
	{
		UpdateWorld();
	}
	std::string& TemplateName() { return templateName; }
	virtual std::string Iam() { return "TEMPLATE_REF"; };
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override { return false; }
	void ScaleTo(float scaleX, float scaleY, float scaleZ) override {}
	void ScaleBy(float dScaleX, float dScaleY, float dScaleZ) override {}
	void Erase() override {}
	void Clear() override {}
};

class gCircle : public Object
{
	float diameter = 0;
public:
	gCircle() : Object("", "WHITE") {}
	gCircle(std::string name, float startX = 0, float startY = 0, float startZ = 0, float diamater = 1, std::string color = "WHITE")
		: Object(name, XMFLOAT4(startX, startY, startZ, 1), color), diameter(diamater)
	{
		UpdateWorld();
	}
	gCircle(std::string name, XMFLOAT4 center, float diamater = 1, std::string color = "WHITE")
		: Object(name, center, color), diameter(diamater)
	{
		UpdateWorld();
	}
	float& Diameter() { return diameter; }
	XMFLOAT4& Center() { return position; }
	virtual std::string Iam() { return "CIRCLE"; };
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override { return false; }
	void Erase() override {}
	void Clear() override {}
};

class gTriangle : public Object
{
	std::vector <XMFLOAT4> vertex_list;
public:
	gTriangle(std::string name, std::vector <XMFLOAT4> vertices, std::string color)
		: Object(name, 0, 0, 0, color), vertex_list(vertices)
	{
		UpdateWorld();
	}
	~gTriangle() {}
	XMFLOAT4& Vertex1() { return vertex_list[0]; }
	XMFLOAT4& Vertex2() { return vertex_list[1]; }
	XMFLOAT4& Vertex3() { return vertex_list[2]; }
	std::vector<XMFLOAT4>& Vertices() { return vertex_list; }
	virtual std::string Iam() { return "TRIANGLE"; };
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override { return false; }
	void Erase() override {}
	void Clear() override {}
};

class gPolygon : public Object
{
	std::vector<VertexType> vertex_list;
public:
	gPolygon(std::string name, std::vector<XMFLOAT4> vertices, std::string color)
		: Object(name, 0, 0, 0, color)
	{
		for (auto& v : vertices)
		{
			vertex_list.push_back(std::make_pair(v, FindColor(color)));
		}
		UpdateWorld();
	}
	~gPolygon() { Clear(); }
	virtual std::string Iam() { return "POLYGON"; };
	std::vector<VertexType>& Vertices() { return vertex_list; }
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override { return false; }
	void Erase() override {}
	void Clear() override
	{
		vertex_list.erase(vertex_list.begin(), vertex_list.end());
		vertex_list.clear();
	}
};

class gMultiPolygon : public Object
{
	std::vector<std::vector<VertexType>> polygons;
public:
	gMultiPolygon(std::string name, std::vector<std::vector<VertexType>> polys, std::string color)
		: Object(name, 0, 0, 0, color), polygons(polys)
	{
//		for (auto& poly : polys)
//		{
//			vector<pair<XMFLOAT4, XMFLOAT3>> polygon = {};
//			for (auto& v : poly)
//			{
//				polygon.push_back(make_pair(v, RGB2FRGB(color)));
//			}
//			polygons.push_back(polygon);
//		}
		UpdateWorld();
	}
	~gMultiPolygon() { Clear(); }
	std::vector<std::vector<VertexType>>& Polygons() { return polygons; }
	virtual std::string Iam() { return "MULTIPLOYGON"; };
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override { return false; }
	void Erase() override {}
	void Clear() override
	{
		for (auto& poly : polygons)
		{
			poly.erase(poly.begin(), poly.end());
			poly.clear();
		}
		polygons.erase(polygons.begin(), polygons.end());
		polygons.clear();
	}
};
class MeshObject : public Object
{
protected:
	// list of vertices. XMFLOAT4 is the position, XMFLOAT3 is float of RGB components
	std::vector<VertexType>					vertexList;
	std::vector<vertex_index>				vertexIndex;
	std::vector<XMFLOAT3>					textureValues;
	std::vector<XMFLOAT3>					normalValues;
public:
	std::vector<uint32_t>					groupStartIndex = {};
	std::vector<std::string>				groupName = {};
	std::vector<XMFLOAT4>					groupColor = {};
	MeshObject() : Object("", "VIOLET") {}
	MeshObject(std::string name, std::string color = "BROWN") : Object(name, color) {}
	MeshObject(std::string n, XMFLOAT4 p, std::string color = "WHITE") : Object(n, p, color) {}
	~MeshObject()
	{
		Clear();
	}
//	virtual uint32_t MeshTriangleCount() { return (uint32_t)vertexIndex.size(); }
//	virtual uint32_t MeshVerticeCount() { return (uint32_t)vertexList.size();; }
	virtual std::string Iam() { return "MESH"; };
	std::vector<VertexType>& Vertices() { return vertexList; }
	std::vector<vertex_index>& VertexIndex() { return vertexIndex; }
	std::vector<XMFLOAT3>& TextureList() { return textureValues; }
	std::vector<XMFLOAT3>& NormalList() { return normalValues; }
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override;
	class Mesh;
	void UpdateMeshVertices(std::shared_ptr<Mesh> mesh, XMMATRIX cameraMatrix = XMMatrixIdentity());

	void Erase() override {}
	void Clear() override
	{
		vertexList.erase(vertexList.begin(), vertexList.end());
		vertexList.clear();
		vertexIndex.erase(vertexIndex.begin(), vertexIndex.end());
		vertexIndex.clear();
		groupStartIndex.erase(groupStartIndex.begin(), groupStartIndex.end());
		groupStartIndex.clear();
		groupName.erase(groupName.begin(), groupName.end());
		groupName.clear();
		groupColor.erase(groupColor.begin(), groupColor.end());
		groupColor.clear();
	}

};

class OBJ_File_Mesh : public MeshObject
{
	std::string filename = "";
public:
	OBJ_File_Mesh() : MeshObject() {}
	OBJ_File_Mesh(std::string name, std::string filename = "", float scale = 1, std::string color = "WHITE", VertexExchange reverseYZ = XYZ, ImageJustify justification = ImageJustify::NOTHING)
		: MeshObject(name, color), filename(filename)
	{
		ScaleTo(scale, scale, scale);
		objReader(filename, reverseYZ, justification);
		UpdateWorld();
	}
//	virtual string Iam() { return "OBJ_FILE"; };
	int  objReader(std::string filename, VertexExchange reverseYZ = XYZ, ImageJustify justification = ImageJustify::NOTHING);
	void JustifyVertices(std::vector<VertexType>& vertices, XMFLOAT3 minVertex, XMFLOAT3 maxVertex, ImageJustify justification);
	void Erase() override {}
	void Clear() override
	{
		MeshObject::Clear();
	}
};


class Fractal : public MeshObject
{
	std::string fractalType;
	int iterations;
	std::vector<std::string> colorTable;
public:
	Fractal(std::string name, std::string type, int iter, XMFLOAT3 scaleFactor = { 1,1,1 }, std::vector<VertexType> baseCoords = {}, std::vector<vertex_index> baseIndex = {}, std::vector<std::string> colorTable = { "BLACK" })
		: MeshObject(name), fractalType(type), iterations(iter), colorTable(colorTable)
	{
		ScaleTo(scaleFactor.x, scaleFactor.y, scaleFactor.z);
		MountainOutOfNothing(baseCoords, baseIndex);
		UpdateWorld();
	}
	~Fractal() { MeshObject::Clear(); }
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override { return false; }
	void Erase() override {}
	void Clear() override { MeshObject::Clear(); }
	void MountainOutOfNothing(std::vector<VertexType> baseCoords, std::vector<vertex_index> baseIndex);
};

class TextMesh : public MeshObject
{
//	vector<gMultiPolygon*> o_glyphOutlines; // original glyph outlines 0,0,0 origin
public:
	TextMesh(std::string name, std::vector<std::string> t = {},
		std::string fontname = "Arial", uint32_t fontFamily = FF_SWISS, uint32_t fontsize = 24,
		std::string color = "WHITE", std::string _hOrigin = "Left", std::string _vOrigin = "Top");
	~TextMesh() { Clear(); }
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override { return false; }
	void Erase() override {}
	void CreateGlyphOutline(CDC& dc, std::vector<std::string> text, CFont& font, float start_x, float start_y, std::string color, char hOrigin = 'L', char vOrigin = 'T');
	void Clear() override
	{}
};

const XMUINT3 No_Index_XMUINT3 = { 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF };

class Box : public MeshObject
{
public:
	Box(std::string name, float width, float height, float depth, std::map<std::string, std::string> colorTable) : MeshObject(name)
	{
		float dWidth = width / 2.0f;
		float dHeight = height / 2.0f;
		float dDepth = depth / 2.0f;
		VertexType ruf = std::make_pair(XMFLOAT4(dWidth, dHeight, dDepth, 1), FindColor(colorTable["ruf"]));
		VertexType rlf = std::make_pair(XMFLOAT4(dWidth, -dHeight, dDepth, 1), FindColor(colorTable["rlf"]));
		VertexType luf = std::make_pair(XMFLOAT4(-dWidth, dHeight, dDepth, 1), FindColor(colorTable["luf"]));
		VertexType llf = std::make_pair(XMFLOAT4(-dWidth, -dHeight, dDepth, 1), FindColor(colorTable["llf"]));
		VertexType rur = std::make_pair(XMFLOAT4(dWidth, dHeight, -dDepth, 1), FindColor(colorTable["rur"]));
		VertexType rlr = std::make_pair(XMFLOAT4(dWidth, -dHeight, -dDepth, 1), FindColor(colorTable["rlr"]));
		VertexType lur = std::make_pair(XMFLOAT4(-dWidth, dHeight, -dDepth, 1), FindColor(colorTable["lur"]));
		VertexType llr = std::make_pair(XMFLOAT4(-dWidth, -dHeight, -dDepth, 1), FindColor(colorTable["llr"]));
		vertexList = { ruf,rlf,luf,llf,rur,rlr,lur,llr };
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

		vertexIndex = { vertex_index(i00,No_Index_XMUINT3,No_Index_XMUINT3),
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

		groupStartIndex = { 0,(uint32_t)vertexIndex.size() };
		groupColor = { FindColor("GREEN") };
		UpdateWorld();
	}
};

class MultiMeshObject : public Object
{
protected:
	std::map<std::string, std::shared_ptr<MeshObject>> meshList;
	std::shared_ptr<MeshObject> CreateBox(float width, float height, float depth, float rollAngle, XMFLOAT3 CenterPoint);
public:
	MultiMeshObject(std::string name = "Multi", std::string color = "WHITE") : Object(name, color) {}
	~MultiMeshObject() { Clear(); }
	virtual std::string Iam() { return "MULTIPLE MESH"; }
	std::map<std::string, std::shared_ptr<MeshObject>>& MeshList() { return meshList; }
	bool Update(uint64_t frameNumber, XMMATRIX ViewMatrix) override { return false; }
	void Erase() override {}
	void Clear() override
	{
		for (auto& [key, value] : meshList)
		{
		}
		meshList.clear();
	}
};

class RoundTunnel : public MultiMeshObject
{
	std::string tunnel_name;
	float diameter;
	float length;
	float ringWidth;
	float tilesPerRing;
public:
	RoundTunnel(std::string tunnel_name, float diameter, float length, float ringWidth, float tilesPerSegment);
};

class TriangleTunnel : public MultiMeshObject
{
	std::string tunnel_name;
	float base;
	float height;
	float length;
	float segmentWidth;
public:
	TriangleTunnel(std::string tunnel_name, float base, float height, float length, float segmentWidth);
};
