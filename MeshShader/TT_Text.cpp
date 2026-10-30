#include "stdafx.h"
#include <afx.h>
#include <afxstr.h>
#include <afxwin.h>
#include <afxwin1.inl>
#include <algorithm>
#include <atltypes.h>
#include <cstdint>
#include <cstdlib>
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>
#include "Object.h"
#include "Utility.h"
#include "Win32Application.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

inline float FixedToFloat(const FIXED& f)
{
	return (long(f.value) << 16 | (unsigned short)f.fract) / 65536.0f;
}

TextMesh::TextMesh(string name, vector<string> t, string fontname, uint32_t fontFamily, uint32_t fontsize, string material, string hOrigin, string vOrigin, Justification justify)
	: Object(name)
{
	CPaintDC dc(CWnd::FromHandle(Win32Application::GetHwnd()));
	CFont font;
	CString fontName = fontname.c_str();
	font.CreateFont(fontsize,				  // nHeight
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
							fontName);				  // lpszFacename

	CFont* old_font = dc.SelectObject(&font);
	CreateGlyphOutline(dc, t, font, justify, hOrigin[0], vOrigin[0], material);
	dc.SelectObject(old_font);
}

//typedef struct _GLYPHMETRICS {
//	UINT    gmBlackBoxX;
//	UINT    gmBlackBoxY;
//	POINT   gmptGlyphOrigin;
//	short   gmCellIncX;
//	short   gmCellIncY;
//} GLYPHMETRICS, FAR* LPGLYPHMETRICS;

void TextMesh::CreateGlyphOutline(CDC& dc, vector<string> text, CFont& font, Justification justify, char hOrigin, char vOrigin, string material)
{
	GLYPHMETRICS gm;
	MAT2 mat2 = { {0,1}, {0,0}, {0,0}, {0,1} }; // Identity matrix
	vector <XMFLOAT3> pts;
	XMFLOAT3 pt;
//	vector<pair<string, pair<vector<XMFLOAT3>, vector<XMUINT3>>>> allTriangles;
	vector<vector<XMFLOAT3>> allContours;
	float maxX = -99999.0f;
	float maxY = -99999.0f;
	float minX = 99999.0f;
	float minY = 99999.0f;
	float start_x = 0;
	float start_y = 0;

	DWORD dwSize = dc.GetGlyphOutline(L'M', GGO_NATIVE, &gm, 0, NULL, &mat2);
	float advanceY = (float)(gm.gmBlackBoxY * 3) / 2.0f;
	float current_x = start_x;
	float current_y = start_y;
	float justified_start_x = start_x;
	for (auto& s : text)
	{
		CString cs(s.c_str());
		CSize sz = dc.GetTextExtent(cs);
		switch (justify)
		{
			case Justification::Left_Justified:
				justified_start_x = start_x;
				break;
			case Justification::Center_Justified:
				justified_start_x = start_x - (float)sz.cx / 2.0f;
				break;
			case Justification::Right_Justified:
				justified_start_x = start_x - (float)sz.cx;
				break;
		}
		current_x = justified_start_x;
		wstring theText = string_to_wstring_mb(s);


		for (wchar_t uChar : theText)
		{
			if (uChar == L'\r')
			{
				current_x = justified_start_x;
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

					pt.x = FixedToFloat(h->pfxStart.x); // .value / (float)(1 << h->pfxStart.x.fract)) + 50;
					pt.y = FixedToFloat(h->pfxStart.y); // .value / (float)(1 << h->pfxStart.y.fract));
					pt.z = 0;
					pt.x += current_x;
					pt.y += current_y;
					pts.push_back(pt);

					maxX = std::max(maxX, pt.x);
					maxY = max(maxY, pt.y);
					minX = min(minX, pt.x);
					minY = min(minY, pt.y);

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
							pt.x = FixedToFloat(c->apfx[i].x);
							pt.y = FixedToFloat(c->apfx[i].y);
							pt.z = 0;
							pt.x += current_x;
							pt.y += current_y;
							pts.push_back(pt);
							maxX = max(maxX, pt.x);
							maxY = max(maxY, pt.y);
							minX = min(minX, pt.x);
							minY = min(minY, pt.y);
						}
					}
					pt = pts[0];
					pts.push_back(pt);
					allContours.push_back(pts);
					pts.erase(pts.begin(), pts.end());
					pts.clear();
				}
//				OutputDebugStringW(L"-----------------------------------------------\r\n");
				pair<vector<XMFLOAT3>, vector<vector<XMUINT3>>> something = EarClippingLevel1(allContours);

				char c_array[16];
				wctomb(c_array, uChar);
				string name = string(c_array);
				shared_ptr<MeshObject> component = make_shared<MeshObject>(name, material);
				component->Vertices() = something.first;
	//			component->VertexIndex() = something.second;
				AddComponent(name, component);
//				allTriangles.push_back(make_pair(s, something)); // add the Letter to the mass to set as GroupName

				allContours.erase(allContours.begin(), allContours.end());
				allContours.clear();
			}
			current_x += gm.gmCellIncX;
		}
		current_x = justified_start_x;
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
		for (auto& [name, mesh_object] : MeshObjectList)
		{
			for (auto& v : mesh_object->Vertices())
			{
				v.x += dx;
				v.y += dy;
			}
		}
	}
}

