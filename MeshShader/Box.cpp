#include "stdafx.h"
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include "Material.h"
#include "Object.h"

using namespace std;
extern std::unordered_map<std::string, std::shared_ptr<Material>> MaterialsMap;

shared_ptr<MeshObject> Box::CreateSide(std::string name, std::string sideName, vector<XMFLOAT3> Vertices, UINT normal, std::string materialName)
{
	shared_ptr < MeshObject> component = make_shared<MeshObject>(name + "_" + sideName, materialName);
	UINT i0 = component->findVertex(Vertices[0]);
	UINT i1 = component->findVertex(Vertices[1]);
	UINT i2 = component->findVertex(Vertices[2]);
	UINT i3 = component->findVertex(Vertices[3]);
	component->VertexIndex().push_back({ { i0,0,0,0 }, {i1,0,0,0}, {i2,0,0,0} });
	component->VertexIndex().push_back({ { i0,0,0,0 }, {i2,0,0,0}, {i3,0,0,0} });
//	component->VertexIndex().push_back({ { i0,0,0 }, {i2,0,0}, {i1,0,0} });
//	component->VertexIndex().push_back({ { i0,0,0 }, {i3,0,0}, {i1,0,0} });
	component->TextureList().push_back({ 0,0 });
	vector<XMFLOAT3> normalList = { {0,0,-1.0f}, {0,0,1.0f}, {0,1.0f,0}, {0,-1.0f,0}, {-1.0f,0,0}, {1.0f,0,0} };
	component->NormalList().push_back(normalList[normal]);
	component->groupStartIndex = { 0,2 };
	component->groupName = sideName;
	return component;
}

Box::Box(std::string name, float width = 1, float height = 1, float depth = 1,
	std::map<std::string, std::string> materialTable = {
	 { "front", "white" }, { "back", "white" }, { "top", "white" },
	 { "bottom", "white" }, { "left", "white" }, { "right", "white" } }) :Object(name)

{
	float dWidth = width / 2.0f;
	float dHeight = height / 2.0f;
	float dDepth = depth / 2.0f;
	string											groupName;
	shared_ptr<MeshObject>							component = nullptr;

	XMFLOAT3 ruf(dWidth, dHeight, -dDepth);
	XMFLOAT3 rlf(dWidth, -dHeight, -dDepth);
	XMFLOAT3 luf(-dWidth, dHeight, -dDepth);
	XMFLOAT3 llf(-dWidth, -dHeight, -dDepth);
	XMFLOAT3 rur(dWidth, dHeight, dDepth);
	XMFLOAT3 rlr(dWidth, -dHeight, dDepth);
	XMFLOAT3 lur(-dWidth, dHeight, dDepth);
	XMFLOAT3 llr(-dWidth, -dHeight, dDepth);

	groupName = "front";
	component = CreateSide(name, groupName, { ruf,luf,llf,rlf }, 0, materialTable[groupName]);
	AddComponent(groupName, component);

	groupName = "back";
	component = CreateSide(name, groupName, { lur,rur,rlr,llr }, 1, materialTable[groupName]);
	AddComponent(groupName, component);

	groupName = "top";
	component = CreateSide(name, groupName, { rur,lur,luf,ruf }, 2, materialTable[groupName]);
	AddComponent(groupName, component);

	groupName = "bottom";
	component = CreateSide(name, groupName, { rlf,rlf,llr,llf }, 3, materialTable[groupName]);
	AddComponent(groupName, component);

	groupName = "left";
	component = CreateSide(name, groupName, { luf,lur,llr,llf }, 4, materialTable[groupName]);
	AddComponent(groupName, component);

	groupName = "right";
	component = CreateSide(name, groupName, { ruf,rlf,rlr,rur }, 5, materialTable[groupName]);
	AddComponent(groupName, component);


}
