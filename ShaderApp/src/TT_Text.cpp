#include <afxdlgs.h>
#include <afxwin.h>
#include <DirectXMath.h>
#include <iostream>
#include <string>
#include <vector>
#include "afxdialogex.h"
#include "Object.h"
#include "pch.h"
#include "ShaderAppDlg.h"
//#include "Render.h"
#include "Utility.h"
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern CShaderAppDlg* parent;

inline float FixedToFloat(const FIXED& f)
{
	return (long(f.value) << 16 | (unsigned short)f.fract) / 65536.0f;
}
TextMesh::TextMesh(string name, vector<string> t, string fontname, uint32_t fontFamily, uint32_t fontsize, string color, string hOrigin, string vOrigin)
	: MeshObject(name, color)
{
	CPaintDC dc(parent);
	CFont font;
	CString fontName = string_to_wstring_mb(fontname).c_str();
	VERIFY(font.CreateFontW(fontsize,				  // nHeight
							0,                        // nWidth
							0,                        // nEscapement
							0,                        // nOrientation
							FW_NORMAL,                // nWeight
							FALSE,					  // bItalic
							FALSE,                    // bUnderline
							0,                        // cStrikeOut
							ANSI_CHARSET,             // nCharSet
							OUT_DEFAULT_PRECIS,       // nOutPrecision
							CLIP_DEFAULT_PRECIS,      // nClipPrecision
							DEFAULT_QUALITY,          // nQuality
							DEFAULT_PITCH | fontFamily, // nPitchAndFamily
							fontName));				  // lpszFacename

	CFont* old_font = dc.SelectObject(&font);
	CreateGlyphOutline(dc, t, font, Position().x, Position().y, color, hOrigin[0], vOrigin[0]);
	dc.SelectObject(old_font);
	UpdateWorld();
}

//typedef struct _GLYPHMETRICS {
//	UINT    gmBlackBoxX;
//	UINT    gmBlackBoxY;
//	POINT   gmptGlyphOrigin;
//	short   gmCellIncX;
//	short   gmCellIncY;
//} GLYPHMETRICS, FAR* LPGLYPHMETRICS;

void TextMesh::CreateGlyphOutline(CDC& dc, vector<string> text, CFont& font, float start_x, float start_y, string color, char hOrigin, char vOrigin)
{
	GLYPHMETRICS gm;
	MAT2 mat2 = { {0,1}, {0,0}, {0,0}, {0,1} }; // Identity matrix
	vector <pair<XMFLOAT4, XMFLOAT4>> pts;
	pair<XMFLOAT4, XMFLOAT4> pt;
	vector<pair<string, pair<vector<VertexType>, vector<XMUINT3>>>> allTriangles;
	float maxX = -99999.0f;
	float maxY = -99999.0f;
	float minX = 99999.0f;
	float minY = 99999.0f;


	DWORD dwSize = dc.GetGlyphOutline(L'M', GGO_NATIVE, &gm, 0, NULL, &mat2);
	float advanceY = (float)(gm.gmBlackBoxY * 3) / 2.0f;
	float current_x = start_x;
	float current_y = start_y;
	vector<vector<VertexType>> allContours;
	for (auto& s : text)
	{
		wstring theText = string_to_wstring_mb(s);
		for (wchar_t uChar : theText)
		{
			if (uChar == L'\r')
			{
				current_x = start_x;
				continue;
			}
			if (uChar == L'\n')
			{
				current_y -= advanceY;
				continue;
			}
			dwSize = dc.GetGlyphOutline(uChar, GGO_NATIVE, &gm, 0, NULL, &mat2);
			DWORD usedBytes = 0;
			if (dwSize != GDI_ERROR)
			{
				uint32_t currentOffset = 0;
				vector<BYTE> buffer(dwSize);
				dc.GetGlyphOutline(uChar, GGO_NATIVE, &gm, dwSize, buffer.data(), &mat2);
				// Process buffer as TTPOLYGONHEADER
				while (usedBytes < dwSize)
				{
					TTPOLYGONHEADER* h = (TTPOLYGONHEADER*)((uint8_t*)buffer.data() + currentOffset);
					currentOffset += sizeof(TTPOLYGONHEADER);
					usedBytes += h->cb;

					pt.first.x = FixedToFloat(h->pfxStart.x); // .value / (float)(1 << h->pfxStart.x.fract)) + 50;
					pt.first.y = FixedToFloat(h->pfxStart.y); // .value / (float)(1 << h->pfxStart.y.fract));
					pt.first.z = 0;
					pt.first.w = 1;
					pt.first.x += current_x;
					pt.first.y += current_y;
					pt.second = FindColor(color);
					pts.push_back(pt);

					maxX = max(maxX, pt.first.x);
					maxY = max(maxY, pt.first.y);
					minX = min(minX, pt.first.x);
					minY = min(minY, pt.first.y);

//					CString st;
//					st.Format(_T("%c  x = %7.2f   y = %7.2f \r\n"), uChar, pt.x, pt.y);
//					OutputDebugStringW(L"-----------------------------------------------\r\n");
//					OutputDebugString(st);
					uint32_t contourBytes = sizeof(TTPOLYGONHEADER);
					while (contourBytes < h->cb)
					{
						TTPOLYCURVE* c = (TTPOLYCURVE*)((uint8_t*)buffer.data() + currentOffset);
						currentOffset += sizeof(TTPOLYCURVE) + (sizeof(POINTFX) * (c->cpfx - 1));
						contourBytes += sizeof(TTPOLYCURVE) + (sizeof(POINTFX) * (c->cpfx - 1));
						for (int i = 0; i < c->cpfx; i++)
						{
							pt.first.x = FixedToFloat(c->apfx[i].x);
							pt.first.y = FixedToFloat(c->apfx[i].y);
							pt.first.z = 0;
							pt.first.w = 1;
							pt.first.x += current_x;
							pt.first.y += current_y;
							pt.second = FindColor(color);
							pts.push_back(pt);
							maxX = max(maxX, pt.first.x);
							maxY = max(maxY, pt.first.y);
							minX = min(minX, pt.first.x);
							minY = min(minY, pt.first.y);
//							st.Format(_T("%c  x = %7.2f   y = %7.2f \r\n"), uChar, pt.x, pt.y);
//							OutputDebugString(st);
						}
					}
					pt = pts[0];
					pts.push_back(pt);
					allContours.push_back(pts);
					pts.erase(pts.begin(), pts.end());
					pts.clear();
				}
//				OutputDebugStringW(L"-----------------------------------------------\r\n");
				char c_array[12];
				wctomb(c_array, uChar);
/*
pair<vector<pair<XMFLOAT4, XMFLOAT3>>, vector<XMUINT3>> EarClipping(vector<vector<pair<XMFLOAT4, XMFLOAT3>>>& polygons)
*/
				pair<vector<VertexType>, vector<XMUINT3>> something = EarClipping(allContours);
				allTriangles.push_back(make_pair(s, something)); // add the Letter to the mass to set as GroupName

				allContours.erase(allContours.begin(), allContours.end());
				allContours.clear();
			}
			current_x += gm.gmCellIncX;
		}
		current_x = start_x;
		current_y -= advanceY;
	}

	// justify all polygons according to origin markers
	float dx = 0;
	if (hOrigin == 'C') // x axis through center of text
	{
		dx = -(minX + maxX) / 2.0f;
	}
	if (hOrigin == 'R')
	{
		dx = -maxX;
	}
	float dy = 0;
	if (hOrigin == 'C') // x axis through center of text
	{
		dy = -(minY + maxY) / 2.0f;

	}
	if (hOrigin == 'B')
	{
		dy = -maxY;
	}
	if ((dx != 0) or (dy != 0)) // skip if origin remains Left Top
	{
		for (auto& triangle : allTriangles)
		{
			for (auto& vertexColor : triangle.second.first)
			{
				vertexColor.first.x += dx;
				vertexColor.first.y += dy;
			}
		}
	}
	// build VertexList and VertexIndex for the text
	int indexOffset = 0;
	groupStartIndex.push_back(0);
	for (auto& triangle : allTriangles)
	{
		if (triangle.second.first.size() == 0) continue;
		if (triangle.second.second.size() == 0) continue;
		groupColor.push_back(triangle.second.first[0].second);
		groupName.push_back(triangle.first);
		for (auto& vertexCoords : triangle.second.first)
		{
//			vertexCoords.first.x *= 1000.0f;
//			vertexCoords.first.y *= 1000.0f;
//			vertexCoords.first.z *= 1000.0f;
			vertexList.push_back(vertexCoords);
		}
		for (auto& index : triangle.second.second)
		{
			vertex_index vi;
			vi.v_index = index;
			vi.t_index = { 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF };
			vi.n_index = { 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF };
			vi.v_index.x += indexOffset;
			vi.v_index.y += indexOffset;
			vi.v_index.z += indexOffset;
			vertexIndex.push_back(vi);
		}
		indexOffset = (int)vertexList.size();
		groupStartIndex.push_back((UINT)vertexIndex.size());
	}
	// destroy allTriangles as they are no longer needed
	for (auto& triangle : allTriangles)
	{
		triangle.first.erase(triangle.first.begin(), triangle.first.end());
		triangle.first.clear();
		triangle.second.first.erase(triangle.second.first.begin(), triangle.second.first.end());
		triangle.second.first.clear();
		triangle.second.second.erase(triangle.second.second.begin(), triangle.second.second.end());
		triangle.second.second.clear();
	}
}

