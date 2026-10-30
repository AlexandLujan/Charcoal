#include "stdafx.h"
#include <afx.h>
#include <afxstr.h>
#include <afxwin.h>
#include <cstdint>
#include <cstdlib>
#include <DirectXMath.h>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>
#include "Object.h"
#include "Utility.h"

#include <cmath>
#include <cmath>
#include <cstdint>
#include <cstdint>
#include <cstdlib>
#include <cstdlib>
#include <DirectXMath.h>
#include <DirectXMath.h>
#include <iterator>
#include <memory>
#include <string>
#include <string>
#include <unordered_map>
#include <utility>
#include <utility>
#include <vector>
#include <vector>
#include <windows.h>
#include "ColorControl.h"
#include "Material.h"
#include "Object.h"
#include "Object.h"
#include "Utility.h"

using namespace std;
using namespace DirectX;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern unordered_map<std::string, std::shared_ptr<Material>> MaterialsMap;

StarShip::StarShip(std::string ship_name, std::string material, float cube_size, float height, UINT pin_count) :
	Object(ship_name), base_cube(cube_size), max_height(height), pin_count(pin_count)
{
	vector<XMFLOAT3> spirePoints = {};

	// construct central cube
	shared_ptr<MeshObject> box = make_shared<MeshObject>(ship_name + "_box", Material::RandomMaterial());
	spirePoints = CreateCube(base_cube);
	AssignIndicesToSpireVertices(spirePoints);
	box->groupStartIndex.push_back((uint32_t)v_index.size());
	box->Vertices() = vertices;
//	box->VertexIndex() = v_index;
	AddComponent(box->Name(), box);
	spirePoints.clear();
	vertices.clear();
	v_index.clear();

	float pin_width = floor(base_cube / (float)pin_count);
	int ring_count = (pin_count + 1) / 2;
	int edge_ring = pin_count / 2;
	XMFLOAT3 center = XMFLOAT3(0, 0, 0);
	float spire_height = max_height;
	float half_cube = base_cube / 2.0f;
	int spire_layer = 0;
	for (int row = -edge_ring; row <= edge_ring; row++)
	{
		for (int col = -edge_ring; col <= edge_ring; col++)
		{
			center = XMFLOAT3((float)row * pin_width, (float)col * pin_width, 0);
			if ((labs(row) == 0) && (labs(col) == 0))
			{
				spire_height = max_height;
				spire_layer = 0;
			}
			else if ((labs(row) == 1) || (labs(col) == 1))
			{
				spire_height = max_height * 0.8f;
				spire_layer = 1;
			}
			else if ((labs(row) == 2) || (labs(col) == 2))
			{
				spire_height = max_height * 0.6f;
				spire_layer = 2;
			}
			else if ((labs(row) == 3) || (labs(col) == 3))
			{
				spire_height = max_height * 0.4f;
				spire_layer = 3;
			}
			else if ((labs(row) == 4) || (labs(col) == 4))
			{
				spire_height = max_height * 0.3f;
				spire_layer = 4;
			}
			else
			{
				spire_height = max_height * 0.2f;
				spire_layer = 5;
			}

			// construct top spires
			shared_ptr<MeshObject> box_top = make_shared<MeshObject>(ship_name + "_spire_top" + std::to_string(row) + "_" + std::to_string(col), Material::RandomMaterial());
			spirePoints = BuildSpire(0, { center.x, half_cube, center.y }, pin_width, spire_height, spire_layer);
			AssignIndicesToSpireVertices(spirePoints);
			box_top->groupStartIndex.push_back((uint32_t)v_index.size());
			box_top->Vertices() = vertices;
	//		box_top->VertexIndex() = v_index;
			AddComponent(box_top->Name(), box_top);
			vertices.clear();
			v_index.clear();
			spirePoints.clear();

			// construct bottom spires
			shared_ptr<MeshObject> box_bottom = make_shared<MeshObject>(ship_name + "_spire_bottom" + std::to_string(row) + "_" + std::to_string(col), Material::RandomMaterial());
			spirePoints = BuildSpire(1, { center.x,-half_cube, center.y }, pin_width, spire_height, spire_layer);
			AssignIndicesToSpireVertices(spirePoints);
			box_bottom->groupStartIndex.push_back((uint32_t)v_index.size());
			box_bottom->Vertices() = vertices;
	//		box_bottom->VertexIndex() = v_index;
			AddComponent(box_bottom->Name(), box_bottom);
			vertices.clear();
			v_index.clear();
			spirePoints.clear();

			// construct left spires
			shared_ptr<MeshObject> box_left = make_shared<MeshObject>(ship_name + "_spire_left" + std::to_string(row) + "_" + std::to_string(col), Material::RandomMaterial());
			spirePoints = BuildSpire(2, { -half_cube,center.x,center.y }, pin_width, spire_height, spire_layer);
			AssignIndicesToSpireVertices(spirePoints);
			box_left->groupStartIndex.push_back((uint32_t)v_index.size());
			box_left->Vertices() = vertices;
	//		box_left->VertexIndex() = v_index;
			AddComponent(box_left->Name(), box_left);
			vertices.clear();
			v_index.clear();
			spirePoints.clear();

			// construct right spires
			shared_ptr<MeshObject> box_right = make_shared<MeshObject>(ship_name + "_spire_right" + std::to_string(row) + "_" + std::to_string(col), Material::RandomMaterial());
			spirePoints = BuildSpire(3, { half_cube,center.x,center.y }, pin_width, spire_height, spire_layer);
			AssignIndicesToSpireVertices(spirePoints);
			box_right->groupStartIndex.push_back((uint32_t)v_index.size());
			box_right->Vertices() = vertices;
	//		box_right->VertexIndex() = v_index;
			AddComponent(box_right->Name(), box_right);
			vertices.clear();
			v_index.clear();
			spirePoints.clear();

			// construct front spires
			shared_ptr<MeshObject> box_front = make_shared<MeshObject>(ship_name + "_spire_front" + std::to_string(row) + "_" + std::to_string(col), Material::RandomMaterial());
			spirePoints = BuildSpire(4, { center.x,center.y,-half_cube }, pin_width, spire_height, spire_layer);
			AssignIndicesToSpireVertices(spirePoints);
			box_front->groupStartIndex.push_back((uint32_t)v_index.size());
			box_front->Vertices() = vertices;
	//		box_front->VertexIndex() = v_index;
			AddComponent(box_front->Name(), box_front);
			vertices.clear();
			v_index.clear();
			spirePoints.clear();

			// construct back spires
			shared_ptr<MeshObject> box_back = make_shared<MeshObject>(ship_name + "_spire_back" + std::to_string(row) + "_" + std::to_string(col), Material::RandomMaterial());
			spirePoints = BuildSpire(5, { center.x,center.y,half_cube }, pin_width, spire_height, spire_layer);
			AssignIndicesToSpireVertices(spirePoints);
			box_back->groupStartIndex.push_back((uint32_t)v_index.size());
			box_back->Vertices() = vertices;
	//		box_back->VertexIndex() = v_index;
			AddComponent(box_back->Name(), box_back);
			vertices.clear();
			v_index.clear();
			spirePoints.clear();
		}
	}
}

vector<XMFLOAT3> StarShip::CreateCube(float side)
{
	float dSide = side / 2.0f;
	vector<XMFLOAT3> box;
	box.push_back({ dSide, dSide, dSide });
	box.push_back({ dSide, -dSide, dSide });
	box.push_back({ -dSide, dSide, dSide });
	box.push_back({ -dSide, -dSide,dSide });

	box.push_back({ dSide, dSide, -dSide });
	box.push_back({ dSide, -dSide, -dSide });
	box.push_back({ -dSide, dSide, -dSide });
	box.push_back({ -dSide, -dSide, -dSide });
	return box;
}
float random(float min, float max)
{
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}
vector<XMFLOAT3> StarShip::BuildSpire(int orientation, XMFLOAT3 base, float pin_width, float spire_height, int spire_layer)
{
	float Width = pin_width / 2.5f;
	XMFLOAT3 b0 = { 0,0,0 };
	XMFLOAT3 b1 = { 0,0,0 };
	XMFLOAT3 b2 = { 0,0,0 };
	XMFLOAT3 b3 = { 0,0,0 };
	XMFLOAT3 s0 = { 0,0,0 };
	XMFLOAT3 s1 = { 0,0,0 };
	XMFLOAT3 s2 = { 0,0,0 };
	XMFLOAT3 s3 = { 0,0,0 };
	vector<XMFLOAT3> points;
	spire_height = random(spire_height * 0.8f, spire_height * 1.2f);
	XMFLOAT3 d = { 0,0,0 };
	float dX = base.x;
	float dY = base.y;
	float dZ = base.z;
	switch (orientation)
	{
		case 0: // top x and z are the same, yis different
			d = VECTOR({ 0,0,0 }, { base.x,0,base.z });
			b0 = XMFLOAT3(base.x + Width, base.y, base.z + Width);
			b1 = XMFLOAT3(base.x + Width, base.y, base.z - Width);
			b2 = XMFLOAT3(base.x - Width, base.y, base.z + Width);
			b3 = XMFLOAT3(base.x - Width, base.y, base.z - Width);
			s0 = XMFLOAT3(base.x + Width + dX, base.y + spire_height, base.z + Width + dZ);
			s1 = XMFLOAT3(base.x + Width + dX, base.y + spire_height, base.z - Width + dZ);
			s2 = XMFLOAT3(base.x - Width + dX, base.y + spire_height, base.z + Width + dZ);
			s3 = XMFLOAT3(base.x - Width + dX, base.y + spire_height, base.z - Width + dZ);
			break;
		case 1: // bottom
			b0 = XMFLOAT3(base.x + Width, base.y, base.z + Width);
			b1 = XMFLOAT3(base.x + Width, base.y, base.z - Width);
			b2 = XMFLOAT3(base.x - Width, base.y, base.z + Width);
			b3 = XMFLOAT3(base.x - Width, base.y, base.z - Width);
			s0 = XMFLOAT3(base.x + Width, base.y - spire_height, base.z + Width);
			s1 = XMFLOAT3(base.x + Width, base.y - spire_height, base.z - Width);
			s2 = XMFLOAT3(base.x - Width, base.y - spire_height, base.z + Width);
			s3 = XMFLOAT3(base.x - Width, base.y - spire_height, base.z - Width);
			break;
		case 2: // left 
			b0 = XMFLOAT3(base.x, base.y + Width, base.z + Width);
			b1 = XMFLOAT3(base.x, base.y + Width, base.z - Width);
			b2 = XMFLOAT3(base.x, base.y - Width, base.z + Width);
			b3 = XMFLOAT3(base.x, base.y - Width, base.z - Width);
			s0 = XMFLOAT3(base.x - spire_height, base.y + Width, base.z + Width);
			s1 = XMFLOAT3(base.x - spire_height, base.y + Width, base.z - Width);
			s2 = XMFLOAT3(base.x - spire_height, base.y - Width, base.z + Width);
			s3 = XMFLOAT3(base.x - spire_height, base.y - Width, base.z - Width);
			break;
		case 3: // right
			b0 = XMFLOAT3(base.x, base.y + Width, base.z + Width);
			b1 = XMFLOAT3(base.x, base.y + Width, base.z - Width);
			b2 = XMFLOAT3(base.x, base.y - Width, base.z + Width);
			b3 = XMFLOAT3(base.x, base.y - Width, base.z - Width);
			s0 = XMFLOAT3(base.x + spire_height, base.y + Width, base.z + Width);
			s1 = XMFLOAT3(base.x + spire_height, base.y + Width, base.z - Width);
			s2 = XMFLOAT3(base.x + spire_height, base.y - Width, base.z + Width);
			s3 = XMFLOAT3(base.x + spire_height, base.y - Width, base.z - Width);
			break;
		case 4: // front
			b0 = XMFLOAT3(base.x + Width, base.y + Width, base.z);
			b1 = XMFLOAT3(base.x + Width, base.y - Width, base.z);
			b2 = XMFLOAT3(base.x - Width, base.y + Width, base.z);
			b3 = XMFLOAT3(base.x - Width, base.y - Width, base.z);
			s0 = XMFLOAT3(base.x + Width, base.y + Width, base.z - spire_height);
			s1 = XMFLOAT3(base.x + Width, base.y - Width, base.z - spire_height);
			s2 = XMFLOAT3(base.x - Width, base.y + Width, base.z - spire_height);
			s3 = XMFLOAT3(base.x - Width, base.y - Width, base.z - spire_height);
			break;
		case 5: // back
			b0 = XMFLOAT3(base.x + Width, base.y + Width, base.z);
			b1 = XMFLOAT3(base.x + Width, base.y - Width, base.z);
			b2 = XMFLOAT3(base.x - Width, base.y + Width, base.z);
			b3 = XMFLOAT3(base.x - Width, base.y - Width, base.z);
			s0 = XMFLOAT3(base.x + Width, base.y + Width, base.z + spire_height);
			s1 = XMFLOAT3(base.x + Width, base.y - Width, base.z + spire_height);
			s2 = XMFLOAT3(base.x - Width, base.y + Width, base.z + spire_height);
			s3 = XMFLOAT3(base.x - Width, base.y - Width, base.z + spire_height);
			break;
	}
	points.push_back(b0);
	points.push_back(b1);
	points.push_back(b2);
	points.push_back(b3);
	points.push_back(s0);
	points.push_back(s1);
	points.push_back(s2);
	points.push_back(s3);
	return points;
}

uint32_t StarShip::findVertex(XMFLOAT3 position)
{
	for (int i = 0; i < (int)vertices.size(); i++)
	{
		if ((vertices[i].x == position.x) and
			(vertices[i].y == position.y) and
			(vertices[i].z == position.z))
		{
			return i;
		}
	}
	uint32_t index = (uint32_t)vertices.size();
	XMFLOAT3 v = position;
	vertices.push_back(v);
	return index;
};

void StarShip::AssignIndicesToSpireVertices(vector<XMFLOAT3> spirePoints)
{
	vector<uint32_t> indexList;
	for (auto& point : spirePoints)
	{
		indexList.push_back(findVertex(point));
	}
	// use indexList to create triangles for the spire
	//v_index.push_back({ { indexList[0],indexList[1],indexList[3] },No_Index_XMUINT4, No_Index_XMUINT4 });
	//v_index.push_back({ { indexList[0],indexList[3],indexList[2] },No_Index_XMUINT4, No_Index_XMUINT4 });
	//v_index.push_back({ { indexList[4],indexList[7],indexList[5] },No_Index_XMUINT4, No_Index_XMUINT4 });
	//v_index.push_back({ { indexList[4],indexList[6],indexList[7] },No_Index_XMUINT4, No_Index_XMUINT4 });

	//v_index.push_back({ { indexList[1],indexList[4],indexList[0] },No_Index_XMUINT4, No_Index_XMUINT4 });
	//v_index.push_back({ { indexList[1],indexList[5],indexList[4] },No_Index_XMUINT4, No_Index_XMUINT4 });

	//v_index.push_back({ { indexList[6],indexList[0],indexList[4] },No_Index_XMUINT4, No_Index_XMUINT4 });
	//v_index.push_back({ { indexList[6],indexList[2],indexList[0] },No_Index_XMUINT4, No_Index_XMUINT4 });

	//v_index.push_back({ { indexList[5],indexList[1],indexList[3] },No_Index_XMUINT4, No_Index_XMUINT4 });
	//v_index.push_back({ { indexList[5],indexList[3],indexList[7] },No_Index_XMUINT4, No_Index_XMUINT4 });

	//v_index.push_back({ { indexList[3],indexList[6],indexList[2] },No_Index_XMUINT4, No_Index_XMUINT4 });
	//v_index.push_back({ { indexList[3],indexList[7],indexList[6] },No_Index_XMUINT4, No_Index_XMUINT4 });

}
