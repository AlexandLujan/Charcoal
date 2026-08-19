#pragma once

#include <cmath>
#include <vector>

//using namespace std;
#define LIGHT_BACKGROUND_COLOR_CHANGED	0x1001
#define LIGHT_AMBIENT_COLOR_CHANGED		0x1002
#define LIGHT_0_COLOR_CHANGED			0x2001
#define LIGHT_1_COLOR_CHANGED			0x2002
#define LIGHT_2_COLOR_CHANGED			0x2003
#define LIGHT_0_POSITION_X_CHANGED		0x3001
#define LIGHT_0_POSITION_Y_CHANGED		0x3002
#define LIGHT_0_POSITION_Z_CHANGED		0x3003
#define LIGHT_1_POSITION_X_CHANGED		0x4001
#define LIGHT_1_POSITION_Y_CHANGED		0x4002
#define LIGHT_1_POSITION_Z_CHANGED		0x4003
#define LIGHT_2_POSITION_X_CHANGED		0x5001
#define LIGHT_2_POSITION_Y_CHANGED		0x5002
#define LIGHT_2_POSITION_Z_CHANGED		0x5003
#define LIGHT_0_TARGET_X_CHANGED		0x6001
#define LIGHT_0_TARGET_Y_CHANGED		0x6002	
#define LIGHT_0_TARGET_Z_CHANGED		0x6003
#define LIGHT_1_TARGET_X_CHANGED		0x7001
#define LIGHT_1_TARGET_Y_CHANGED		0x7002
#define LIGHT_1_TARGET_Z_CHANGED		0x7003
#define LIGHT_2_TARGET_X_CHANGED		0x8001
#define LIGHT_2_TARGET_Y_CHANGED		0x8002
#define LIGHT_2_TARGET_Z_CHANGED		0x8003
#define LIGHT_0_ON_CHANGED				0x9001
#define LIGHT_1_ON_CHANGED				0x9002
#define LIGHT_2_ON_CHANGED				0x9003

class Lighting
{
protected:
	vector<float> intensity = { 0.5,0.5,0.5 };
public:
	Lighting(float n = 0.5f)
	{
		intensity[RED] = n;
		intensity[GREEN] = n;
		intensity[BLUE] = n;
	}
	Lighting(vector<float> n)
	{
		intensity[RED] = n[RED];
		intensity[GREEN] = n[GREEN];
		intensity[BLUE] = n[BLUE];
	}
	virtual vector<float> getIntensity() { return intensity; }
	virtual void setIntensity(vector<float> n)
	{
		intensity[RED] = n[RED];
		intensity[GREEN] = n[GREEN];
		intensity[BLUE] = n[BLUE];
	}
};

class PositionedLighting : public Lighting
{
	XMFLOAT3 placed_location;
	XMFLOAT3 last_location;
	XMFLOAT3 lightTarget;
	XMFLOAT3 location;
	XMFLOAT3 lightVector;
	boolean status;

public:
	PositionedLighting(XMFLOAT3 _location, XMFLOAT3 _lightTarget, vector<float> _intensity, boolean _status = false) : Lighting(_intensity)
	{
		placed_location = _location;
		lightTarget = _lightTarget;
		status = _status;
	}
	void SetLocation(XMFLOAT3 here) { placed_location = here; }
	void SetTarget(XMFLOAT3 here) { lightTarget = here; }
	void SetVector(XMFLOAT3 howmuch)
	{
		lightVector = howmuch;
		Normalize(lightVector);
	}
	void Translation(XMMATRIX& trans);
	XMFLOAT3& Vector() { return lightVector; } // { lightVector.x(), lightVector.y(), lightVector.z() };
	XMFLOAT3& PlacedLocation() { return placed_location; } // { placed_location.x(), placed_location.y(), placed_location.z() };
	XMFLOAT3& TargetLocation() { return lightTarget; } // { lightTarget.x(), lightTarget.y(), lightTarget.z() };
	XMFLOAT3& Location() { return location; } //{ location.x(),location.y(),location.z() }; }
	XMFLOAT3& LastLocation() { return last_location; };
	boolean IsOn() { return status; }
	boolean& Status() { return status; }
	vector<float> CalculateSpectralLighting(XMFLOAT3 surfacePoint, XMFLOAT3 surfaceNormal, XMFLOAT3 eye);

};
