#include "pch.h"
#include "Object.h"
//#include "Render.h"
#include "ShaderAppDlg.h"

// https://www.castledragmire.com/Projects/Fractal_Landscape#SourceCodeDefinitions
// https://www.castledragmire.com/Projects/Fractal_Landscape
// https://vickymmcd.github.io/3DFractalMountains/#:~:text=The%20goal%20of%20this%20project,run%20to%20generate%20the%20terrain.

using namespace std;
using namespace DirectX;
extern CShaderAppDlg* parent;
vector<string> altitudeColors = { "Blue","Green","Bronze","Chrome","White" };
int ColorByAltitude(float altitude)
{
	if (altitude < 0) return 0; // blue for water
	else if (altitude < 0.3f) return 1; // green for low land
	else if (altitude < 0.45f)return 2; // yellow for mid land
	else if (altitude < 0.66f)return 3; // brown for high land
	return 4; // white for snow
}

XMFLOAT4 MidPoint(XMFLOAT4 p0, XMFLOAT4 p1)
{
	// from a number 0-1.0, make it 0-0.4. make -0.2 - 0.2 = distance from center
	float dc = ((float)rand() / (float)RAND_MAX);
	dc /= 2.5f;
	dc -= 0.2f;
	float mx = p0.x + (p1.x - p0.x) * (dc + 0.5f);
	float my = p0.y + (p1.y - p0.y) * (dc + 0.5f);
	float mz = p0.z + (p1.z - p0.z) * (dc + 0.5f);

	return { mx,my,mz,1 };
}

void Fractal::MountainOutOfNothing(vector<VertexType> baseCoords, vector<vertex_index> baseIndex)
{
	float delta = 0.2f * ((float)rand() / (float)RAND_MAX);
	vertexList = baseCoords;
	vertexIndex = baseIndex;
	map<pair<uint32_t, uint32_t>, uint32_t> midPointCache = {}; // cache to store the midpoint index for each edge
	vector<vertex_index> newIndices = {};
	int altitude;
	for (int i = 0; i < iterations; i++)
	{
		for (int j = 0; j < (int)vertexIndex.size(); j++)
		{
			XMUINT3 corner = vertexIndex[j].v_index;
			XMFLOAT4 c0 = vertexList[corner.x].first;
			XMFLOAT4 c1 = vertexList[corner.y].first;
			XMFLOAT4 c2 = vertexList[corner.z].first;

			auto xy = make_pair(corner.x, corner.y);
			auto yx = make_pair(corner.y, corner.x);
			XMFLOAT4 mid01 = { 0,0,0,0 };
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
				altitude = ColorByAltitude(mid01.y);
				vertexList.push_back(make_pair(mid01, FindColor(altitudeColors[altitude])));	// index01
			}

			auto yz = make_pair(corner.y, corner.z);
			auto zy = make_pair(corner.z, corner.y);
			XMFLOAT4 mid12 = { 0,0,0,0 };
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
				altitude = ColorByAltitude(mid12.y);
				vertexList.push_back(make_pair(mid12, FindColor(altitudeColors[altitude])));	// index01
			}

			auto xz = make_pair(corner.x, corner.z);
			auto zx = make_pair(corner.z, corner.x);

			XMFLOAT4 mid20 = { 0,0,0,0 };
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
				mid20 = MidPoint(c0, c2);
				mid20.y += ((float)rand() / (float)RAND_MAX - 0.25f) * powf(2.0f, (float)-i);
				index20 = (uint32_t)vertexList.size();
				midPointCache[xz] = index20;
				altitude = ColorByAltitude(mid20.y);
				vertexList.push_back(make_pair(mid20, FindColor(altitudeColors[altitude])));
			}
			XMUINT3 tri0 = { corner.x,  index20, index01 };
			XMUINT3 tri1 = { corner.y,  index01, index12 };
			XMUINT3 tri2 = { corner.z,  index12, index20 };
			XMUINT3 tri3 = { index20,  index12, index01 };
			newIndices.push_back({ tri0,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF} });
			newIndices.push_back({ tri1,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF} });
			newIndices.push_back({ tri2,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF} });
			newIndices.push_back({ tri3,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF} });
		}
		vertexIndex = newIndices;
		newIndices.erase(newIndices.begin(), newIndices.end());
		newIndices.clear();
		newIndices = {};
	}
	midPointCache.clear();
	vector<vertex_index> Blue = {};
	vector<vertex_index> Green = {};
	vector<vertex_index> Bronze = {};
	vector<vertex_index> Chrome = {};
	vector<vertex_index> White = {};
	for (auto& idx : vertexIndex)
	{
		VertexType v0 = vertexList[idx.v_index.x];
		VertexType v1 = vertexList[idx.v_index.y];
		VertexType v2 = vertexList[idx.v_index.z];
		FLOAT alt = (v0.first.y + v1.first.y + v2.first.y) / 3.0f;
		int group = ColorByAltitude(alt);
		switch (group)
		{
			case 0:
				Blue.push_back(idx);
				break;
			case 1:
				Green.push_back(idx);
				break;
			case 2:
				Bronze.push_back(idx);
				break;
			case 3:
				Chrome.push_back(idx);
				break;
			case 4:
				White.push_back(idx);
				break;
		}
	}
	vertexIndex.clear();
	groupStartIndex.push_back(0);
	groupName.push_back("Blue");
	groupColor.push_back(FindColor("Blue"));
	vertexIndex.insert(vertexIndex.end(), Blue.begin(), Blue.end());
	groupStartIndex.push_back((uint32_t)vertexIndex.size());
	groupName.push_back("Green");
	groupColor.push_back(FindColor("Green"));
	vertexIndex.insert(vertexIndex.end(), Green.begin(), Green.end());
	groupStartIndex.push_back((uint32_t)vertexIndex.size());
	groupName.push_back("Bronze");
	groupColor.push_back(FindColor("Bronze"));
	vertexIndex.insert(vertexIndex.end(), Bronze.begin(), Bronze.end());
	groupStartIndex.push_back((uint32_t)vertexIndex.size());
	groupName.push_back("Chrome");
	groupColor.push_back(FindColor("Chrome"));
	vertexIndex.insert(vertexIndex.end(), Chrome.begin(), Chrome.end());
	groupStartIndex.push_back((uint32_t)vertexIndex.size());
	groupName.push_back("White");
	groupColor.push_back(FindColor("White"));
	vertexIndex.insert(vertexIndex.end(), White.begin(), White.end());
	groupStartIndex.push_back((uint32_t)vertexIndex.size());
	Blue.clear();
	Green.clear();
	Bronze.clear();
	Chrome.clear();
	White.clear();

}
