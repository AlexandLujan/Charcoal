#pragma once

#include <map>
#include <string>
#include <vector>
//#include <iostream>
//#include <sstream>
//#include <gdiplus.h>
//#include <iomanip>
//#include "Scheduler.h"
#define PICTURE_WIDTH 1200
#define PICTURE_HEIGHT 900
const int RED = 0;
const int GREEN = 1;
const int BLUE = 2;
const int X = 0;
const int Y = 1;
const int Z = 2;

//using namespace std;

enum TrackingStyle
{
	DoNotTrack,
	TrackToSetLocation,
	TrackToObject,
	TrackToOrigin
};
const TrackingStyle allStyles[] = { DoNotTrack,TrackToSetLocation,TrackToObject,TrackToOrigin };

class Object;
class Lighting;
class Camera;
//class Texture;
//class Point3D;
//class Scheduler;

extern uint32_t backgroundColor;
extern uint32_t picture[PICTURE_HEIGHT][PICTURE_WIDTH];

extern std::map<std::string, std::shared_ptr<Object>> ObjectList;
//extern vector<Shape*> LayoutList;
extern std::map<std::string, Lighting*> LightingList;
extern std::vector<std::string> LightNames;
extern std::vector<uint32_t> LightColors;
extern std::map<std::string, std::shared_ptr<Camera>> CameraList;
extern std::vector<std::string> CameraNames;
//extern Scheduler theScheduler;
//extern map<string, Texture*> textureList;
//extern vector<Shape*> ViewList;

//extern omp_lock_t raytrace_lock; // Declare an OpenMP lock variable
//extern omp_lock_t shading_lock; // Declare an OpenMP lock variable
//extern omp_lock_t texture_lock; // Declare an OpenMP lock variable
//extern omp_lock_t lighting_lock; // Declare an OpenMP lock variable
//extern omp_lock_t plot_lock; // Declare an OpenMP lock variable
//extern omp_lock_t map_lock; // Declare an OpenMP lock variable

typedef struct vertex_index vertex_index;
struct vertex_index
{
public:
	vertex_index()
	{
		v_index = { 0,0,0 };
		t_index = { 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF };
		n_index = { 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF };
	}
	vertex_index(XMUINT3 i, XMUINT3 t, XMUINT3 n)
	{
		v_index = i;
		t_index = t;
		n_index = n;
	}
	XMUINT3 v_index;
	XMUINT3 t_index;
	XMUINT3 n_index;
};

//typedef std::pair<XMFLOAT4, std::string> VertexType;
typedef std::pair<XMFLOAT4, XMFLOAT4> VertexType; // location and color FRGB


COLORREF RGB_Color(std::vector<float> intensity);
COLORREF windowsColor(uint32_t red, uint32_t green, uint32_t blue);
COLORREF windowsColor(std::vector<float> intensity);
FLOAT DISTANCE(XMFLOAT4 a, XMFLOAT4 b);
FLOAT DOT(XMFLOAT4 a, XMFLOAT4 b);
XMFLOAT3 CROSS(XMFLOAT3 a, XMFLOAT3 b);
XMFLOAT4 FindColor(std::string cName);
XMFLOAT3 Normal(XMFLOAT3 P1, XMFLOAT3 P2, XMFLOAT3 P3); // return the normal
XMFLOAT3 Normal(XMFLOAT4 P1, XMFLOAT4 P2, XMFLOAT4 P3); // return the normal
XMFLOAT3 Tangent(XMFLOAT3 P1, XMFLOAT3 P2);
XMFLOAT3 Tangent(XMFLOAT4 P1, XMFLOAT4 P2);
XMFLOAT4 RGB2FRGB(uint32_t c);
XMFLOAT3 ScaleColor(XMFLOAT3 color, XMFLOAT3 scale);
XMFLOAT3 ScaleColor(XMFLOAT3 color, float scale);
XMFLOAT3 computeBarycentric(const XMFLOAT4& A, const XMFLOAT4& B, const XMFLOAT4& C, const  XMFLOAT4& P);
XMFLOAT4 CROSS(XMFLOAT4 P1, XMFLOAT4 P2, XMFLOAT4 P3); // return the normal
XMFLOAT4 Normalize(XMFLOAT4 v);
XMFLOAT4 SUBTRACT(XMFLOAT4 To, XMFLOAT4 From);
XMFLOAT4 VECTOR(XMFLOAT4 To, XMFLOAT4 From); // return normalized
XMFLOAT4 intersect_line_plane_v3(XMFLOAT4 eye, XMFLOAT4 v, XMFLOAT4 p_co, XMFLOAT4 p_no);
XMFLOAT4 toScreenCoords(XMFLOAT4 p);
bool isInside(XMFLOAT3& P, XMFLOAT3& Q, XMFLOAT3& R, XMFLOAT3& S);
bool isInside(XMFLOAT4 v0, XMFLOAT4 v1, XMFLOAT4 v2, XMFLOAT4 P);
bool isOdd(int n);
boolean Near(XMFLOAT3 p1, XMFLOAT3 p2);
float AreaOfTriangle(XMFLOAT3& P, XMFLOAT3& Q, XMFLOAT3& R);
float DOT(XMFLOAT3 a, XMFLOAT3 b);
float distance(XMFLOAT3 P1, XMFLOAT3 P2);
float findZ(XMFLOAT3& P, XMFLOAT3& N, float x, float y);
float maximum(int idx, XMFLOAT3& P, XMFLOAT3& Q, XMFLOAT3& R);
float minimum(int idx, XMFLOAT3& P, XMFLOAT3& Q, XMFLOAT3& R);
VertexType TranslateXMFLOAT4(VertexType pt, XMMATRIX& world, XMMATRIX& camera, XMMATRIX& Perspective);
std::pair<std::vector<VertexType>, std::vector<XMUINT3>> EarClipping(std::vector<VertexType>& vertices);
std::pair<std::vector<VertexType>, std::vector<XMUINT3>> EarClipping(std::vector<std::vector<VertexType>>& polygons);
std::vector<vertex_index> EarClipping(std::vector<VertexType> vertexList, std::vector<XMUINT3>& indices);
std::string ConvertToStrStd(CString cs);
std::string formatFloat(float n);
uint32_t FRGB2RGB(XMFLOAT4 c);
uint32_t colorswap(uint32_t n);
std::vector<float> toRGBFloat(uint32_t rgb);
std::vector<VertexType>TranslateXMFLOAT4(std::vector<VertexType> f, XMMATRIX& world, XMMATRIX& camera, XMMATRIX& Perspective);
std::vector<std::string> split(const std::string& str, char delimiter);
void ClearCanvas();
void Normalize(XMFLOAT3& c);
//void PlacePixel(float x, float y, uint32_t rgb);
void SetCameraRotation(std::shared_ptr<Camera> c, int32_t _pitch, int32_t _yaw, int32_t _roll);
std::wstring string_to_wstring_mb(const std::string& narrow_str);

inline float DegreesToRadians(float degrees)
{
	return degrees * 3.14159265358979323846f / 180.0f;
}
inline float RadiansToDegrees(float radians)
{
	return (radians * 180.0f) / 3.14159265358979323846f;
}


