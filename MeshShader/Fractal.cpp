#include "stdafx.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <DirectXMath.h>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>
#include "Object.h"
#include "Utility.h"

using namespace std;
using namespace DirectX;

vector<string> altitudeColors = { "Blue","Green","Bronze","Chrome","White" };

int ColorByAltitude(float altitude)
{
	if (altitude < 0) return 0; // blue for water
	else if (altitude < 0.3f) return 1; // green for low land
	else if (altitude < 0.45f)return 2; // yellow for mid land
	else if (altitude < 0.66f)return 3; // brown for high land
	return 4; // white for snow
}

XMFLOAT3 MidPoint(XMFLOAT3 p0, XMFLOAT3 p1)
{
	// from a number 0-1.0, make it 0-0.4. make -0.2 - 0.2 = distance from center
	float dc = ((float)rand() / (float)RAND_MAX);
	dc /= 2.5f;
	dc -= 0.2f;
	float mx = p0.x + (p1.x - p0.x) * (dc + 0.5f);
	float my = p0.y + (p1.y - p0.y) * (dc + 0.5f);
	float mz = p0.z + (p1.z - p0.z) * (dc + 0.5f);

	return { mx,my,mz };
}

void Fractal::MountainOutOfNothing(vector<XMFLOAT3> baseCoords, vector<XMUINT3> baseIndex, int iterations)
{
	vector<uint32_t>						groupStartIndex = {};
	vector<XMFLOAT3>						vertexList = baseCoords;
	vector<XMUINT3>							vertexIndex = baseIndex;
	vector<XMUINT3>							newIndices = {};
	map<pair<uint32_t, uint32_t>, uint32_t> midPointCache = {}; // cache to store the midpoint index for each edge
	for (int i = 0; i < iterations; i++)
	{
		groupStartIndex.push_back(0);

		for (int j = 0; j < (int)vertexIndex.size(); j++)
		{

			XMUINT3 corner = vertexIndex[j];
			XMFLOAT3 c0 = vertexList[corner.x];
			XMFLOAT3 c1 = vertexList[corner.y];
			XMFLOAT3 c2 = vertexList[corner.z];

			auto xy = make_pair(corner.x, corner.y);
			auto yx = make_pair(corner.y, corner.x);
			XMFLOAT3 mid01 = { 0,0,0 };
			uint32_t index01 = 0;
			if (midPointCache.count(xy) > 0)
			{
				index01 = midPointCache[xy];
			}
			else if (midPointCache.count(yx) > 0)
			{
				index01 = midPointCache[yx];
			}
			else
			{
				mid01 = MidPoint(c0, c1);
				mid01.y += ((float)rand() / (float)RAND_MAX - 0.25f) * powf(2.0f, (float)-i);
				index01 = (uint32_t)vertexList.size();
				midPointCache[xy] = index01;
				vertexList.push_back(mid01);	// index01
			}

			auto yz = make_pair(corner.y, corner.z);
			auto zy = make_pair(corner.z, corner.y);
			XMFLOAT3 mid12 = { 0,0,0 };
			uint32_t index12 = 0;
			if (midPointCache.count(yz) > 0)
			{
				index12 = midPointCache[yz];
			}
			else if (midPointCache.count(zy) > 0)
			{
				index12 = midPointCache[zy];
			}
			else
			{
				mid12 = MidPoint(c1, c2);
				mid12.y += ((float)rand() / (float)RAND_MAX - 0.25f) * powf(2.0f, (float)-i);
				index12 = (uint32_t)vertexList.size();
				midPointCache[yz] = index12;
				vertexList.push_back(mid12);	// index01
			}

			auto xz = make_pair(corner.x, corner.z);
			auto zx = make_pair(corner.z, corner.x);

			XMFLOAT3 mid20 = { 0,0,0 };
			uint32_t index20 = 0;
			if (midPointCache.count(xz) > 0)
			{
				index20 = midPointCache[xz];
			}
			else if (midPointCache.count(zx) > 0)
			{
				index20 = midPointCache[zx];
			}
			else
			{
				mid20 = MidPoint(c2, c0);
				mid20.y += ((float)rand() / (float)RAND_MAX - 0.25f) * powf(2.0f, (float)-i);
				index20 = (uint32_t)vertexList.size();
				midPointCache[xz] = index20;
				vertexList.push_back(mid20);	// index01
			}
			XMUINT3 tri0 = { corner.x,  index01, index20 };
			XMUINT3 tri1 = { corner.y,  index12, index01 };
			XMUINT3 tri2 = { corner.z,  index20, index12 };
			XMUINT3 tri3 = { index01,   index12, index20 };
			newIndices.push_back(tri0);
			newIndices.push_back(tri1);
			newIndices.push_back(tri2);
			newIndices.push_back(tri3);
		}
		groupStartIndex.push_back((uint32_t)vertexIndex.size());
		vertexIndex = newIndices;
		newIndices.erase(newIndices.begin(), newIndices.end());
		newIndices.clear();
		newIndices = {};
	}

	midPointCache.clear();

	shared_ptr<MeshObject> blue = make_shared<MeshObject>("Blue", "Blue");
	shared_ptr<MeshObject> green = make_shared<MeshObject>("Green", "Green");
	shared_ptr<MeshObject> bronze = make_shared<MeshObject>("Bronze", "Bronze");
	shared_ptr<MeshObject> chrome = make_shared<MeshObject>("Chrome", "Chrome");
	shared_ptr<MeshObject> white = make_shared<MeshObject>("White", "White");

	for (auto& idx : vertexIndex)
	{
		XMFLOAT3 v0 = vertexList[idx.x];
		XMFLOAT3 v1 = vertexList[idx.y];
		XMFLOAT3 v2 = vertexList[idx.z];
		UINT i0 = 0;
		UINT i1 = 0;
		UINT i2 = 0;
		FLOAT alt = (v0.y + v1.y + v2.y) / 3.0f;
		int group = ColorByAltitude(alt);
		switch (group)
		{
			case 0:
				i0 = blue->findVertex(v0);
				i1 = blue->findVertex(v1);
				i2 = blue->findVertex(v2);
//				blue->VertexIndex().push_back({ {i0,i1,i2},No_Index_XMUINT4,No_Index_XMUINT4 });
				break;
			case 1:
				i0 = green->findVertex(v0);
				i1 = green->findVertex(v1);
				i2 = green->findVertex(v2);
//				green->VertexIndex().push_back({ {i0,i1,i2},No_Index_XMUINT4,No_Index_XMUINT4 });
				break;
			case 2:
				i0 = bronze->findVertex(v0);
				i1 = bronze->findVertex(v1);
				i2 = bronze->findVertex(v2);
//				bronze->VertexIndex().push_back({ {i0,i1,i2},No_Index_XMUINT4,No_Index_XMUINT4 });
				break;
			case 3:
				i0 = chrome->findVertex(v0);
				i1 = chrome->findVertex(v1);
				i2 = chrome->findVertex(v2);
//				chrome->VertexIndex().push_back({ {i0,i1,i2},No_Index_XMUINT4,No_Index_XMUINT4 });
				break;
			case 4:
				i0 = white->findVertex(v0);
				i1 = white->findVertex(v1);
				i2 = white->findVertex(v2);
//				white->VertexIndex().push_back({ {i0,i1,i2},No_Index_XMUINT4,No_Index_XMUINT4 });
				break;
		}
	}
	vertexIndex.clear();
}
