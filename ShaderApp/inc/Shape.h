#pragma once
#error "this file is no longer used"
#include <algorithm>
#include <iostream>
#include <math.h>
#include <pch.h>
#include <stdint.h>
#include <string>
//#include "Point3D.h"
//#include "Matrix.h"
//#include "rgbFloat.h"

//using namespace std;

extern XMFLOAT4 FindColor(string cName);

enum Translation
{
	Original,
	Placed,
	Viewed
};

class HitPoint;

class Shape
{
private:
	//	string marker = "shape";
	uint32_t ID;
	static uint32_t NextID;
protected:
	string theName;
	XMFLOAT3 original;
	XMFLOAT3 last;
	string color;
	XMFLOAT3 rgb;
	string surface;
	float roll = 0;
	float pitch = 0;
	float yaw = 0;
	float scale = 0;
	float dx = 0;
	float dy = 0;
	float dz = 0;
	XMFLOAT3 normal_static = { 0,0,0 };
	XMFLOAT3 normal_hit = { 0,0,0 };
	//	Point3D hitPoint = { 0,0,0 };


public:
	Shape(string name, float cx = 0.0f, float cy = 0.0f, float cz = 0.0f, string clr = "", string surfaceIndex = "") : theName(name), surface(surfaceIndex)
	{
		original = { cx, cy, cz };
		last = original;
		color = clr;
		rgb = FindColor(clr);
		ID = 0;
	}

	Shape(string name, XMFLOAT3& cx, string clr = "", string surfaceIndex = "") : theName(name), surface(surfaceIndex)
	{
		original = cx;
		last = original;
		color = clr;
		rgb = FindColor(clr);
		ID = 0;
	}
	virtual ~Shape() {}
	virtual Shape* Translate(Translation t, XMMATRIX& trans) = 0;
	virtual HitPoint* rayTrace(XMFLOAT3& eyeLocation, XMFLOAT3& rayVector, boolean wantFurthest = true) = 0;
	virtual string isa(void) = 0;// { return "shape"; }
	virtual string where(void) { return "no location"; }
	virtual void SetTranslatedNormal(Translation t) {}

	XMFLOAT3 pixelColor(void) const { return rgb; }
	string& Surface() { return surface; }
	void setID(void) { ID = ++NextID; }
	void setID(uint32_t n) { ID = n; }
	uint32_t getID(void) const { return ID; }
	//	float getHitPointZ(void) { return hitPoint.z(); }
	//	Point3D& getHitPoint(void) { return hitPoint; }
	//	Point3D& surfaceNormal(void) { return normal_hit; }
	XMFLOAT3& Original(void) { return original; }
	XMFLOAT3& Last(void) { return last; }
	string getName(void) { return theName; }
	//	void clearHitPoint() { hitPoint = { 0,0,0 }; }
	virtual void getExtents(XMFLOAT3& minExtent, XMFLOAT3& maxExtent)
	{
		minExtent.x = FLT_MAX;
		minExtent.y = FLT_MAX;
		minExtent.z = FLT_MAX;
		maxExtent.x = FLT_MIN;
		maxExtent.y = FLT_MIN;
		maxExtent.z = FLT_MIN;
	}
	virtual XMFLOAT2 surfaceSize(void) { return { 0,0 }; }
	virtual XMFLOAT3 GetTextureColor(HitPoint* hitPoint) = 0; // { return { 0.0f,0.0f,0.0f }; };
	virtual XMFLOAT3 surfacePoint(XMFLOAT3 p) = 0;

};

class HitPoint
{
	//	string marker = "hit";
	Shape* hitShape;
	XMFLOAT3 theHit;
	XMFLOAT3 eye;
	int32_t screen_x;
	int32_t screen_y;
	XMFLOAT3 _rgbColor;
	XMFLOAT3 _surfaceNormal;
public:
	HitPoint()
	{
		hitShape = nullptr;
		theHit = { 0,0,0 };
		eye = { 0,0,0 };
		screen_x = 0;
		screen_y = 0;
		_rgbColor = { 0,0,0 };; // rgbFloat(0);
	}
	HitPoint(Shape* s, XMFLOAT3 hitPoint, XMFLOAT3 normal)
	{
		hitShape = s;
		theHit = hitPoint;
		_surfaceNormal = normal;
		eye = { 0,0,0 };
		screen_x = 0;
		screen_y = 0;
		_rgbColor = { 0,0,0 }; // rgbFloat(0);
	}
	HitPoint(HitPoint* s)
	{
		hitShape = s->getHitShape();
		theHit = s->theHit; // { s->x(), s->y(), s->zetHitPointZ() }; // s->getHitPoint();
		_surfaceNormal = s->surfaceNormal();
		eye = { 0,0,0 };
		screen_x = 0;
		screen_y = 0;
		_rgbColor = { 0,0,0 }; // rgbFloat(0);
	}
	int32_t& x() { return screen_x; }
	int32_t& y() { return screen_y; }
	XMFLOAT3& eyePoint() { return eye; }
	XMFLOAT3& rgbColor() { return _rgbColor; }

	Shape* getHitShape() { return hitShape; }
	XMFLOAT3 getHitPoint() { return theHit; }
	XMFLOAT3& hitPoint() { return theHit; }
	float getHitPointZ(void) { return theHit.z; }
	XMFLOAT3& surfaceNormal(void) { return _surfaceNormal; }
	void setrgbColor(XMFLOAT3 other)
	{
		_rgbColor = other;
	}

	void updateHitPoint(HitPoint* h)
	{
		hitShape = h->getHitShape();
		theHit = h->theHit; // getHitPoint();
	}
	//	string& Marker() { return marker; }
};

