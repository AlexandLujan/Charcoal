#include "pch.h"
#include "Utility.h"
#include "Lighting.h"

vector<float> PositionedLighting::CalculateSpectralLighting(XMFLOAT3 surfacePoint, XMFLOAT3 surfaceNormal, XMFLOAT3 eye)
{
	XMFLOAT3 hit_light_vector = { surfacePoint.x - lightVector.x,surfacePoint.y - lightVector.y,surfacePoint.z - lightVector.z };
	Normalize(hit_light_vector);
	XMFLOAT3 eye_hit_vector = { eye.x - surfacePoint.x,eye.y - surfacePoint.y,eye.z - surfacePoint.z };
	Normalize(eye_hit_vector);
	float cosTheta = DOT(hit_light_vector, surfaceNormal);
	// reflection = light vector - (2 * surfaceNormal * DOT(sufraceNormal, light_vector))
	XMFLOAT3 temp = {surfaceNormal.x * (cosTheta * 2.0f), surfaceNormal.y * (cosTheta * 2.0f), surfaceNormal.z * (cosTheta * 2.0f)};
	XMFLOAT3 reflection_vector = { hit_light_vector.x - temp.x,hit_light_vector.y - temp.y, hit_light_vector.z - temp.z };
	Normalize(reflection_vector);
	XMFLOAT3 h = { eye_hit_vector.x + reflection_vector.x, eye_hit_vector.y + reflection_vector.y, eye_hit_vector.z + reflection_vector.z };
	Normalize(h);
	float cosAlpha = DOT(h, surfaceNormal);
	float k1 = 0.9f;
	float k2 = 0.9f;
	float n = 5000.0f;
	float iRed = min(1.0f, intensity[RED] * ((cosTheta * k1) + (pow(cosAlpha, n) * k2)));
	float iGreen = min(1.0f, intensity[GREEN] * ((cosTheta * k1) + (pow(cosAlpha, n) * k2)));
	float iBlue = min(1.0f, intensity[BLUE] * ((cosTheta * k1) + (pow(cosAlpha, n) * k2)));
	vector<float> i = { fabs(iRed), fabs(iGreen), fabs(iBlue) };
	return i;
}

void PositionedLighting::Translation(XMMATRIX& trans)
{
	XMFLOAT4 result = {0,0,0,0};
	XMFLOAT4 Vp(placed_location.x,placed_location.y,placed_location.z,1);
	XMFLOAT4 Vt(lightTarget.x,lightTarget.y,lightTarget.z,1);
//	result = trans * Vp;
	XMFLOAT3 a(result.x, result.y, result.z);
//	trans.multiplyMatrices(Vt, result);
	XMFLOAT3 b(result.x, result.y, result.z);
	location = a;
	lightVector.x = b.x - a.x;
	lightVector.y = b.y - a.y;
	lightVector.z = b.z - a.z;
	Normalize(lightVector);
}

