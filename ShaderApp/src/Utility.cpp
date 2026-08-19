#include "pch.h"
#include <iostream>
#include "Const.h"
#include "Utility.h"

//#include "Texture.h"
//#include <filesystem> // Required for std::filesystem

using namespace std;

uint32_t backgroundColor = 0x00000000;

uint32_t picture[PICTURE_HEIGHT][PICTURE_WIDTH];

void ClearCanvas()
{
	for (int i = 0; i < PICTURE_HEIGHT; i++)
	{
		for (int j = 0; j < PICTURE_WIDTH; j++)
		{
			picture[i][j] = backgroundColor;
			//			zDepth[i][j] = -99999;
		}
	}
}

uint32_t colorswap(uint32_t n)
{
	uint32_t i = ((n >> 16) & 0x000000FF) | (n & 0x0000FF00) | ((n << 16) & 0x00FF0000); // | */ ((n << 16) & 0xFF000000);
	return i;
}

COLORREF windowsColor(vector<float> intensity)
{
	uint32_t red = (uint32_t)(intensity[RED] * 255.0f) & 0x00FF;
	uint32_t green = (uint32_t)(intensity[GREEN] * 255.0f) & 0x00FF;
	uint32_t blue = (uint32_t)(intensity[BLUE] * 255.0f) & 0x00FF;
	return (red & 0x00FF) + ((green << 8) & 0x00FF00) + ((blue << 16) & 0x00FF0000);
}
COLORREF RGB_Color(vector<float> intensity)
{
	uint32_t red = (uint32_t)(intensity[RED] * 255.0f) & 0x00FF;
	uint32_t green = (uint32_t)(intensity[GREEN] * 255.0f) & 0x00FF;
	uint32_t blue = (uint32_t)(intensity[BLUE] * 255.0f) & 0x00FF;
	return (blue & 0x00FF) + ((green << 8) & 0x00FF00) + ((red << 16) & 0x00FF0000);
}
COLORREF windowsColor(uint32_t red, uint32_t green, uint32_t blue)
{
	return (red & 0x00FF) + ((green << 8) & 0x00FF00) + ((blue << 16) & 0x00FF0000);
}

string formatFloat(float n)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << n;
	std::string s = stream.str();
	return s;
}
const float M_PI = 3.14159265358979323846f;
void SetCameraRotation(shared_ptr<Camera> c, int32_t _pitch, int32_t _yaw, int32_t _roll)
{
	const float toRadians = (float)(M_PI / 180.0);
	float pitch = (float)_pitch * toRadians; // elevation* toRadians;
	float yaw = (float)_yaw * toRadians;
	float roll = (float)_roll * toRadians;
}

float DOT(XMFLOAT3 a, XMFLOAT3 b) { return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z)); }

XMFLOAT3 CROSS(XMFLOAT3 a, XMFLOAT3 b)
{
	float x = (a.y * b.z) - (a.z * b.y);
	float y = (a.z * b.x) - (a.x * b.z);
	float z = (a.x * b.y) - (a.y * b.x);
	return { x,y,z }; // Point3D(x, y, z);
}

void Normalize(XMFLOAT3& c)
{
	float n = sqrtf((c.x * c.x) + (c.y * c.y) + (c.z * c.z));
	if (n != 0)
	{
		c.x = c.x / n;
		c.y = c.y / n;
		c.z = c.z / n;
	}
}

XMFLOAT3 Normal(XMFLOAT3 P1, XMFLOAT3 P2, XMFLOAT3 P3) // return the normal
{
	XMFLOAT3 a(P2.x - P1.x, P2.y - P1.y, P2.z - P1.z);
	XMFLOAT3 b(P3.x - P1.x, P3.y - P1.y, P3.z - P1.z);
	XMFLOAT3 result = CROSS(a, b);
	Normalize(result);
	return { result.x, result.y, result.z };
}
XMFLOAT3 Normal(XMFLOAT4 P1, XMFLOAT4 P2, XMFLOAT4 P3) // return the normal
{
	XMFLOAT3 a(P2.x - P1.x, P2.y - P1.y, P2.z - P1.z);
	XMFLOAT3 b(P3.x - P1.x, P3.y - P1.y, P3.z - P1.z);
	XMFLOAT3 result = CROSS(a, b);
	Normalize(result);
	return { result.x, result.y, result.z };
}
XMFLOAT3 Tangent(XMFLOAT3 P1, XMFLOAT3 P2)
{
	XMFLOAT3 result = { P2.x - P1.x, P2.y - P1.y, P2.z - P1.z };
	Normalize(result);
	return result;
}
XMFLOAT3 Tangent(XMFLOAT4 P1, XMFLOAT4 P2)
{
	XMFLOAT3 result = { P2.x - P1.x, P2.y - P1.y, P2.z - P1.z };
	Normalize(result);
	return result;
}

// UTILITY FUNCTIONS 

float findZ(XMFLOAT3& P, XMFLOAT3& N, float x, float y)
{
	// a = Nx = N[0]
	// b = Ny = N[1]
	// c = Nz = N[2]
	// a(x - x0) + b(y - y0) + c(z - z0) = 0
	// a(x - x0) + b(y - y0) = -c(z - z0)
	// a(x - x0) + b(y - y0) = -cz + cz0
	// a(x - x0) + b(y - y0) - cz0 = -cz
	// (a(x-x0) + b(y-y0) - cz0) / -c = z
	if (N.z == 0) return NAN;
	float z = (((N.x * (x - P.x)) - (N.y * (y - P.y)) - (N.z * P.z))) / (-N.z);
	return z;
}

float AreaOfTriangle(XMFLOAT3& P, XMFLOAT3& Q, XMFLOAT3& R)
{
	XMFLOAT3 a(Q.x - P.x, Q.y - P.y, Q.z - P.z);
	XMFLOAT3 b(R.x - P.x, R.y - P.y, R.z - P.z);
	XMFLOAT3 c = CROSS(a, b);
	float n = sqrtf((c.x * c.x) + (c.y * c.y) + (c.z * c.z));
	return n / 2.0f;
}
//float y_ = (aZ * bX) - (aX * bZ);

//float largestDifference = -999999.0f;
//float smallestDifference = 9999999.0f;

bool isInside(XMFLOAT3& P, XMFLOAT3& Q, XMFLOAT3& R, XMFLOAT3& S)
{
	// Calculate area of triangle SandBox * 
	float A = AreaOfTriangle(P, Q, R);
	// Calculate area of triangle PBC * 
	float A1 = AreaOfTriangle(S, Q, R);
	if (isnan(A1)) return false;
	// Calculate area of triangle PAC * 
	float A2 = AreaOfTriangle(S, P, R);
	if (isnan(A2)) return false;
	// Calculate area of triangle PAB * 
	float A3 = AreaOfTriangle(S, Q, P);
	if (isnan(A3)) return false;
	float S1 = A1 / A;
	float S2 = A2 / A;
	float S3 = A3 / A;
	float S4 = S1 + S2 + S3;
	//	if ((S1 < 0.0) || (S2 < -0.00003) || (S3 < -0.00003))
	//		return false;
	if ((S1 > 1.00001) || (S2 > 1.00001) || (S3 > 1.00001))
		return false;
	bool results = (fabs((S1 + S2 + S3) - 1.0)) < 0.055;
	return results;
}

float minimum(int idx, XMFLOAT3& P, XMFLOAT3& Q, XMFLOAT3& R)
{
	switch (idx)
	{
		case X:
			if ((P.x < Q.x) && (P.x < R.x))  return P.x;
			return (Q.x < R.x) ? Q.x : R.x;
		case Y:
			if ((P.y < Q.y) && (P.y < R.y))  return P.y;
			return (Q.y < R.y) ? Q.y : R.y;
		case Z:
			if ((P.z < Q.z) && (P.z < R.z))  return P.z;
			return (Q.z < R.z) ? Q.z : R.z;
	}
	return NAN;

}
float maximum(int idx, XMFLOAT3& P, XMFLOAT3& Q, XMFLOAT3& R)
{
	switch (idx)
	{
		case X:
			if ((P.x > Q.x) && (P.x > R.x))  return P.x;
			return (Q.x > R.x) ? Q.x : R.x;
		case Y:
			if ((P.y > Q.y) && (P.y > R.y))  return P.y;
			return (Q.y > R.y) ? Q.y : R.y;
		case Z:
			if ((P.z > Q.z) && (P.z > R.z))  return P.z;
			return (Q.z > R.z) ? Q.z : R.z;
	}
	return NAN;
}

bool isOdd(int n) { return ((int(n) & 0x01) == 1); }

vector<float> toRGBFloat(uint32_t rgb)
{
	float red = float((rgb >> 16) & 0x00FF) / 255.0f;;
	float green = float((rgb >> 8) & 0x00FF) / 255.0f;
	float blue = float(rgb & 0x00FF) / 255.0f;
	return { red, green, blue };
}

void ErrorExit()
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL) == 0)
	{
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
		ExitProcess(dw);
	}

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	//	ExitProcess(dw);
}
#if 0

Texture* getMappingImage(string mapImageName)
{
	return nullptr;
//	omp_set_lock(&map_lock);
	Texture* m_image = textureList[mapImageName];
	if (m_image == nullptr)
	{
		string fname = mapImageName + ".bmp";
		if (std::filesystem::exists(fname))
		{
			//			CString filename(fname.c_str());
			m_image = new Texture();
			if (!m_image->Load(fname))
			{
				ErrorExit();
			}
			textureList[mapImageName] = m_image;
		}
		else
		{
			string msg = "image file" + fname + "does not exist\r\n";
			OutputDebugStringA(msg.c_str());
		}
	}
//	omp_unset_lock(&map_lock);
	return m_image;
}
#endif

float distance(XMFLOAT3 P1, XMFLOAT3 P2)
{
	float dx = P2.x - P1.x;
	float dy = P2.y - P1.y;
	float dz = P2.z - P1.z;
	return sqrtf((dx * dx) + (dy * dy) + (dz * dz));
}
boolean Near(XMFLOAT3 p1, XMFLOAT3 p2)
{
	if (fabs(p2.x - p1.x) > 0.0001) return FALSE;
	if (fabs(p2.y - p1.y) > 0.0001) return FALSE;
	if (fabs(p2.z - p1.z) > 0.0001) return FALSE;
	return TRUE;
}

//===============================================
//===============================================
//===============================================
//===============================================
//===============================================
//===============================================
//===============================================
//===============================================
//===============================================
//===============================================
//===============================================
//===============================================
//===============================================
//===============================================




vector<VertexType> TranslateXMFLOAT4(vector<VertexType> f, XMMATRIX& world, XMMATRIX& camera, XMMATRIX& Perspective)
{
	vector<VertexType> pts;
	XMFLOAT4 results;
	XMMATRIX worldMatrix = world * camera * Perspective;
	for (auto& pt : f)
	{
		XMVECTOR v = XMLoadFloat4(&pt.first);
		v = XMVector3Transform(v, worldMatrix);
		XMStoreFloat4(&results, v);
		pts.push_back(make_pair(results, pt.second));
	}
	return pts;
}

VertexType TranslateXMFLOAT4(VertexType pt, XMMATRIX& world, XMMATRIX& camera, XMMATRIX& Perspective)
{
	XMFLOAT4 results;
	XMMATRIX worldMatrix = world * camera * Perspective;
	XMVECTOR v = XMLoadFloat4(&pt.first);
	v = XMVector3Transform(v, worldMatrix);
	XMStoreFloat4(&results, v);
	return make_pair(results, pt.second);
}

XMFLOAT3 computeBarycentric(const XMFLOAT4& A, const XMFLOAT4& B, const XMFLOAT4& C, const  XMFLOAT4& P)
{
	float det = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
	float factor_alpha = (B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y);
	float factor_beta = (C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y);
	float alpha = factor_alpha / det;
	float beta = factor_beta / det;
	float gamma = 1.0f - (alpha + beta);

	return { alpha,beta,gamma };
}
#if 0
void PlacePixel(float x, float y, uint32_t rgb)
{
	int32_t px = (int32_t)max(x + 0.5f, 0);
	px = (int32_t)min(px, (SCREEN_WIDTH - 1));

	int32_t py = (int32_t)max(y + 0.5f, 0);
	py = (int32_t)min(py, (SCREEN_HEIGHT - 1));

	canvas[py][px] = rgb;
}
#endif

XMFLOAT3 ScaleColor(XMFLOAT3 color, float scale)
{
	return { std::min(color.x * scale, 1.0f), std::min(color.y * scale, 1.0f), std::min(color.z * scale, 1.0f) };
}
XMFLOAT3 ScaleColor(XMFLOAT3 color, XMFLOAT3 scale)
{
	return { std::min(color.x * scale.x, 1.0f), std::min(color.y * scale.y, 1.0f), std::min(color.z * scale.z, 1.0f) };
}

XMFLOAT4 toScreenCoords(XMFLOAT4 p)
{
	return { p.x + GRAPHIC_AREA_HALF_WIDTH,
			(GRAPHIC_AREA_HEIGHT - (p.y + GRAPHIC_AREA_HALF_HEIGHT)), p.z, 1 };
}

std::string ConvertToStrStd(CString cs)
{
	CT2CA pszConvertedAnsiString(cs);
	// construct a std::string using the LPCSTR input
	std::string strStd(pszConvertedAnsiString);
	return strStd;
}

XMFLOAT4 RGB2FRGB(uint32_t c)
{
	float b = (float)(c & 0x000000FF) / 255.0f;
	float g = (float)((c >> 8) & 0x000000FF) / 255.0f;
	float r = (float)((c >> 16) & 0x000000FF) / 255.0f;
	return { r,g,b,1 };
}
uint32_t FRGB2RGB(XMFLOAT4 c)
{
	uint32_t r = (uint32_t)(c.x * 255.0f) & 0x000000FF;
	uint32_t g = (uint32_t)(c.y * 255.0f) & 0x000000FF;
	uint32_t b = (uint32_t)(c.z * 255.0f) & 0x000000FF;
	return (b << 8) | (g << 16) | (r << 24) | 0x00FF;
}

FLOAT DISTANCE(XMFLOAT4 a, XMFLOAT4 b)
{
	return sqrtf(((b.x - a.x) * (b.x - a.x)) + ((b.y - a.y) * (b.y - a.y)) + ((b.z - a.z) * (b.z - a.z)));
}

XMFLOAT4 Normalize(XMFLOAT4 v)
{
	float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	if (length == 0) return { 0,0,0,0 };
	return { v.x / length, v.y / length, v.z / length, 0 };
}

float DOT(XMFLOAT4 a, XMFLOAT4 b) // vectors
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float DOT(XMFLOAT4 a, XMFLOAT4 b, XMFLOAT4 c) // point to vector A is center
{
	XMFLOAT4 v1 = { b.x - a.x, b.y - a.y, b.z - a.z, 0 };
	XMFLOAT4 v2 = { c.x - a.x, c.y - a.y, c.z - a.z, 0 };
	v1 = Normalize(v1);
	v2 = Normalize(v2);
	return DOT(v1, v2);
}

XMFLOAT4 CROSS(XMFLOAT4 a, XMFLOAT4 b)
{
	float x = (a.y * b.z) - (a.z * b.y);
	float y = (a.z * b.x) - (a.x * b.z);
	float z = (a.x * b.y) - (a.y * b.x);
	return { x,y,z,0 };
}

// get the normal of a triangle defined by three points
XMFLOAT4 CROSS(XMFLOAT4 P1, XMFLOAT4 P2, XMFLOAT4 P3) // return the normal
{
	XMFLOAT4 a(P2.x - P1.x, P2.y - P1.y, P2.z - P1.z, 0);
	XMFLOAT4 b(P3.x - P1.x, P3.y - P1.y, P3.z - P1.z, 0);
	XMFLOAT4 result = CROSS(a, b);
	result = Normalize(result);
	return { result.x, result.y, result.z , 0 };
}

XMFLOAT4 SUBTRACT(XMFLOAT4 To, XMFLOAT4 From)
{
	return { To.x - From.x, To.y - From.y, To.z - From.z, 0 };
}

XMFLOAT4 VECTOR(XMFLOAT4 To, XMFLOAT4 From) // return normalized
{
	return Normalize(SUBTRACT(To, From));
}

// This EarClipping is for 3D Polygons from OBJ Reader Face Processor
// Indices are based at 1, so must subtract 1 to get right index for the vertex
// Leave the original index values intact and they will be adjusted 
vector<vertex_index> EarClipping(vector<VertexType> vertexList, vector<XMUINT3>& indices)
{
	vector<XMUINT3>			indexList = indices;
	vector<vertex_index>    meshIndices;
	int		 Vcurrent = 0;
	XMFLOAT4 Vn = { 0,0,0,0 };
	XMFLOAT4 Vc = { 0,0,0,0 };
	XMFLOAT4 Vp = { 0,0,0,0 };
	while (indexList.size() > 3)
	{
		int 	Vnext = ((Vcurrent + 1) >= (int)indexList.size()) ? 0 : (Vcurrent + 1);
		int 	Vprev = ((Vcurrent - 1) < 0) ? ((int)indexList.size() - 1) : (Vcurrent - 1);
		UINT	Icurr = indexList[Vcurrent].x;
		UINT	Inext = indexList[Vnext].x;
		UINT	Iprev = indexList[Vprev].x;
		UINT	Tcurr = indexList[Vcurrent].y;
		UINT	Tnext = indexList[Vnext].y;
		UINT	Tprev = indexList[Vprev].y;
		UINT	Ncurr = indexList[Vcurrent].z;
		UINT	Nnext = indexList[Vnext].z;
		UINT	Nprev = indexList[Vprev].z;
		// start at the first vertex and create triangle edges to the next vector and the previous vertex
		Vn = vertexList[Inext - 1].first;
		Vc = vertexList[Icurr - 1].first;
		Vp = vertexList[Iprev - 1].first;
		// determine if the angle between the two edges is convex or concave - continue if convex
		XMFLOAT4 cross = CROSS(Vc, Vp, Vn);
		// DOT product of the two edges should be negative for a concave angle
		float d = DOT(cross, VECTOR(Vc, Vp));
		if (d < -0.000001) // convex
		{
			Vcurrent = ((Vcurrent + 1) >= (int)indexList.size()) ? 0 : (Vcurrent + 1);
			continue;
		}
		bool isEar = true;
		for (int i = 0; i < (int)indexList.size(); i++)
		{
			if (i == Vcurrent) continue;
			if (i == Vnext) continue;
			if (i == Vprev) continue;
			UINT Ip = indexList[i].x;
			XMFLOAT4 p = vertexList[Ip - 1].first;

			if ((Vc.x == p.x) and (Vc.y == p.y) and (Vc.z == p.z)) continue;
			if ((Vn.x == p.x) and (Vn.y == p.y) and (Vn.z == p.z)) continue;
			if ((Vp.x == p.x) and (Vp.y == p.y) and (Vp.z == p.z)) continue;

			auto bary = computeBarycentric(Vc, Vn, Vp, p);
			if (bary.x == -0.0f) bary.x = 0;
			if (bary.y == -0.0f) bary.y = 0;
			if (bary.z == -0.0f) bary.z = 0;
			// if they do, then this is not an ear - continue
			if (((bary.x >= 0) and (bary.x <= 1)) and
				((bary.y >= 0) and (bary.y <= 1)) and
				((bary.z >= 0) and (bary.z <= 1)))
			{
				// this coordinate is within the triangle, so don't use
				isEar = false;
				break;
				//				stuckCount++;
			}
		}
		if (!isEar)
		{
			// this is not an ear, so continue to the next vertex
			Vcurrent = ((Vcurrent + 1) >= (int)indexList.size()) ? 0 : (Vcurrent + 1);
			continue;
		}
		meshIndices.push_back({ { Iprev, Icurr, Inext },{Tprev, Tcurr, Tnext},{Nprev, Ncurr, Nnext} });
		// start from the beginning again
		indexList.erase(indexList.begin() + Vcurrent);
		Vcurrent = 0;
	}
	meshIndices.push_back({ { indexList[0].x, indexList[1].x, indexList[2].x },
		{indexList[0].y, indexList[1].y, indexList[2].y},
		{indexList[0].z, indexList[1].z, indexList[2].z} });
	return meshIndices;
}
FLOAT AreClockWise(XMFLOAT4 c, XMFLOAT4 p, XMFLOAT4 n)
{
	return ((c.x * p.y) - (c.y * p.x)) + ((p.x * n.y) - (p.y * n.x)) + ((n.x * c.y) - (n.y * c.x));
}
// This Ear Clipping routine is for flat 2D TrueType Glyph polygons
pair<vector<VertexType>, vector<XMUINT3>> EarClipping(vector<VertexType>& vertices)
{
	vector<VertexType> vertexList = vertices;
	vector<VertexType> meshVertices;
	vector<XMUINT3> meshIndices;
	//	XMFLOAT4 target = { vertices[12].first.x,vertices[12].first.y,vertices[12].first.z,1 };

	auto findVertex = [&meshVertices](XMFLOAT4 v, XMFLOAT4 c) -> uint32_t
		{
			for (int i = 0; i < (int)meshVertices.size(); i++)
			{
				if ((meshVertices[i].first.x == v.x) and
					(meshVertices[i].first.y == v.y) and
					(meshVertices[i].first.z == v.z))
				{
					return i;
				}
			}
			uint32_t index = (uint32_t)meshVertices.size();
			auto newV = make_pair(v, c);
			meshVertices.push_back(newV);
			return index;
		};

	int		Vcurrent = 0;
	XMFLOAT4 Vn = { 0,0,0,0 };
	XMFLOAT4 Vc = { 0,0,0,0 };
	XMFLOAT4 Vp = { 0,0,0,0 };
	XMFLOAT4 Cn = { 0,0,0,1 };
	XMFLOAT4 Cc = { 0,0,0,1 };
	XMFLOAT4 Cp = { 0,0,0,1 };
	//	int stuckCount = 0;
	while (vertexList.size() > 3)
	{
		int 	Vnext = ((Vcurrent + 1) >= (int)vertexList.size()) ? 0 : (Vcurrent + 1);
		int 	Vprev = ((Vcurrent - 1) < 0) ? ((int)vertexList.size() - 1) : (Vcurrent - 1);
		// start at the first vertex and create triangle edges to the next vector and the previous vertex
		Vn = vertexList[Vnext].first;
		Vc = vertexList[Vcurrent].first;
		Vp = vertexList[Vprev].first;
		Cn = vertexList[Vnext].second;
		Cc = vertexList[Vcurrent].second;
		Cp = vertexList[Vprev].second;
		//		if ((Vc.x == target.x) and (Vc.y == target.y) and (Vc.z == target.z))
		//		{
		//			stuckCount++;
		//		}

		if ((Vc.x == Vn.x) and (Vc.y == Vn.y) and (Vc.z == Vn.z))
		{
			vertexList.erase(vertexList.begin() + Vnext);
			continue;
		}
		if ((Vc.x == Vp.x) and (Vc.y == Vp.y) and (Vc.z == Vp.z))
		{
			vertexList.erase(vertexList.begin() + Vprev);
			continue;
		}
		if ((Vn.x == Vp.x) and (Vn.y == Vp.y) and (Vn.z == Vp.z))
		{
			vertexList.erase(vertexList.begin() + Vnext);
			continue;
		}
		// determine if the angle between the two edges is convex or concave - continue if convex
		// DOT product of the two edges should be negative for a concave angle
//		if (stuckCount > vertexList.size()) break;
		XMFLOAT4 cross = CROSS(Vc, Vp, Vn);
		if (cross.z <= 0) // convex
		{
			Vcurrent = ((Vcurrent + 1) >= (int)vertexList.size()) ? 0 : (Vcurrent + 1);
			//			stuckCount++;
			//			if (stuckCount > vertexList.size()) 
			//				break; // if we have looped
			continue;
		}
		// use barycentric coordinates from other vertices to determine if they fall within the triangle
		bool isEar = true;
		for (int i = 0; i < (int)vertexList.size(); i++)
		{
			if (i == Vcurrent) continue;
			if (i == Vnext) continue;
			if (i == Vprev) continue;
			auto p = vertexList[i].first;
			if ((Vc.x == p.x) and (Vc.y == p.y) and (Vc.z == p.z)) continue;
			if ((Vn.x == p.x) and (Vn.y == p.y) and (Vn.z == p.z)) continue;
			if ((Vp.x == p.x) and (Vp.y == p.y) and (Vp.z == p.z)) continue;

			auto bary = computeBarycentric(Vc, Vn, Vp, p);
			if (bary.x == -0.0f) bary.x = 0;
			if (bary.y == -0.0f) bary.y = 0;
			if (bary.z == -0.0f) bary.z = 0;
			// if they do, then this is not an ear - continue
			if (((bary.x >= 0) and (bary.x <= 1)) and
				((bary.y >= 0) and (bary.y <= 1)) and
				((bary.z >= 0) and (bary.z <= 1)))
			{
				// this coordinate is within the triangle, so don't use
				isEar = false;
				break;
				//				stuckCount++;
			}
		}
		if (!isEar)
		{
			// this is not an ear, so continue to the next vertex
			Vcurrent = ((Vcurrent + 1) >= (int)vertexList.size()) ? 0 : (Vcurrent + 1);
			continue;
		}
		//		if ((Vc.x == target.x) and (Vc.y == target.y) and (Vc.z == target.z))
		//		{
		//			stuckCount++;
		//		}
		//		stuckCount = 0; // reset the stuck count if we find a convex angle


		// otherwise, this is an ear - add the triangle to the list and remove the center vertex from the polygon
		// generate a triangle and remove the Vcurrent from the mix
		// triangle is Vp, Vc, Vn with colors Cp, Cc, Cn
		uint32_t iVc = findVertex(Vc, Cc);
		uint32_t iVn = findVertex(Vn, Cn);
		uint32_t iVp = findVertex(Vp, Cp);
		// save vertices in counter clockwise direction
		FLOAT S = AreClockWise(Vc, Vp, Vn); // want couter clockwisse
		if (S > 0) meshIndices.push_back({ iVc, iVn, iVp });
		if (S < 0) meshIndices.push_back({ iVc, iVp, iVn });
		// if S == 0, the triangle has only one dimension
		// start from the beginning again
		vertexList.erase(vertexList.begin() + Vcurrent);
		Vcurrent = 0;
	}
	// continue until only three vertices remain - add the final triangle to the list
	Vn = vertexList[0].first;
	Vc = vertexList[1].first;
	Vp = vertexList[2].first;
	Cn = vertexList[0].second;
	Cc = vertexList[1].second;
	Cp = vertexList[2].second;
	uint32_t iVc = findVertex(Vc, Cc);
	uint32_t iVn = findVertex(Vn, Cn);
	uint32_t iVp = findVertex(Vp, Cp);
	FLOAT S = AreClockWise(Vc, Vp, Vn); // want couter clockwisse
	if (S > 0) meshIndices.push_back({ iVc, iVn, iVp });
	if (S < 0) meshIndices.push_back({ iVc, iVp, iVn });
	// if S == 0, the triangle has only one dimension
// return the resulting vertex list and index list
	return make_pair(meshVertices, meshIndices);
}
bool isFirstInsideOfSecond(vector<VertexType> First, vector<VertexType> Second)
{
	XMFLOAT3 first_min = { 9999999.0f,  9999999.0f,  9999999.0f };
	XMFLOAT3 first_max = { -9999999.0f, -9999999.0f, -9999999.0f };
	XMFLOAT3 second_min = { 9999999.0f,  9999999.0f,  9999999.0f };
	XMFLOAT3 second_max = { -9999999.0f, -9999999.0f, -9999999.0f };
	for (auto& f : First)
	{
		XMFLOAT4 value = f.first;
		first_min = { min(first_min.x, value.x), min(first_min.y, value.y), min(first_min.z, value.z) };
		first_max = { max(first_max.x, value.x), max(first_max.y, value.y), max(first_max.z, value.z) };
	}
	for (auto& s : Second)
	{
		XMFLOAT4 value = s.first;
		second_min = { min(second_min.x, value.x), min(second_min.y, value.y), min(second_min.z, value.z) };
		second_max = { max(second_max.x, value.x), max(second_max.y, value.y), max(second_max.z, value.z) };
	}
	if (first_min.x > second_max.x) return false;
	if (first_max.x < second_min.x) return false;
	if (first_min.y > second_max.y) return false;
	if (first_max.y < second_min.y) return false;
	if (first_min.z > second_max.z) return false;
	if (first_max.z < second_min.z) return false;
	return true;

}
vector<XMFLOAT4> DoNotUseList;
bool CoordinateIsOnTheDoNotUseList(float x, float y)
{
	for (auto& v : DoNotUseList)
	{
		if ((v.x == x) and (v.y == y)) return true;
	}
	return false;
}
// This EarClipping routine is to inspect multiple polygons in a TrueType glyph and 
// decides whether they are apart from each other and handle separately (chars i j : ; ? !)
// or embedded holes in glyph outline in which case, the two / three polygons will be merged into one
pair<vector<VertexType>, vector<XMUINT3>> EarClipping(vector<vector<VertexType>>& polygons)
{
	DoNotUseList.erase(DoNotUseList.begin(), DoNotUseList.end());
	DoNotUseList.clear();
	DoNotUseList = {};

	if (polygons.size() == 0) return { {},{} };
	vector<vector<VertexType>> vertexList = polygons;
	// initialize the collection of vertices with the first polygon in the list
	vector<VertexType> Collection = vertexList[0];
	// remove the first polygon from the list
	vertexList.erase(vertexList.begin());
	// all others will be added at their nearest point in the collection
	// if there is a second polygon AND it has coordinates outside of the main polygon (i and j)
	// treat this as two separate objects and then combine their vertices
	if ((vertexList.size() == 1) and (not isFirstInsideOfSecond(vertexList[0], Collection))) // check if second polygon is outside of the first
	{
		auto mainSection = EarClipping(Collection);
		auto outSection = EarClipping(vertexList[0]);
		int offset = (int)mainSection.first.size();
		for (auto outer : outSection.first)
		{
			mainSection.first.push_back(outer);
		}
		for (auto outer : outSection.second)
		{
			mainSection.second.push_back({ outer.x + offset,outer.y + offset, outer.z + offset });
		}
		return mainSection;
	}
	for (auto& poly : vertexList)
	{
		// last item in poly is same as first. remove it
		poly.pop_back();
		// find the nearest point in the collection to the first point in the polygon
		float minDist = FLT_MAX;
		int minIndexC = 0;
		int minIndexP = 0;
		for (int i = 0; i < (int)Collection.size(); i++)
		{
			if (CoordinateIsOnTheDoNotUseList(Collection[i].first.x, Collection[i].first.y)) continue;
			// go through each point and find the nearest point in the collection
			for (int j = 0; j < (int)poly.size(); j++)
			{
				// preventing a second or third ... internal polygon from attaching to another internal polygon
				float dx = Collection[i].first.x - poly[j].first.x;
				float dy = Collection[i].first.y - poly[j].first.y;
				float dist = sqrtf(dx * dx + dy * dy);
				if (dist < minDist)
				{
					minDist = dist;
					minIndexC = i;
					minIndexP = j;
				}
			}
		}
		// insert the polygon into the collection at the nearest point
		// insert an extra point in the collection at the nearest point to the polygon
		int number_to_insert = (int)poly.size();
		for (auto& p : poly)
		{
			DoNotUseList.insert(DoNotUseList.end(), p.first);
		}

		int points_to_end_of_polygon = (int)poly.size() - minIndexP;
		int remaining = (int)poly.size() - points_to_end_of_polygon;
		// save the connection point in the collection to insert back after going through the polygon
		auto connectionPoint = Collection[minIndexC];
		DoNotUseList.insert(DoNotUseList.end(), connectionPoint.first); // and do not attach another polygon to the same spot
		// add the polygon to the collection at the nearest point
		Collection.insert(Collection.begin() + minIndexC + 1, poly.begin() + minIndexP, poly.end());
		// remove the points from the polygon that was added to the collection ecept for the first one
		// it is the exit point from the polygon to the collection
		poly.erase(poly.begin() + minIndexP + 1, poly.end());
		if (poly.size() > 0)
		{
			// add the remaining points in the polygon to the collection
			Collection.insert(Collection.begin() + minIndexC + points_to_end_of_polygon + 1, poly.begin(), poly.end());
		}
		// insert an extra point in the polygon at the nearest point to the collection
		// to return to the collection after going through the polygon
		Collection.insert(Collection.begin() + minIndexC + number_to_insert + 2, connectionPoint);
	}
	// send the collection to the ear clipping function to generate a triangle list
	DoNotUseList.erase(DoNotUseList.begin(), DoNotUseList.end());
	DoNotUseList.clear();
	DoNotUseList = {};
	return EarClipping(Collection);
}

vector<string> split(const string& str, char delimiter)
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

wstring string_to_wstring_mb(const std::string& narrow_str)
{
	std::wstring wide_str(narrow_str.length(), L' ');
	size_t converted_chars = mbstowcs(&wide_str[0], narrow_str.c_str(), narrow_str.length());
	if (converted_chars == static_cast<size_t>(-1))
	{
		// Handle conversion error
		std::cerr << "Error during conversion" << std::endl;
		return L"";
	}
	return wide_str;
}
