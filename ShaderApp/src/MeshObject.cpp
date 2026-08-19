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


void MeshObject::UpdateMeshVertices(shared_ptr<Mesh> mesh, XMMATRIX cameraMatrix)
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
	for (size_t i = 0; i < (groupStartIndex.size() - 1); i++)
	{
		XMFLOAT4 mins = { 999999,999999,999999,0 };
		XMFLOAT4 maxs = { -999999,-999999,-999999,0 };
		UINT start = groupStartIndex[i];
		UINT end = groupStartIndex[i + 1];
		for (UINT here = start; here < end; here++)
		{
			auto& idx = VertexIndex()[here];
			UINT i0 = idx.v_index.x;
			UINT i1 = idx.v_index.y;
			UINT i2 = idx.v_index.z;
			VertexType v0 = Vertices()[i0];
			VertexType v1 = Vertices()[i1];
			VertexType v2 = Vertices()[i2];
			v0 = TranslateXMFLOAT4(v0, World(), cameraMatrix, identity);
			v1 = TranslateXMFLOAT4(v1, World(), cameraMatrix, identity);
			v2 = TranslateXMFLOAT4(v2, World(), cameraMatrix, identity);
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
		}

//		auto vertexBuffer = commandList->CopyVertexBuffer(vertices);
//		auto indexBuffer = commandList->CopyIndexBuffer(indices);
	}

}



shared_ptr<SceneNode> CShaderAppDlg::CreateSceneMesh(string objectName, shared_ptr<CommandList> commandList)
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
	shared_ptr<Object> obj_ptr = make_shared<MeshObject>();
	obj_ptr = ObjectList[objectName];
	if (obj_ptr == nullptr) return nullptr; // continue;
	if (obj_ptr->Iam() != "MESH") return nullptr; //continue;
	shared_ptr<MeshObject> m = std::static_pointer_cast<MeshObject>(obj_ptr);
	string color = m->ColorName();
	auto node = std::make_shared<SceneNode>();
	node->SetName(objectName); // name);
	for (size_t i = 0; i < (m->groupStartIndex.size() - 1); i++)
	{
		XMFLOAT4 mins = { 999999,999999,999999,0 };
		XMFLOAT4 maxs = { -999999,-999999,-999999,0 };
		UINT start = m->groupStartIndex[i];
		UINT end = m->groupStartIndex[i + 1];
		for (UINT here = start; here < end; here++)
		{
			auto& idx = m->VertexIndex()[here];
			UINT i0 = idx.v_index.x;
			UINT i1 = idx.v_index.y;
			UINT i2 = idx.v_index.z;
			VertexType v0 = m->Vertices()[i0];
			VertexType v1 = m->Vertices()[i1];
			VertexType v2 = m->Vertices()[i2];
			v0 = TranslateXMFLOAT4(v0, m->World(), identity, identity);
			v1 = TranslateXMFLOAT4(v1, m->World(), identity, identity);
			v2 = TranslateXMFLOAT4(v2, m->World(), identity, identity);
//			v0.first.x /= 1000.0f;
//			v0.first.y /= 1000.0f;
//			v0.first.z /= 1000.0f;
//			v1.first.x /= 1000.0f;
//			v1.first.y /= 1000.0f;
//			v1.first.z /= 1000.0f;
//			v2.first.x /= 1000.0f;
//			v2.first.y /= 1000.0f;
//			v2.first.z /= 1000.0f;

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
		}

		auto vertexBuffer = commandList->CopyVertexBuffer(vertices);
		auto indexBuffer = commandList->CopyIndexBuffer(indices);
		auto mesh = make_shared<Mesh>();
		auto meshColor = m->groupColor[i];
		// Create a default white material for new meshes.
//		auto mat = Material::findMaterial((meshColor == "") ? "Red" : meshColor);
//		if (mat == nullptr)
//		{
//		}
		auto mat = Material::findMaterial("Zero");
		mat->Reflectance = XMFLOAT4(0.7f, 0.2f, 0.2f, 1.0f);
		mat->Ambient = meshColor;
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
		mesh->SetAABB(aabb);
		mesh->SetVertexBuffer(0, vertexBuffer);
		mesh->SetIndexBuffer(indexBuffer);
		mesh->SetMaterial(material);
		node->AddMesh(mesh);
		vertices.clear();
		vertexBuffer.reset();
		indices.clear();
		indexBuffer.reset();
	}
	return node;
}










inline bool areEqual(XMFLOAT4 a, XMFLOAT4 b)
{
	return (a.x == b.x) and (a.y == b.y) and (a.z == b.z);
}

XMUINT3 AddIndex(XMUINT3 target, uint32_t value, int index)
{
	if (index == 0) { target.x = value; }
	if (index == 1) { target.y = value; }
	if (index == 2) { target.z = value; }
	return target;
}

XMFLOAT4 ReorderVertex(XMFLOAT4 v, VertexExchange exchange = VertexExchange::XYZ)
{
	switch (exchange)
	{
		case XYZ:
			return v;
		case XZY:
			return XMFLOAT4(v.x, v.z, v.y, v.w);
		case YXZ:
			return XMFLOAT4(v.y, v.x, v.z, v.w);
		case YZX:
			return XMFLOAT4(v.y, v.z, v.x, v.w);
		case ZXY:
			return XMFLOAT4(v.z, v.x, v.y, v.w);
		case ZYX:
			return XMFLOAT4(v.z, v.y, v.x, v.w);
		default:
			return v;
	}
}

void OBJ_File_Mesh::JustifyVertices(vector<pair<XMFLOAT4, XMFLOAT4>>& vertices, XMFLOAT3 minVertex, XMFLOAT3 maxVertex, ImageJustify justification)
{
	if (justification == ImageJustify::NOTHING) return;
	// flip the image vertically if needed 
	if ((justification == CENTER_FLIP_IMAGE) or
		(justification == BOTTOM_FLIP_IMAGE) or
		(justification == TOP_FLIP_IMAGE) or
		(justification == FLIP_IMAGE))
	{
		for (auto& v : vertices)
		{
			v.first.y = (maxVertex.y - (v.first.y - minVertex.y));
		}
	}
	float dx = -(maxVertex.x + minVertex.x) / 2.0F; // move the center to 0,0,0
	float dy = 0; // move the center to 0,0,0
	float dz = -(maxVertex.z + minVertex.z) / 2.0F; // move the center to 0,0,0
	switch (justification)
	{
		case CENTER_IMAGE:
		case CENTER_FLIP_IMAGE:
			{
				dy = -(maxVertex.y + minVertex.y) / 2.0F; // move the center to 0,0,0
				break;
			}
		case BOTTOM_IMAGE:
		case BOTTOM_FLIP_IMAGE:
			{
				dy = 0 - minVertex.y; // move the center to 0,0,0
				break;
			}
		case TOP_IMAGE:
		case TOP_FLIP_IMAGE:
			{
				dy = 0 - maxVertex.y; // move the center to 0,0,0
				break;
			}
		default:
			return;
	}

	for (auto& v : vertices)
	{
		v.first.x += dx;
		v.first.y += dy;
		v.first.z += dz;
	}
}

int OBJ_File_Mesh::objReader(string filename, VertexExchange reverseYZ, ImageJustify justification)
{
	uint32_t minIndex = 999999999;
	XMFLOAT3 minVertex = { 999999.0f,999999.0f,999999.0f };
	XMFLOAT3 maxVertex = { -999999.0f,-999999.0f,-999999.0f };
	XMFLOAT4 rgb = FindColor(ColorName());
	string obj_name = filename;
	// list of the starting index of each group in the vertexIndex list. 
	// The last entry is the total number of indices.
	string line;
	ifstream objFile(filename + ".obj");
	Clear();
	while (getline(objFile, line))
	{
		vector<string> parts = split(line, ' ');
		if (parts.size() == 0) continue;
		if (parts[0] == "#") continue;
		if (parts[0] == "v")
		{
			XMFLOAT4 v = ReorderVertex({ stof(parts[1]),stof(parts[2]),stof(parts[3]), 1 }, reverseYZ);
			minVertex.x = min(minVertex.x, v.x);
			minVertex.y = min(minVertex.y, v.y);
			minVertex.z = min(minVertex.z, v.z);
			maxVertex.x = max(maxVertex.x, v.x);
			maxVertex.y = max(maxVertex.y, v.y);
			maxVertex.z = max(maxVertex.z, v.z);

			vertexList.push_back(make_pair(v, rgb));
		}
		else if (parts[0] == "vt")
		{
			XMFLOAT3 vt(stof(parts[1]), stof(parts[2]), (parts.size() == 4) ? stof(parts[3]) : 0);
			TextureList().push_back(vt);
		}
		else if (parts[0] == "vn")
		{
			XMFLOAT3 vn(stof(parts[1]), stof(parts[2]), stof(parts[3]));
			NormalList().push_back(vn);
		}
		else if (parts[0] == "f")
		{
			// mark the start of the first group if not already marked	
			if (groupStartIndex.empty())
			{
				groupStartIndex.push_back(0);
				groupName.push_back(obj_name);
				groupColor.push_back(rgb);
			}
			//	cout << "Face: " << endl;
			parts.erase(parts.begin()); // remove the "f" element so that only indices remain
			vector<XMUINT3> indexList = {}; // gather list of indices
			if (parts.size() == 3)
			{
				vector<string> s;
				s = split(parts[0], '/'); // split to get the elements
				uint32_t i0 = stoi(s[0]); // first element is the vertex index
				uint32_t t0 = stoi(s[1]); // first element is the vertex index
				uint32_t n0 = stoi(s[2]); // first element is the vertex index
				s.clear();
				s = split(parts[1], '/'); // split to get the elements
				uint32_t i1 = stoi(s[0]); // first element is the vertex index
				uint32_t t1 = stoi(s[1]); // first element is the vertex index
				uint32_t n1 = stoi(s[2]); // first element is the vertex index
				s.clear();
				s = split(parts[2], '/'); // split to get the elements
				uint32_t i2 = stoi(s[0]); // first element is the vertex index
				uint32_t t2 = stoi(s[1]); // first element is the vertex index
				uint32_t n2 = stoi(s[2]); // first element is the vertex index
				s.clear();
				minIndex = min(minIndex, min(i0, min(i1, i2)));
				vertexIndex.push_back({ {i0,i1,i2 },{t0,t1,t2},{n0,n1,n2} }); // save that to get a polygon of indices

			}
			else if (parts.size() == 4)
			{
				vector<string> s;
				s = split(parts[0], '/'); // split to get the elements
				uint32_t i0 = stoi(s[0]); // first element is the vertex index
				uint32_t t0 = stoi(s[1]); // first element is the vertex index
				uint32_t n0 = stoi(s[2]); // first element is the vertex index
				s.clear();
				s = split(parts[1], '/'); // split to get the elements
				uint32_t i1 = stoi(s[0]); // first element is the vertex index
				uint32_t t1 = stoi(s[1]); // first element is the vertex index
				uint32_t n1 = stoi(s[2]); // first element is the vertex index
				s.clear();
				s = split(parts[2], '/'); // split to get the elements
				uint32_t i2 = stoi(s[0]); // first element is the vertex index
				uint32_t t2 = stoi(s[1]); // first element is the vertex index
				uint32_t n2 = stoi(s[2]); // first element is the vertex index
				s.clear();
				s = split(parts[3], '/'); // split to get the elements
				uint32_t i3 = stoi(s[0]); // first element is the vertex index
				uint32_t t3 = stoi(s[1]); // first element is the vertex index
				uint32_t n3 = stoi(s[2]); // first element is the vertex index
				s.clear();
				minIndex = min(minIndex, min(i0, min(i1, min(i2, i3))));
				vertexIndex.push_back({ {i0,i1,i2},{t0,t1,t2},{n0,n1,n2} }); // save that to get a polygon of indices
				vertexIndex.push_back({ {i1,i2,i3},{t1,t2,t3},{n1,n2,n3} }); // save that to get a polygon of indices

			}
			else
			{
				for (auto part : parts)
				{
					vector<string> vi = split(part, '/'); // split to get the elements
					uint32_t index = stoi(vi[0]); // first element is the vertex index
					uint32_t texture = stoi(vi[1]); // second element is the texture index
					uint32_t normal = stoi(vi[2]); // first element is the normal index
					minIndex = min(minIndex, index);
					indexList.push_back({ index,texture,normal }); // save that to get a polygon of indices
				}
				auto indices = EarClipping(vertexList, indexList); // use ear clipping to create triangles from polygon
				vertexIndex.insert(vertexIndex.end(), indices.begin(), indices.end()); // attach to the end of the real index list
				indices.clear(); // clean up
				indexList.clear();
			}
		}
		else if (parts[0] == "g")
		{
			// mark start of index list for this object
			groupStartIndex.push_back((uint32_t)vertexIndex.size());
			groupName.push_back(parts[1]);
			groupColor.push_back(rgb);
			obj_name = parts[1];
		}
		else if (parts[0] == "o")
		{
			// mark start of index list for this object
			groupStartIndex.push_back((uint32_t)vertexIndex.size());
			groupName.push_back(parts[1]);
			groupColor.push_back(rgb);
			obj_name = parts[1];
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
				vector<string> desc = split(parts[1], '_');
				colorName = desc[0];
				materialName = desc[1];
				rgb = FindColor(colorName);
			}
			//			if (currentObject)
			//			{
			//				currentObject->material = materialName;
			//				currentObject->color = colorName;
			//			}
		}
		else
		{
			cout << "unknown type: " << parts[0] << endl;
		}

	}
	if (vertexIndex.size() > 0)
	{
		if (minIndex >= 1) // if the indices start at 1, convert them to start at 0
		{
			for (auto& v : vertexIndex)
			{
				v.v_index.x -= minIndex;
				v.v_index.y -= minIndex;
				v.v_index.z -= minIndex;
				v.t_index.x -= minIndex;
				v.t_index.y -= minIndex;
				v.t_index.z -= minIndex;
				v.n_index.x -= minIndex;
				v.n_index.y -= minIndex;
				v.n_index.z -= minIndex;
			}
		}
		groupStartIndex.push_back((uint32_t)vertexIndex.size());
		JustifyVertices(vertexList, minVertex, maxVertex, justification);
	}
	objFile.close();

	return 1;
}

bool MeshObject::Update(uint64_t frameNumber, XMMATRIX ViewMatrix)
{
	// update the world matrix

	// update the color
	if (colorControl.IsColorChanging())
	{
		return colorControl.StepGradientColor(frameNumber);
	}
	return colorControl.TimeToChangeColor(frameNumber);
}

#if 0


for (size_t i = 0; i < (m->groupStartIndex.size() - 1); i++)
{
	//			XMFLOAT4 mins = { 999999,999999,999999,0 };
	//			XMFLOAT4 maxs = { -999999,-999999,-999999,0 };
	UINT start = m->groupStartIndex[i];
	UINT end = m->groupStartIndex[i + 1];
	for (UINT here = start; here < end; here++)
	{
		auto& idx = m->VertexIndex()[here];
		UINT i0 = idx.v_index.x;
		UINT i1 = idx.v_index.y;
		UINT i2 = idx.v_index.z;
		VertexType v0 = vertices[i0];
		VertexType v1 = vertices[i1];
		VertexType v2 = vertices()[i2];
		v0 = TranslateXMFLOAT4(v0, m->World(), identity, identity);
		v1 = TranslateXMFLOAT4(v1, m->World(), identity, identity);
		v2 = TranslateXMFLOAT4(v2, m->World(), identity, identity);
		v0.first.x /= 1000.0f;
		v0.first.y /= 1000.0f;
		v0.first.z /= 1000.0f;
		v1.first.x /= 1000.0f;
		v1.first.y /= 1000.0f;
		v1.first.z /= 1000.0f;
		v2.first.x /= 1000.0f;
		v2.first.y /= 1000.0f;
		v2.first.z /= 1000.0f;

		mins.x = min(mins.x, min(v0.first.x, min(v1.first.x, v2.first.x)));
		maxs.x = max(maxs.x, max(v0.first.x, max(v1.first.x, v2.first.x)));
		mins.y = min(mins.y, min(v0.first.y, min(v1.first.y, v2.first.y)));
		maxs.y = max(maxs.y, max(v0.first.y, max(v1.first.y, v2.first.y)));
		mins.z = min(mins.z, min(v0.first.z, min(v1.first.z, v2.first.z)));
		maxs.z = max(maxs.z, max(v0.first.z, max(v1.first.z, v2.first.z)));

		XMFLOAT3 N = Normal(v0.first, v1.first, v2.first); // return the normal
		XMFLOAT3 T = Tangent(v0.first, v1.first); // return the normal

		VertexPositionNormalTangentBitangentTexture vpntbt0(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0));
		VertexPositionNormalTangentBitangentTexture vpntbt1(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0));
		VertexPositionNormalTangentBitangentTexture vpntbt2(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0));
		vpntbt0.Position = { v0.first.x ,v0.first.y,v0.first.z };
		vpntbt0.Normal = N;
		vpntbt0.Tangent = T;
		//				vpntbt0.TexCoord = (idx.t_index.x != 0xFFFFFFFF) ? m->TextureList()[idx.t_index.x] : XMFLOAT3(0, 0, 0);

		vpntbt1.Position = { v1.first.x ,v1.first.y,v1.first.z };
		vpntbt1.Normal = N;
		vpntbt1.Tangent = T;
		//				vpntbt1.TexCoord = (idx.t_index.y != 0xFFFFFFFF) ? m->TextureList()[idx.t_index.y] : XMFLOAT3(0, 0, 0);

		vpntbt2.Position = { v2.first.x ,v2.first.y,v2.first.z };
		vpntbt2.Normal = N;
		vpntbt2.Tangent = T;
		//				vpntbt2.TexCoord = (idx.t_index.z != 0xFFFFFFFF) ? m->TextureList()[idx.t_index.z] : XMFLOAT3(0, 0, 0);
		vertices.emplace_back(vpntbt0);
		vertices.emplace_back(vpntbt1);
		vertices.emplace_back(vpntbt2);
		indices.emplace_back(static_cast<uint32_t>(indices.size()));
		indices.emplace_back(static_cast<uint32_t>(indices.size()));
		indices.emplace_back(static_cast<uint32_t>(indices.size()));
	}



#endif