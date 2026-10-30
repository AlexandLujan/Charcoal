#include "stdafx.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <DirectXMath.h>
#include <DirectXMathConvert.inl>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <filesystem>
#include <fstream>
#include <ios>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <Windows.h>
namespace fs = std::filesystem;
#include "Const.h"
#include "Material.h"
#include "Utility.h"
#include "Object.h"

using namespace DirectX;
using namespace std;

extern std::unordered_map<std::string, std::shared_ptr<Material>> MaterialsMap;

/*
vector<shared_ptr<MeshWork>> Assembly::GenerateMeshList(XMMATRIX world)
{
	std::vector<std::shared_ptr<MeshWork>> meshList;
	for (auto& [name, component] : MeshObjectList)
	{
		auto m = component->GenerateMeshList(world);
		meshList.insert(meshList.end(), m.begin(), m.end());
	}
	return meshList;
}
*/

#include <afxstr.h>
#include <tchar.h>
#include <cstringt.h>

const float scale = 1.0f;
const float xoffset = 0.0f;
const float yoffset = 0.0f;
const float zoffset = 0.0f;

std::vector<XMFLOAT3>					vertexList;
std::vector<vector<XMUINT4>>			vertexIndex;
std::vector<XMFLOAT2>					textureList;
std::vector<XMFLOAT3>					normalList;
void DumpMaterialList();

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

XMFLOAT3 ReorderVertex(XMFLOAT3 v, VertexExchange exchange = VertexExchange::XYZ)
{
	switch (exchange)
	{
		case XYZ:
			return v;
		case XZY:
			return XMFLOAT3(v.x, v.z, v.y);
		case YXZ:
			return XMFLOAT3(v.y, v.x, v.z);
		case YZX:
			return XMFLOAT3(v.y, v.z, v.x);
		case ZXY:
			return XMFLOAT3(v.z, v.x, v.y);
		case ZYX:
			return XMFLOAT3(v.z, v.y, v.x);
		default:
			return v;
	}
}

void OBJ_File_Mesh::JustifyVertices(vector<XMFLOAT3>& vertices, XMFLOAT3 minVertex, XMFLOAT3 maxVertex, ImageJustify justification)
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
			v.y = (maxVertex.y - (v.y - minVertex.y));
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
		v.x += dx;
		v.y += dy;
		v.z += dz;
	}
}

bool IsWoundCW(UINT i0, UINT i1, UINT i2, vector<XMFLOAT3> vertexList)
{
// 1. Create vectors from the points
	XMFLOAT3 v1 = { vertexList[i0].x - vertexList[i1].x, vertexList[i0].y - vertexList[i1].y, vertexList[i0].z - vertexList[i1].z };
	XMFLOAT3 v2 = { vertexList[i2].x - vertexList[i1].x, vertexList[i2].y - vertexList[i1].y, vertexList[i2].z - vertexList[i1].z };

	// 2. Compute the actual surface normal using the cross product
	XMFLOAT3 actualNormal = CROSS(v1, v2);

	// 3. Use the dot product to compare the actual normal with the expected normal
//	float dot = DOT(actualNormal, { 0,0,1 });

//	return (dot > 0.0);
	return (actualNormal.z > 0.0);

}

UINT ReadVertices(string path, VertexExchange reverseXYZ, XMUINT3 mirror)
{
	UINT vertexCount = 0;
	ifstream objFile(path);
	objFile.seekg(0);
	string line;
	string groupName;
	int linenumber = 0;
	int vertexIndex = 1;
	float maxNumber = 0;
	CString st;
	XMMATRIX turn = XMMatrixRotationY(DegreesToRadians(90.0f));

	while (getline(objFile, line))
	{
		linenumber++;
		vector<string> parts = split(line, ' ');
		if (parts.size() == 0) continue;
		if (parts[0] == "#") continue;
		if ((parts[0] == "g") or (parts[0] == "o"))
		{
			groupName = parts[1];
			continue;
		}

		if (parts[0] == "v")
		{
			FLOAT x = stof(parts[1]);
			FLOAT y = stof(parts[2]);
			FLOAT z = stof(parts[3]);
			maxNumber = max(maxNumber, max(abs(x), max(abs(y), abs(z))));
			vertexIndex++;
			XMFLOAT3 v = ReorderVertex({ x,y,z }, reverseXYZ);
			if (false)
			{
				if (mirror.x) x = -x;
				if (mirror.y) y = -y;
				if (mirror.z) z = -z;
				XMFLOAT3 pt(x, y, z);
				XMVECTOR vec = XMLoadFloat3(&pt);
				vec = XMVector3Transform(vec, turn);
				XMStoreFloat3(&v, vec);
			}
			if (mirror.x) v.x = -v.x;
			if (mirror.y) v.y = -v.y;
			if (mirror.z) v.z = -v.z;
			vertexList.push_back(v);
			vertexCount++;
		}
		else if (parts[0] == "vt")
		{
			XMFLOAT2 vt = { stof(parts[1]), stof(parts[2]) };
			textureList.push_back(vt);
		}
		else if (parts[0] == "vn")
		{
			XMFLOAT3 vn(stof(parts[1]), stof(parts[2]), stof(parts[3]));
			normalList.push_back(vn);
		}
	}
	objFile.close();
	for (auto& v : vertexList)
	{
		v.x /= maxNumber;
		v.y /= maxNumber;
		v.z /= maxNumber;
	}
	return vertexCount;
}
std::string ToUpper(const std::string& str);
void DumpMaterialList(string objectName);

int OBJ_File_Mesh::objReader(string directoryname, string filename, VertexExchange reverseXYZ, ImageJustify justification, XMUINT3 mirror, BOOL woundCCW)
{
	string								groupName;
	shared_ptr<MeshObject>				component;
	bool skip = false;
	vertexList.reserve(70000);
	textureList.reserve(70000);
	normalList.reserve(70000);
	vertexIndex.reserve(10000);
	uint32_t trianglesGenerated = 0;
	uint32_t vertexOffset = 0;
	// list of the starting index of each group in the vertexIndex list.
	// The last entry is the total number of indices.
	string obj_name = filename;
	string path = ((directoryname.size() == 0) ? "" : (directoryname + "/")) + (filename + ".obj");
	UINT vertexCount = ReadVertices(path, reverseXYZ, mirror);
	ifstream objFile(path);
	objFile.clear(); // clear the EOF flag
	objFile.seekg(0); // rewind to the beginning of the file to read the faces and groups
	Clear();
	int linenumber = 0;
	string line;
	string materialName = "";
	while (getline(objFile, line))
	{
		linenumber++;
		vector<string> parts = split(line, ' ');
		if (parts.size() == 0) continue;
		if (parts[0] == "#") continue;
		if (parts[0] == "mtllib")
		{
			// open MTL file and create Materials used
			string mtlFileName = parts[1];
			LoadMTLFile(directoryname, mtlFileName);
//			DumpMaterialList("mtllib");

		}
		else if (parts[0] == "usemtl")
		{
			// remember material descriptor for this section
			vector<string> p = split(parts[1], " .");
			materialName = ToUpper(p[0]);
			/*
			if (groupName == "fuselage_fuselage.mesh") materialName = "GreenPlastic";
			if (groupName == "cover1") materialName = "ORANGE";
			if (groupName == "cover2") materialName = "BLUE";
			if (groupName == "sides_sides.mesh") materialName = "RED";
			if (groupName == "panel1") materialName = "BLUE";
			if (groupName == "panel2") materialName = "VIOLET";
			if (groupName == "echappes_echappes.mesh") materialName = "MAGENTA";
			if (groupName == "voletD_voletD.mesh") materialName = "DODGERBLUE";
			if (groupName == "voletG_voletG.mesh") materialName = "DODGERBLUE";
			//if (groupName == "canopyA_canopyA.mesh") materialName = "CORAL";
			//if (groupName == "canopyB_canopyB.mesh") materialName = "CORAL";
			//if (groupName == "intcanopyA_intcanopyA.mesh") materialName = "DARKORANGE";
			//if (groupName == "intcanopyB_intcanopyB.mesh") materialName = "GOLD";
			//if (groupName == "intvitrecanopyA_intvitrecanopyA.mesh") materialName = "INDIGO";
			//if (groupName == "intvitrecanopyB_intvitrecanopyB.mesh") materialName = "KHAKI";
			//if (groupName == "vitrecanopyA_vitrecanopyA.mesh") materialName = "LAVENDER";
			//if (groupName == "vitrecanopyB_vitrecanopyB.mesh") materialName = "MAROON";
			if (groupName == "planchet_planchet.mesh") materialName = "RED";

			if (groupName == "ailes_ailes.mesh") materialName = "LIGHTGOLDENRODYELLOW";
			if (groupName == "ailes2_ailes2.mesh") materialName = "LIGHTGOLDENRODYELLOW";
			if (groupName == "direction_direction.mesh") materialName = "INDIANRED";

			if (groupName == "airs_airs.mesh") materialName = "SEAGREEN";
			if (groupName == "aileronD_aileronD.mesh") materialName = "CORNSILK";
			if (groupName == "aileronG_aileronG.mesh") materialName = "CORNSILK";
			if (groupName == "slatD_slatD.mesh") materialName = "LIGHTGREEN";
			if (groupName == "slatG_slatG.mesh") materialName = "LIGHTGREEN";
			if (groupName == "spoilerD_spoilerD.mesh") materialName = "LIGHTSALMON";
			if (groupName == "spoilerG_spoilerG.mesh") materialName = "LIGHTSALMON";
			if (groupName == "tanks_tanks.mesh") materialName = "LIGHTSKYBLUE";
			if (groupName == "beacon_beacon.mesh") materialName = "RED";
			if (groupName == "interieur_interieur.mesh") materialName = "CYANPLASTIC";
			if (groupName == "inttourvitres_inttourvitres.mesh") materialName = "DEEPPINK";
			if (groupName == "intvitres_intvitres.mesh") materialName = "GOLDENROD";
			if (groupName == "axeGB_axeGB.mesh") materialName = "LIGHTSKYBLUE";
			if (groupName == "axeGH_axeGH.mesh") materialName = "GOLDENROD";
			if (groupName == "roueG_roueG.mesh") materialName = "BlackRubber";
			if (groupName == "roueD_roueD.mesh") materialName = "BlackRubber";
			if (groupName == "roueA_roueA.mesh") materialName = "BlackRubber";

			if (groupName.contains("fond")) materialName = "SLATEBLUE";

			if (groupName == "Glass") materialName = "BlackRubber";
			if (groupName == "Glass001") materialName = "BlackRubber";
			if (groupName == "Glass002") materialName = "BlackRubber";
			if (groupName == "Interior") materialName = "RED"; // flight deck
			if (groupName == "Main_Body") materialName = "BLUE";
			if (groupName == "Main_Body001") materialName = "COPPER"; // port nose gear door
			if (groupName == "Main_Body002") materialName = "SILVER"; // starboard nose gear door
			if (groupName == "Wings") materialName = "RED"; //"GOLDENROD"; // tail and wheel fairings
			if (groupName == "Wings001") materialName = "LIGHTSKYBLUE"; // prop 4
			if (groupName == "Wings002") materialName = "CYANPLASTIC"; // prop 1
			if (groupName == "Wings003") materialName = "DEEPPINK"; // prop 2
			if (groupName == "Wings004") materialName = "ORANGE"; // prop 3
			if (groupName == "Wings005") materialName = "INDIGO"; // nose landing gear
			if (groupName == "Wings006") materialName = "INDIANRED"; // port landing gear
			if (groupName == "Wings007") materialName =  "KHAKI"; // starboard landing gear
			if (groupName == "Wings008") materialName = "GREEN"; // wings and engines
			if (groupName == "Wings009") materialName = "VIOLET"; // starboard aileron
			if (groupName == "Wings010") materialName = "GOLD"; // port aileron
			if (groupName == "Wings011") materialName = "CORAL"; // port elevator
			if (groupName == "Wings012") materialName = "DODGERBLUE"; // starboard elevator
			if (groupName == "Wings013") materialName = "MAROON"; // rudder
			if (groupName == "Wings014") materialName = "DARKORANGE"; // starboard landing gea door
			if (groupName == "Wings015") materialName = "CORNSILK"; // starboard inner landing gea door
			if (groupName == "Wings016") materialName = "LIGHTGREEN"; // port outer landing gear door
			if (groupName == "Wings017") materialName = "LIGHTSALMON"; port inner landing gear door
			materialName = ToUpper(materialName);
			*/

			if (not Material::DoesMaterialExist(materialName))
			{
				MaterialProperties materialProperties;
				materialProperties.Emissive = FindColor(materialName);
				materialProperties.Emissive.w = 1.0f;
				materialProperties.Opacity = 1.0f;
				Material::AddMaterial(materialName, materialProperties);
			}
			if (component != nullptr) component->MaterialName() = materialName;

		}
		else if ((parts[0] == "g") or (parts[0] == "o"))
		{
			if (component != nullptr)
			{
				if (vertexIndex.size() > 0)
				{
//					JustifyVertices(vertexList, minVertex, maxVertex, justification);
					for (auto it = vertexIndex.begin(); it != vertexIndex.end(); ++it)
					{
						vector<XMUINT4> v = *it;
						UINT i0 = component->findVertex(vertexList[v[0].x]);
						UINT i1 = component->findVertex(vertexList[v[1].x]);
						UINT i2 = component->findVertex(vertexList[v[2].x]);
						UINT t0 = component->findTexture(textureList[v[0].y]);
						UINT t1 = component->findTexture(textureList[v[1].y]);
						UINT t2 = component->findTexture(textureList[v[2].y]);
						UINT n0 = component->findNormal(normalList[v[0].z]);
						UINT n1 = component->findNormal(normalList[v[1].z]);
						UINT n2 = component->findNormal(normalList[v[2].z]);
						component->VertexIndex().push_back({ { i0,t0,n0,0 }, {i1,t1,n1,0}, {i2,t2,n2,0} });
					}
					component->groupStartIndex = { 0,(UINT)vertexIndex.size() };
					component->groupName = groupName;
					AddComponent(groupName, component);
				}
				component = nullptr;
			}
			// mark start of index list for this object
			groupName = parts[1];
			vertexIndex.clear();
			skip = false;
			if (!UseMeshName(groupName)) // make sure that the mesh is in the keep list
			{
				skip = true;
				component = nullptr;
				continue;
			}
			component = make_shared<MeshObject>(groupName, materialName);
		}
		//else if (parts[0] == "v")
		//{
		//	XMFLOAT3 v = ReorderVertex({ (stof(parts[1]) * scale) + xoffset,
		//								(stof(parts[2]) * scale) + yoffset,
		//								(stof(parts[3]) * scale) + zoffset },
		//								reverseYZ);
		//	minVertex.x = min(minVertex.x, v.x);
		//	minVertex.y = min(minVertex.y, v.y);
		//	minVertex.z = min(minVertex.z, v.z);
		//	maxVertex.x = max(maxVertex.x, v.x);
		//	maxVertex.y = max(maxVertex.y, v.y);
		//	maxVertex.z = max(maxVertex.z, v.z);

		//	vertexList.push_back(v);
		//}
		//else if (parts[0] == "vt")
		//{
		//	XMFLOAT2 vt = { stof(parts[1]), stof(parts[2]) };
		//	textureList.push_back(vt);
		//}
		//else if (parts[0] == "vn")
		//{
		//	XMFLOAT3 vn(stof(parts[1]), stof(parts[2]), stof(parts[3]));
		//	normalList.push_back(vn);
		//}

		else if (parts[0] == "f")
		{
			if (skip) continue;

			// mark the start of the first group if not already marked
			//	cout << "Face: " << endl;
			parts.erase(parts.begin()); // remove the "f" element so that only indices remain
			vector<XMUINT4> indexList = {}; // gather list of indices

			for (auto part : parts)
			{
				vector<string> vi = split(part, '/'); // split to get the elements
				uint32_t index = stoi(vi[0]) - 1; // first element is the vertex index
				uint32_t texture = stoi(vi[1]) - 1; // second element is the texture index
				uint32_t normal = stoi(vi[2]) - 1; // third element is the normal index
				indexList.push_back({ index,texture,normal, 0 }); // save that to get a polygon of indices
			}
			if (parts.size() == 3)
			{
				if (not woundCCW) // changing order of coordinates
				{
					vertexIndex.push_back({ indexList[0], indexList[2], indexList[1] });
				}
				else
				{
					vertexIndex.push_back(indexList);
				}
				trianglesGenerated++;
				continue;
			}
			if (parts.size() == 4)
			{
				if (not woundCCW) // changing order of coordinates
				{
					vertexIndex.push_back({ indexList[0], indexList[2], indexList[1] });
					vertexIndex.push_back({ indexList[0], indexList[3], indexList[2] });
				}
				else
				{
				// good
					vertexIndex.push_back({ indexList[0], indexList[1], indexList[2] });
					vertexIndex.push_back({ indexList[0], indexList[2], indexList[3] });
				}
				trianglesGenerated += 2;
				continue;
			}
			trianglesGenerated += (UINT)parts.size() - 2;
			{
				UINT i0 = indexList[0].x;
				vector<XMUINT4> oI = {};
				for (int i = 1; i < (int)parts.size() - 1; i++)
				{
					UINT i1 = indexList[i].x;
					UINT i2 = indexList[i + 1].x;
					XMFLOAT3 v0 = vertexList[i0];
					XMFLOAT3 v1 = vertexList[i1];
					XMFLOAT3 v2 = vertexList[i2];
					if (not woundCCW) // changing order of coordinates
					{
						oI = { indexList[0], indexList[i + 1], indexList[i] };
					}
					else
					{
						oI = { indexList[0], indexList[i], indexList[i + 1] };
					}
					vertexIndex.push_back(oI);
					oI.clear();
				}
			}
		}
		else if (parts[0] == "s")
		{
		}
		else // new one slipping through
		{
		}

	}
	if (component != nullptr)
	{
		if (vertexIndex.size() > 0)
		{
//			JustifyVertices(vertexList, minVertex, maxVertex, justification);
			for (auto it = vertexIndex.begin() + vertexOffset; it != vertexIndex.end(); ++it)
			{
				vector<XMUINT4> v = *it;
				UINT i0 = component->findVertex(vertexList[v[0].x]);
				UINT i1 = component->findVertex(vertexList[v[1].x]);
				UINT i2 = component->findVertex(vertexList[v[2].x]);
				UINT t0 = component->findTexture(textureList[v[0].y]);
				UINT t1 = component->findTexture(textureList[v[1].y]);
				UINT t2 = component->findTexture(textureList[v[2].y]);
				UINT n0 = component->findNormal(normalList[v[0].z]);
				UINT n1 = component->findNormal(normalList[v[1].z]);
				UINT n2 = component->findNormal(normalList[v[2].z]);
				component->VertexIndex().push_back({ { i0,t0,n0,0 }, {i1,t1,n1,0}, {i2,t2,n2,0} });
			}
			component->groupStartIndex = { 0,(UINT)vertexIndex.size() };
			component->groupName = groupName;
			AddComponent(groupName, component);
		}
		component = nullptr;
	}
	objFile.close();

	vertexList.clear();
	textureList.clear();
	normalList.clear();
	vertexIndex.clear();

	return 1;
}

/*
		newmtl DefaultWhite
		Ns 96.078431
		Ka 1.000000 1.000000 1.000000
		Kd 0.800000 0.800000 0.800000
		Ks 0.250000 0.250000 0.250000
		Ke 0.000000 0.000000 0.000000
		Ni 1.000000
		d 1.000000
		illum 2
		map_Kd / home / helijah / Essais - SL / FlightGear - Blender / Northrop - T38 - Talon / Models / texture.png

newmtl Glass
Tr 0.0000 // transparency same as d
Tf 1.0000 1.0000 1.0000 // transmission filter
illum 1
Pr 0.2928932309150696 // roughness
Pds 1 // diffuse roughness
Pl 1 // light decay
Ka 1 1 1 //ambient color
Kd 1 1 1 //diffuse color
Ke 0 0 0 //emissive color
Ks 0 0 0 //specular color
*/

int OBJ_File_Mesh::LoadMTLFile(string directoryname, const string mtlFileName)
{
	string line;
	string path = ((directoryname.size() == 0) ? "" : (directoryname + "/")) + mtlFileName;

	ifstream mtlFile(path);
	string materialName = "";
	shared_ptr<Material> NewMaterial = nullptr;
	int material_count = 0;
	while (getline(mtlFile, line))
	{
		vector<string> parts = split(line, " ");
		if (parts.size() == 0) continue;
		if (parts[0] == "#") continue;
		if (parts[0] == "newmtl")
		{
			vector<string> p = split(parts[1], ".");
			materialName = ToUpper(p[0]);
			// create a new material with this name
			if (MaterialsMap.contains(materialName))
			{
				NewMaterial = MaterialsMap[materialName];
				continue;
			}
			MaterialsMap[materialName] = make_shared<Material>(materialName);
			NewMaterial = MaterialsMap[materialName];
			material_count++;
			continue;
		}
		// need to have an active material 
		if (NewMaterial == nullptr) continue;
		if (parts[0] == "Ns")
		{
			XMFLOAT4 shininess(stof(parts[1]), stof(parts[1]), stof(parts[1]), 1);
			NewMaterial->Reflectance() = shininess.x;
		}
		else if (parts[0] == "Ka")
		{
			XMFLOAT4 ambient(stof(parts[1]), stof(parts[2]), stof(parts[3]), 1);
			NewMaterial->Ambient() = ambient.x;
		}
		else if (parts[0] == "Kd")
		{
			XMFLOAT4 diffuse(stof(parts[1]), stof(parts[2]), stof(parts[3]), 1);
			NewMaterial->Diffuse() = diffuse;
		}
		else if (parts[0] == "Ks")
		{
			XMFLOAT4 specular(stof(parts[1]), stof(parts[2]), stof(parts[3]), 1);
			NewMaterial->Specular() = specular;
		}
		else if (parts[0] == "Ke")
		{
			XMFLOAT4 emissive(stof(parts[1]), stof(parts[2]), stof(parts[3]), 1);
			NewMaterial->Emissive() = emissive;
		}
		else if (parts[0] == "Ni")
		{
			float ior = stof(parts[1]);
			NewMaterial->IndexOfRefraction() = ior;
		}
		else if ((parts[0] == "d") or (parts[0] == "Tr"))
		{
			float alpha = stof(parts[1]);
			NewMaterial->Emissive().w = alpha;
			NewMaterial->Opacity() = alpha;
		}
		else if (parts[0] == "illum")
		{
			int illumination = stoi(parts[1]);
			// not sure what to do with this
		}
		else if (parts[0] == "map_Kd")
		{
			fs::path texturePath = parts[1];
			string textureFileName = texturePath.filename().string();
			NewMaterial->TextureFileName() = textureFileName;
			NewMaterial = nullptr;
		}
	}
#if 0
	int offset = 0;
	for (auto it = MaterialsMap.begin(); it != MaterialsMap.end(); ++it)
	{
		const auto& [name, mat] = *it;
		CString st;
		st.Format(_T("%3d (index %3d) Material %s\r\n\tEmissive: %6.4f,%6.4f,%6.4f\r\n\tDiffuse: %6.4f,%6.4f,%6.4f\r\n\tAmbient: %6.4f,%6.4f,%6.4f\r\n"),
					 offset, Material::FindMaterialIndex(name), name.c_str(),
					 mat->Emissive().x, mat->Emissive().y, mat->Emissive().z,
					 mat->Diffuse().x, mat->Diffuse().y, mat->Diffuse().z,
					 mat->Ambient().x, mat->Ambient().y, mat->Ambient().z);
		OutputDebugString(st);
		offset++;
	}
#endif
	return material_count;
}
extern std::string RandomColor();

RoundTunnel::RoundTunnel(std::string tunnel_name, float diameter, float length, float ringWidth, float tilesPerSegment) :
	Object(tunnel_name), diameter(diameter), length(length), ringWidth(ringWidth), tilesPerRing(tilesPerSegment)
{
	UINT numberOfRings = (UINT)floor(length / ringWidth); // how many rings from front to back
	UINT tileHeight = (UINT)floor((XM_2PI * diameter) / (float)tilesPerRing);
	float TileStepSizeInRadians = (XM_2PI / tilesPerRing);
	int id = 0;
	for (UINT i = 0; i < numberOfRings; i++)
	{
		float z = i * ringWidth;
		for (int j = 0; j < tilesPerRing; j++)
		{
			float angle = (j * TileStepSizeInRadians);
			XMFLOAT3 location = XMFLOAT3((diameter / 2.0f) * cosf(angle), (diameter / 2.0f) * sinf(angle), z);
			std::shared_ptr<MeshObject> segment = CreateBox(i, j, .002f, tileHeight * 0.50f, ringWidth * 0.9f, angle, location);
			AddComponent(segment->Name(), segment);
		}
	}
}

shared_ptr<MeshObject> Object::CreateBox(int ring, int id, float width, float height, float depth, float rollAngle, XMFLOAT3 CenterPoint)
{
	XMMATRIX identity = DirectX::XMMatrixIdentity();
	XMMATRIX matRotateZ = DirectX::XMMatrixRotationZ(rollAngle);
	XMMATRIX matTranslate = DirectX::XMMatrixTranslation(CenterPoint.x, CenterPoint.y, CenterPoint.z);
//	XMMATRIX worldMatrix = matTranslate * matRotateZ;
	XMMATRIX worldMatrix = matRotateZ * matTranslate;
	string boxColor = RandomColor();
	float dWidth = width / 2.0f;
	float dHeight = height / 2.0f;
	float dDepth = depth / 2.0f;
	XMFLOAT3 ruf(dWidth, dHeight, dDepth);
	XMFLOAT3 rlf(dWidth, -dHeight, dDepth);
	XMFLOAT3 luf(-dWidth, dHeight, dDepth);
	XMFLOAT3 llf(-dWidth, -dHeight, dDepth);
	XMFLOAT3 rur(dWidth, dHeight, -dDepth);
	XMFLOAT3 rlr(dWidth, -dHeight, -dDepth);
	XMFLOAT3 lur(-dWidth, dHeight, -dDepth);
	XMFLOAT3 llr(-dWidth, -dHeight, -dDepth);
	vector<XMFLOAT3> vertexList = { ruf,rlf,luf,llf,rur,rlr,lur,llr };
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
	std::vector<XMUINT3> vi = { i00,i01,i02,i03,i04,i05,i06,i07,i08,i09,i10,i11 };
	vertexList = TranslateXMFLOAT3(vertexList, worldMatrix, identity, identity);
	shared_ptr<MeshObject> box = make_shared<MeshObject>("tile_" + std::to_string(ring) + "_" + to_string(id), boxColor);
	box->Vertices() = vertexList;
//	box->VertexIndex() = vi;
	box->groupStartIndex = { 0,(UINT)vi.size() };
	return box;
}

TriangleTunnel::TriangleTunnel(std::string tunnel_name, float base, float height, float length, float segmentWidth) :
	Object(tunnel_name), base(base), height(height), length(length), segmentWidth(segmentWidth)
{}

#ifdef OLDSTUFF




static bool showThis(string name)
{
	for (auto& s : showThese)
	{
		if (s == name)
			return true;
	}
	return false;
}

const string SkipList[] =
{
	// TEMPORARY
	// PERMANENT
"AttitudeBall.001_AttitudeBall.mesh.001",
"AttitudeBall_AttitudeBall.mesh",
"HSI.001_HSI.mesh.001",
"HSI_HSI.mesh",
"SkyPointer.001_SkyPointer.mesh.001",
"SkyPointer_SkyPointer.mesh",
"VSI.fond.001_VSI.fond.mesh.001",
"VSI.fond_VSI.fond.mesh",

"course-flag.001_course-flag.mesh.001",
"course-flag_course-flag.mesh",
"button.001_button.mesh.001",
"button_button.mesh",
"buttons.001_buttons.mesh.001",
"buttons_buttons.mesh",
"ai-ring.001_ai-ring.mesh.001",
"ai-ring_ai-ring.mesh",
"bank-index.001_bank-index.mesh.001",
"bank-index.002_bank-index.mesh.002",
"bank-index.003_bank-index.mesh.003",
"bank-index_bank-index.mesh",
"nav-pitch-steering-ba.001_nav-pitch-steering-ba.mesh.001",
"nav-pitch-steering-ba_nav-pitch-steering-ba.mesh",
"nav-yaw-steering-bar.001_nav-yaw-steering-bar.mesh.001",
"nav-yaw-steering-bar_nav-yaw-steering-bar.mesh",
// these might be tires
/*
"barre.001_barre.mesh.001",
"barre1.001_barre1.mesh.001",
"barre1_barre1.mesh",
"barre2.001_barre2.mesh.001",
"barre2_barre2.mesh",
"barre_barre.mesh",
"bkg.001_bkg.mesh.001",
"bkg_bkg.mesh",
"bouton.001_bouton.mesh.001",
"bouton_bouton.mesh",
"drum.001_drum.mesh.001",
"drum_drum.mesh",
"face.001_face.mesh.001",
"face.002_face.mesh.002",
"face.003_face.mesh.003",
"face.004_face.mesh.004",
"face.005_face.mesh.005",
"face_face.mesh",
"cacheL.001_cacheL.mesh.001",
"cacheL_cacheL.mesh",
"Fond.001_Fond.mesh.001",
"Fond_Fond.mesh",
"fond.001_fond.mesh.001",
"fond.002_fond.mesh.002",
"fond.003_fond.mesh.003",
"fond.004_fond.mesh.004",
"fond.005_fond.mesh.005",
"fond.006_fond.mesh.006",
"fond.007_fond.mesh.007",
"fond.008_fond.mesh.008",
"fond.009_fond.mesh.009",
"fond.010_fond.mesh.010",
"fond.011_fond.mesh.011",
"fond.012_fond.mesh.012",
"fond.013_fond.mesh.013",
"fond.014_fond.mesh.014",
"fond.015_fond.mesh.015",
"fond.016_fond.mesh.016",
"fond.017_fond.mesh.017",
"fond.018_fond.mesh.018",
"fond.019_fond.mesh.019",
"fond.020_fond.mesh.020",
"fond.021_fond.mesh.021",
"fond.022_fond.mesh.022",
"fond.023_fond.mesh.023",
"fond.024_fond.mesh.024",
"fond.025_fond.mesh.025",
"fond.026_fond.mesh.026",
"fond.027_fond.mesh.027",
"fond.028_fond.mesh.028",
"fond.029_fond.mesh.029",
"fond.030_fond.mesh.030",
"fond.031_fond.mesh.031",
"fond.032_fond.mesh.032",
"fond.033_fond.mesh.033",
"fond.034_fond.mesh.035",
"fond.035_fond.mesh.036",
"fond.036_fond.mesh.034",
"fond.037_fond.mesh.037",
"fond.042_fond.mesh.042",
"fond.043_fond.mesh.043",
"fond_fond.mesh",
"rect.001_rect.mesh.001",
"rect_rect.mesh",
"trous_trous.mesh",
*/
"gearlevel.001_gearlevel.mesh.001",
"gearlevel_gearlevel.mesh",
"gs-scale.001_gs-scale.mesh.001",
"gs-scale_gs-scale.mesh",
"ils-flag.001_ils-flag.mesh.001",
"ils-flag_ils-flag.mesh",
"ring-slip.001_ring-slip.mesh.001",
"ring-slip_ring-slip.mesh",
"ring.001_ring.mesh.001",
"ring_ring.mesh",
"slip-ball.001_slip-ball.mesh.001",
"slip-ball_slip-ball.mesh",
"scale.001_scale.mesh.001",
"scale_scale.mesh",

"vitre.001_vitre.mesh.001",
"vitre.002_vitre.mesh.002",
"vitre.003_vitre.mesh.003",
"vitre.004_vitre.mesh.004",
"vitre.005_vitre.mesh.005",
"vitre.006_vitre.mesh.006",
"vitre.007_vitre.mesh.007",
"vitre.008_vitre.mesh.008",
"vitre.009_vitre.mesh.009",
"vitre.010_vitre.mesh.010",
"vitre.011_vitre.mesh.011",
"vitre.012_vitre.mesh.012",
"vitre.013_vitre.mesh.013",
"vitre.014_vitre.mesh.014",
"vitre.015_vitre.mesh.015",
"vitre.016_vitre.mesh.016",
"vitre.017_vitre.mesh.017",
"vitre.018_vitre.mesh.018",
"vitre.019_vitre.mesh.019",
"vitre.020_vitre.mesh.020",
"vitre.021_vitre.mesh.021",
"vitre.022_vitre.mesh.022",
"vitre.023_vitre.mesh.023",
"vitre.024_vitre.mesh.024",
"vitre.025_vitre.mesh.025",
"vitre.026_vitre.mesh.026",
"vitre.027_vitre.mesh.027",
"vitre.028_vitre.mesh.028",
"vitre.029_vitre.mesh.029",
"vitre.030_vitre.mesh.030",
"vitre.031_vitre.mesh.031",
"vitre.032_vitre.mesh.032",
"vitre.033_vitre.mesh.033",
"vitre.034_vitre.mesh.034",
"vitre.035_vitre.mesh.035",
"vitre.036_vitre.mesh.036",
"vitre.037_vitre.mesh.037",
"vitre.038_vitre.mesh.038",
"vitre.039_vitre.mesh.039",
"vitre.040_vitre.mesh.040",
"vitre.041_vitre.mesh.041",
"vitre.042_vitre.mesh.042",
"vitre.043_vitre.mesh.043",
"vitre.044_vitre.mesh.044",
"vitre.045_vitre.mesh.045",
"vitre.046_vitre.mesh.046",
"vitre.047_vitre.mesh.047",
"vitre_vitre.mesh"

};
bool SkipThisGroup(string groupName)
{
	for (const auto& name : SkipList)
	{
		if (groupName == name) return true;
	}
	return false;
}
std::string showThese[] =
{
#if 1
/*
"panel.003",
"panel1",
"panel2",
"plane.001_plane.mesh.001",
"plane_plane.mesh",
*/

"fuselage_fuselage.mesh",
"cover1",
"cover2",
// "holes" Wheel wells / speed brake wells / inlets
"trous_trous.mesh",
// Flaps
"voletD_voletD.mesh",
"voletG_voletG.mesh",
// ailerons
"aileronD_aileronD.mesh",
"aileronG_aileronG.mesh",
// engine nozzlees
"echappes_echappes.mesh",
// idk
"planchet_planchet.mesh",

// landing gear doors
// landing gear struts
// tires or wheels
//
// front
"porteA1_porteA1.mesh",
"porteA2_porteA2.mesh",
"axeAB_axeAB.mesh",
"axeAH_axeAH.mesh",
"roueA_roueA.mesh",
// right
"porteD1_porteD1.mesh",
"porteD2_porteD2.mesh",
"axeDB_axeDB.mesh",
"axeDH_axeDH.mesh",
"roueD_roueD.mesh",
// left
"porteG1_porteG1.mesh",
"porteG2_porteG2.mesh",
"axeGB_axeGB.mesh",
"axeGH_axeGH.mesh",
"roueG_roueG.mesh",
// elevators
"ailes2_ailes2.mesh",
"ailes_ailes.mesh",
// rudder
"direction_direction.mesh",

"antennes_antennes.mesh",
"beacon_beacon.mesh",
"seat.001_seat.mesh.001",
"seat_seat.mesh",
// cockpit side panels
"sides_sides.mesh",
// interior panel under front glass
"intvitres_intvitres.mesh",
// not sure
//"interieur_interieur.mesh",
// pitot tube // landing light
"inttourvitres_inttourvitres.mesh",
// tip tanks
//"tanks_tanks.mesh",
// leading edge
"slatD_slatD.mesh",
"slatG_slatG.mesh",
// speed brake
"spoilerD_spoilerD.mesh",
"spoilerG_spoilerG.mesh",
// canopy support rings
"support_support.mesh",
// glass
"vitres_vitres.mesh",
// canopy
"canopyA_canopyA.mesh",
"canopyB_canopyB.mesh",
"intcanopyA_intcanopyA.mesh",
"intcanopyB_intcanopyB.mesh",
"intvitrecanopyA_intvitrecanopyA.mesh",
"intvitrecanopyB_intvitrecanopyB.mesh",
"vitrecanopyA_vitrecanopyA.mesh",
"vitrecanopyB_vitrecanopyB.mesh",
"",

// TEMPORARY

#else
"drum.001_drum.mesh.001",
"drum_drum.mesh",
"face.001_face.mesh.001",
"face.002_face.mesh.002",
"face.003_face.mesh.003",
"face.004_face.mesh.004",
"face.005_face.mesh.005",
"face_face.mesh",
"cacheL.001_cacheL.mesh.001",
"cacheL_cacheL.mesh",
"bkg.001_bkg.mesh.001",
"bkg_bkg.mesh",
"bouton.001_bouton.mesh.001",
"bouton_bouton.mesh",
"rect.001_rect.mesh.001",
"rect_rect.mesh",

"AttitudeBall.001_AttitudeBall.mesh.001",
"AttitudeBall_AttitudeBall.mesh",
"Fond.001_Fond.mesh.001",
"Fond_Fond.mesh",
"HSI.001_HSI.mesh.001",
"HSI_HSI.mesh",
"SkyPointer.001_SkyPointer.mesh.001",
"SkyPointer_SkyPointer.mesh",
"VSI.fond.001_VSI.fond.mesh.001",
"VSI.fond_VSI.fond.mesh",
"bank-index.001_bank-index.mesh.001",
"bank-index.002_bank-index.mesh.002",
"bank-index.003_bank-index.mesh.003",
"bank-index_bank-index.mesh",
"direction_direction.mesh",
"nav-pitch-steering-ba.001_nav-pitch-steering-ba.mesh.001",
"nav-pitch-steering-ba_nav-pitch-steering-ba.mesh",
"nav-yaw-steering-bar.001_nav-yaw-steering-bar.mesh.001",
"nav-yaw-steering-bar_nav-yaw-steering-bar.mesh",
"drum.001_drum.mesh.001",
"drum_drum.mesh",
"face.001_face.mesh.001",
"face.002_face.mesh.002",
"face.003_face.mesh.003",
"face.004_face.mesh.004",
"face.005_face.mesh.005",
"face_face.mesh",
"cacheL.001_cacheL.mesh.001",
"cacheL_cacheL.mesh",
"course-flag.001_course-flag.mesh.001",
"course-flag_course-flag.mesh",
"barre.001_barre.mesh.001",
"barre1.001_barre1.mesh.001",
"barre1_barre1.mesh",
"barre2.001_barre2.mesh.001",
"barre2_barre2.mesh",
"barre_barre.mesh",
"bkg.001_bkg.mesh.001",
"bkg_bkg.mesh",
"bouton.001_bouton.mesh.001",
"bouton_bouton.mesh",
"button.001_button.mesh.001",
"button_button.mesh",
"buttons.001_buttons.mesh.001",
"buttons_buttons.mesh",
"ai-ring.001_ai-ring.mesh.001",
"ai-ring_ai-ring.mesh",

// these might be tires
"fond.001_fond.mesh.001",
"fond.002_fond.mesh.002",
"fond.003_fond.mesh.003",
"fond.004_fond.mesh.004",
"fond.005_fond.mesh.005",
"fond.006_fond.mesh.006",
"fond.007_fond.mesh.007",
"fond.008_fond.mesh.008",
"fond.009_fond.mesh.009",
"fond.010_fond.mesh.010",
"fond.011_fond.mesh.011",
"fond.012_fond.mesh.012",
"fond.013_fond.mesh.013",
"fond.014_fond.mesh.014",
"fond.015_fond.mesh.015",
"fond.016_fond.mesh.016",
"fond.017_fond.mesh.017",
"fond.018_fond.mesh.018",
"fond.019_fond.mesh.019",
"fond.020_fond.mesh.020",
"fond.021_fond.mesh.021",
"fond.022_fond.mesh.022",
"fond.023_fond.mesh.023",
"fond.024_fond.mesh.024",
"fond.025_fond.mesh.025",
"fond.026_fond.mesh.026",
"fond.027_fond.mesh.027",
"fond.028_fond.mesh.028",
"fond.029_fond.mesh.029",
"fond.030_fond.mesh.030",
"fond.031_fond.mesh.031",
"fond.032_fond.mesh.032",
"fond.033_fond.mesh.033",
"fond.034_fond.mesh.035",
"fond.035_fond.mesh.036",
"fond.036_fond.mesh.034",
"fond.037_fond.mesh.037",
"fond.042_fond.mesh.042",
"fond.043_fond.mesh.043",
"fond_fond.mesh",
"gearlevel.001_gearlevel.mesh.001",
"gearlevel_gearlevel.mesh",
"gs-scale.001_gs-scale.mesh.001",
"gs-scale_gs-scale.mesh",
"ils-flag.001_ils-flag.mesh.001",
"ils-flag_ils-flag.mesh",
"rect.001_rect.mesh.001",
"rect_rect.mesh",
"ring-slip.001_ring-slip.mesh.001",
"ring-slip_ring-slip.mesh",
"ring.001_ring.mesh.001",
"ring_ring.mesh",
"slip-ball.001_slip-ball.mesh.001",
"slip-ball_slip-ball.mesh",
"scale.001_scale.mesh.001",
"scale_scale.mesh",
"trous_trous.mesh",

"vitre.001_vitre.mesh.001",
"vitre.002_vitre.mesh.002",
"vitre.003_vitre.mesh.003",
"vitre.004_vitre.mesh.004",
"vitre.005_vitre.mesh.005",
"vitre.006_vitre.mesh.006",
"vitre.007_vitre.mesh.007",
"vitre.008_vitre.mesh.008",
"vitre.009_vitre.mesh.009",
"vitre.010_vitre.mesh.010",
"vitre.011_vitre.mesh.011",
"vitre.012_vitre.mesh.012",
"vitre.013_vitre.mesh.013",
"vitre.014_vitre.mesh.014",
"vitre.015_vitre.mesh.015",
"vitre.016_vitre.mesh.016",
"vitre.017_vitre.mesh.017",
"vitre.018_vitre.mesh.018",
"vitre.019_vitre.mesh.019",
"vitre.020_vitre.mesh.020",
"vitre.021_vitre.mesh.021",
"vitre.022_vitre.mesh.022",
"vitre.023_vitre.mesh.023",
"vitre.024_vitre.mesh.024",
"vitre.025_vitre.mesh.025",
"vitre.026_vitre.mesh.026",
"vitre.027_vitre.mesh.027",
"vitre.028_vitre.mesh.028",
"vitre.029_vitre.mesh.029",
"vitre.030_vitre.mesh.030",
"vitre.031_vitre.mesh.031",
"vitre.032_vitre.mesh.032",
"vitre.033_vitre.mesh.033",
"vitre.034_vitre.mesh.034",
"vitre.035_vitre.mesh.035",
"vitre.036_vitre.mesh.036",
"vitre.037_vitre.mesh.037",
"vitre.038_vitre.mesh.038",
"vitre.039_vitre.mesh.039",
"vitre.040_vitre.mesh.040",
"vitre.041_vitre.mesh.041",
"vitre.042_vitre.mesh.042",
"vitre.043_vitre.mesh.043",
"vitre.044_vitre.mesh.044",
"vitre.045_vitre.mesh.045",
"vitre.046_vitre.mesh.046",
"vitre.047_vitre.mesh.047",
"vitre_vitre.mesh",

#endif


};
#endif