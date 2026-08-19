// objReader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "OBJ_3D_Model.h"
#include <DirectXMath.h>
#include <Mesh.h>
#include <Scene.h>
#include <SceneNode.h>
#include "Utility.h"
using namespace std;
using namespace DirectX;
using namespace dx12lib;

shared_ptr<Scene> OBJ_3D_Model::CreateModel(string filename, shared_ptr<CommandList> commandList, float scale)
{
	auto node = std::make_shared<SceneNode>();
	node->SetName(filename);
	int r = objReader(filename+".obj");
	VertexCollection vertexCollection;
	IndexCollection indexCollection;
	for (object* obj : objects)
	{
		for (face f : obj->faces)
		{
			if (f.vertexIndex.size() > 2)
			{
				XMFLOAT3 P0 = vertices[f.vertexIndex[0]];
				XMFLOAT3 T0 = (f.textureIndex[0] != 0xFFFFFFFF) ? textureVertices[f.textureIndex[0]] : XMFLOAT3(0.0f, 0.0f, 0.0f);
				XMFLOAT3 N0 = (f.normalIndex[0] != 0xFFFFFFFF) ? vertexNormals[f.normalIndex[0]] : XMFLOAT3(0.0f, 0.0f, 0.0f);
				P0 = XMFLOAT3(P0.x * scale, P0.y * scale, P0.z * scale);
				obj->mn = XMFLOAT4(std::min(obj->mn.x, P0.x), std::min(obj->mn.y, P0.y), std::min(obj->mn.z, P0.z),0);
				obj->mx = XMFLOAT4(std::max(obj->mx.x, P0.x), std::max(obj->mx.y, P0.y), std::max(obj->mx.z, P0.z),0);
				FXMVECTOR mx;
				for (int i = 1; i < f.vertexIndex.size() - 1; i++)
				{
					XMFLOAT3 P1 = vertices[f.vertexIndex[i]];
					XMFLOAT3 P2 = vertices[f.vertexIndex[i + 1]];
					P1 = XMFLOAT3(P1.x * scale, P1.y * scale, P1.z * scale);
					P2 = XMFLOAT3(P2.x * scale, P2.y * scale, P2.z * scale);
					obj->mn = XMFLOAT4(std::min(obj->mn.x, P1.x), std::min(obj->mn.y, P1.y), std::min(obj->mn.z, P1.z),0);
					obj->mx = XMFLOAT4(std::max(obj->mx.x, P1.x), std::max(obj->mx.y, P1.y), std::max(obj->mx.z, P1.z),0);
					obj->mn = XMFLOAT4(std::min(obj->mn.x, P2.x), std::min(obj->mn.y, P2.y), std::min(obj->mn.z, P2.z),0);
					obj->mx = XMFLOAT4(std::max(obj->mx.x, P2.x), std::max(obj->mx.y, P2.y), std::max(obj->mx.z, P2.z),0);
					XMFLOAT3 T1 = (f.textureIndex[i] != 0xFFFFFFFF) ? textureVertices[f.textureIndex[i]] : XMFLOAT3(0.0f, 0.0f, 0.0f);
					XMFLOAT3 T2 = (f.textureIndex[i+1] != 0xFFFFFFFF) ? textureVertices[f.textureIndex[i+1]] : XMFLOAT3(0.0f, 0.0f, 0.0f);
					XMFLOAT3 N1 = (f.normalIndex[i] != 0xFFFFFFFF) ? vertexNormals[f.normalIndex[i]] : XMFLOAT3(0.0f, 0.0f, 0.0f);
					XMFLOAT3 N2 = (f.normalIndex[i+1] != 0xFFFFFFFF) ? vertexNormals[f.normalIndex[i+1]] : XMFLOAT3(0.0f, 0.0f, 0.0f);
					vertexCollection.emplace_back(P0, T0, N0);
					vertexCollection.emplace_back(P1, T1, N1);
					vertexCollection.emplace_back(P2, T2, N2);
					indexCollection.emplace_back(static_cast<uint32_t>(indexCollection.size()));
					indexCollection.emplace_back(static_cast<uint32_t>(indexCollection.size()));
					indexCollection.emplace_back(static_cast<uint32_t>(indexCollection.size()));
				}

			}
		}

		auto mesh = CreateMesh(obj->name, obj->color, vertexCollection, indexCollection, commandList, obj->mn,obj->mx);
		node->AddMesh(mesh);
	}
	vertexCollection.clear();
	indexCollection.clear();
	auto scene = std::make_shared<Scene>();
	scene->SetRootNode(node);
	return scene;
}
#if 1
/*
Material OBJ_3D_Model::findMaterial(string name)
{
	if (name == "Zero") return Material::Zero;
	if (name == "Red") return Material::Red;
	if (name == "Green") return Material::Green;
	if (name == "Blue") return Material::Blue;
	if (name == "Cyan") return Material::Cyan;
	if (name == "Magenta") return Material::Magenta;
	if (name == "Yellow") return Material::Yellow;
	if (name == "White") return Material::White;
	if (name == "WhiteDiffuse") return Material::WhiteDiffuse;
	if (name == "Black") return Material::Black;
	if (name == "Emerald") return Material::Emerald;
	if (name == "Jade") return Material::Jade;
	if (name == "Obsidian") return Material::Obsidian;
	if (name == "Pearl") return Material::Pearl;
	if (name == "Ruby") return Material::Ruby;
	if (name == "Turquoise") return Material::Turquoise;
	if (name == "Brass") return Material::Brass;
	if (name == "Bronze") return Material::Bronze;
	if (name == "Chrome") return Material::Chrome;
	if (name == "Copper") return Material::Copper;
	if (name == "Gold") return Material::Gold;
	if (name == "Silver") return Material::Silver;
	if (name == "BlackPlastic") return Material::BlackPlastic;
	if (name == "CyanPlastic") return Material::CyanPlastic;
	if (name == "GreenPlastic") return Material::GreenPlastic;
	if (name == "RedPlastic") return Material::RedPlastic;
	if (name == "WhitePlastic") return Material::WhitePlastic;
	if (name == "YellowPlastic") return Material::YellowPlastic;
	if (name == "BlackRubber") return Material::BlackRubber;
	if (name == "CyanRubber") return Material::CyanRubber;
	if (name == "GreenRubber") return Material::GreenRubber;
	if (name == "RedRubber") return Material::RedRubber;
	if (name == "WhiteRubber") return Material::WhiteRubber;
	if (name == "YellowRubber") return Material::YellowRubber;

	return Material::Zero;
}
*/
vector<string> splitOBJ(const string& str, char delimiter)
{
	vector<string> tokens;
	string token = "";
	for (char c : str)
	{
		if (c == delimiter)
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
		}
		else
		{
			token += c;
		}
	}
	if (!token.empty())
	{
		tokens.push_back(token);
	}
	return tokens;
}

int OBJ_3D_Model::objReader(string filename)
{
	string line;
	ifstream objFile(filename);
	while (getline(objFile, line))
	{
		vector<string> parts = splitOBJ(line, ' ');
		if (parts.size() == 0) continue;
		if (parts[0] == "v")
		{
			XMFLOAT3 v(stof(parts[1]), stof(parts[2]), stof(parts[3]));
			vertices.push_back(v);
		}
		else if (parts[0] == "vt")
		{
			//	cout << "Texture Vertex: " << endl;
			XMFLOAT3 vt(stof(parts[1]), stof(parts[2]), 0.0f);
			textureVertices.push_back(vt);
		}
		else if (parts[0] == "vn")
		{
			//	cout << "Vertex Normal: " << endl;
			XMFLOAT3 vn(stof(parts[1]), stof(parts[2]), stof(parts[3]));
			vertexNormals.push_back(vn);
		}
		else if (parts[0] == "f")
		{
			//	cout << "Face: " << endl;
			face f;
			f.vertexIndex.clear();
			f.textureIndex.clear();
			f.normalIndex.clear();
			for (int i = 1; i < parts.size(); i++)
			{
				vector<string> v = splitOBJ(parts[i], '/');
				f.vertexIndex.push_back((v.size() > 0) ? stoi(v[0])-1 : 0xFFFFFFFF);
				f.textureIndex.push_back((v.size() > 1) ? stoi(v[1])-1 : 0xFFFFFFFF);
				f.normalIndex.push_back((v.size() > 2) ? stoi(v[2])-1 : 0xFFFFFFFF);
			}
			currentObject->faces.push_back(f);
		}
		else if (parts[0] == "g")
		{
//			cout << "Group: " << parts[1] << endl;
			currentObject = new object();
			currentObject->name = parts[1];
			objects.push_back(currentObject);
			currentObject->mn = { 999999.0f,999999.0f,999999.0f,0 };
			currentObject->mx = { -999999.0f,-999999.0f,-999999.0f,0 };
		}
		else if (parts[0] == "o")
		{
//			cout << "Object: " << parts[1] << endl;
			currentObject = new object();
			currentObject->name = parts[1];
			currentObject->mn = { 999999.0f,999999.0f,999999.0f,0 };
			currentObject->mx = { -999999.0f,-999999.0f,-999999.0f,0 };

			objects.push_back(currentObject);
		}
		else if (parts[0] == "s")
		{
			cout << "smoothing: " << parts[1] << endl;
		}
		else if (parts[0] == "usemtl")
		{
//			cout << "Use Material: " << parts[1] << endl;
			string colorName = parts[1];
			string materialName = "";
			if (parts[1].find_first_of("_") != std::string::npos)
			{
				vector<string> desc = splitOBJ(parts[1], '_');
				colorName = desc[0];
				materialName = desc[1];
			}
			if (currentObject)
			{
				currentObject->material = materialName;
				currentObject->color = colorName;
			}
		}
		else
		{
			cout << "unknown type: " << parts[0] << endl;
		}

	}
	objFile.close();

	return 1;
}

std::shared_ptr<Mesh> OBJ_3D_Model::CreateMesh(string name, string color, const VertexCollection& vertices, const IndexCollection& indices, shared_ptr<CommandList> commandList, XMFLOAT4 mins, XMFLOAT4 maxs)
{
	if (vertices.empty())
	{
		return nullptr;
	}

	auto vertexBuffer = commandList->CopyVertexBuffer(vertices);
	auto indexBuffer = commandList->CopyIndexBuffer(indices);

	auto mesh = std::make_shared<Mesh>();
	// Create a default white material for new meshes.
	auto mat = Material::findMaterial((color=="")? "Red" : color);
	if (mat == nullptr)
	{
		mat = Material::findMaterial("Zero");
	}
//	mat->Reflectance = XMFLOAT4(0.7f, 0.2f, 0.2f, 1.0f);
//	mat->Ambient = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);	
//	mat->Diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
//	mat->Opacity = 1.0f;
	auto material = std::make_shared<Material>(*mat);
	auto v_mins = XMLoadFloat4(&mins);
	auto v_maxs = XMLoadFloat4(&maxs);
	BoundingBox aabb;
	BoundingBox::CreateFromPoints(aabb, v_mins, v_maxs);
	mesh->SetAABB(aabb);
	mesh->SetVertexBuffer(0, vertexBuffer);
	mesh->SetIndexBuffer(indexBuffer);
	mesh->SetMaterial(material);
	return mesh;
}

#endif

#if 0
void T38::Roll(float angle) {}
void T38::Pitch(float angle) {}
void T38::Yaw(float angle) {}
void T38::GearUp() {}
void T38::GearDown() {}
void T38::Flaps(FlapPosition Position) {}
void C130A::Roll(float angle) {}
void C130A::Pitch(float angle) {}
void C130A::Yaw(float angle) {}
void C130A::GearUp() {}
void C130A::GearDown() {}
void C130A::Flaps(FlapPosition Position) {}

#endif
