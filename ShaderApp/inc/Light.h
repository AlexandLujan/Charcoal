#pragma once

/*
 *  Copyright(c) 2020 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

 /**
  *  @file Light.h
  *  @date November 10, 2020
  *  @author Jeremiah van Oosten
  *
  *  @brief Light structures that use HLSL constant buffer padding rules.
  */
#include <DirectXMath.h>
#include <string>
#include "Utility.h"

//using namespace std;
//using namespace DirectX;

class Light
{
public:
	XMFLOAT4 PositionWS;  // Light position in world space.
	//----------------------------------- (16 byte boundary)
	XMFLOAT4 PositionVS;  // Light position in view space.
	//----------------------------------- (16 byte boundary)
	XMFLOAT4 color;
	std::string colorName;
	//----------------------------------- (16 byte boundary)
	FLOAT ambient;

	FLOAT diffuseIntensity;
	FLOAT specularIntensity;

	FLOAT ConstantAttenuation;
	FLOAT LinearAttenuation;
	FLOAT QuadraticAttenuation;
	bool status = false;
	static FLOAT Nothing;
	static XMFLOAT4 Nothing4;
	//----------------------------------- (16 byte boundary)
	// Total:  
	//                           16 * 4 = 64 bytes

	Light(
		XMFLOAT4 _PositionWS, 
		XMFLOAT4 _PositionVS, 
		std::string _Color,
		float _Ambient, 
		float _DiffuseIntensity,
		float _SpecularIntensity,
		float _ConstantAttenuation, 
		float _LinearAttenuation, 
		float _QuadraticAttenuation) 
		:
		PositionWS(_PositionWS),
		PositionVS(_PositionVS),
		colorName(_Color),
		ambient(_Ambient),
		diffuseIntensity(_DiffuseIntensity),
		specularIntensity(_SpecularIntensity),
		ConstantAttenuation(_ConstantAttenuation),
		LinearAttenuation(_LinearAttenuation),
		QuadraticAttenuation(_QuadraticAttenuation)
	{
		color = FindColor(colorName);
	}
	void TurnOn() { status = true; }
	void TurnOff() { status = false; }
	std::string& Color() { return colorName; }
	XMFLOAT4& FRGB() { return color; }
	XMFLOAT4& Position() { return PositionWS; }
	FLOAT& X() { return PositionWS.x; }
	FLOAT& Y() { return PositionWS.y; }
	FLOAT& Z() { return PositionWS.z; }
	FLOAT& Ambient() { return ambient; }
	FLOAT& DiffuseIntensity() { return diffuseIntensity; }
	FLOAT& SpecularIntensity() { return specularIntensity; }
	virtual FLOAT& TargetX() { return Nothing; }
	virtual FLOAT& TargetY() { return Nothing; }
	virtual FLOAT& TargetZ() { return Nothing; }
	virtual XMFLOAT4& Target() { return Nothing4; }
	virtual std::string LightType() { return "Unknown"; }
	virtual FLOAT& Spread() { return Nothing; }
	std::string Status() { return (status) ? "ON" : "OFF"; }
	void SetColor(std::string cn)
	{
		colorName = cn;
		color = FindColor(cn);
	}
	void SetColor(XMFLOAT4 c)
	{
		colorName = "";
		color = c;
	}

};

class PointLight : public Light
{
public:
	PointLight(XMFLOAT4 PositionWS = { 0.0f, 0.0f, 0.0f, 1.0f },
		XMFLOAT4 PositionVS = { 0.0f, 0.0f, 0.0f, 1.0f },
		std::string Color = "WHITE",
		float Ambient = 0.01f,
		float DiffuseIntensity = 1.00f,
		float SpecularIntensity = 1.00f,
		float ConstantAttenuation = 1.0f,
		float LinearAttenuation = 0.0f,
		float QuadraticAttenuation = 0.0f) :
		Light(PositionWS, PositionVS, Color, Ambient, DiffuseIntensity, SpecularIntensity, ConstantAttenuation, LinearAttenuation, QuadraticAttenuation)
	{}
	virtual std::string LightType() { return "PointLight"; }

};

class SpotLight : public Light
{
public:
	XMFLOAT4 DirectionWS = { 0.0f, 0.0f, 0.0f, 1.0f };  // Light direction in world space.
	//----------------------------------- (16 byte boundary)
	XMFLOAT4 DirectionVS = { 0.0f, 0.0f, 0.0f, 1.0f };  // Light direction in view space.
	//----------------------------------- (16 byte boundary)
	float SpotAngle;
	SpotLight(XMFLOAT4 PositionWS = { 0.0f, 0.0f, 0.0f, 1.0f },
		XMFLOAT4 PositionVS = { 0.0f, 0.0f, 0.0f, 1.0f },
		std::string Color = "WHITE",
		float Ambient = 0.01f,
		float DiffuseIntensity = 1.00f,
		float SpecularIntensity = 1.00f,
		float _SpotAngle = XM_PIDIV2,
		float ConstantAttenuation = 1.0f,
		float LinearAttenuation = 0.0f,
		float QuadraticAttenuation = 0.0f) :
		Light(PositionWS, PositionVS, Color, Ambient, DiffuseIntensity, SpecularIntensity, ConstantAttenuation, LinearAttenuation, QuadraticAttenuation),
		SpotAngle(_SpotAngle)
	{}
	virtual std::string LightType() { return "SpotLight"; }
	virtual FLOAT& Spread() { return SpotAngle; }
	virtual XMFLOAT4& Target() { return DirectionWS; }
	virtual FLOAT& TargetX() { return DirectionWS.x; }
	virtual FLOAT& TargetY() { return DirectionWS.y; }
	virtual FLOAT& TargetZ() { return DirectionWS.z; }

};

class DirectionalLight : public SpotLight
{
public:
	DirectionalLight(XMFLOAT4 _DirectionWS = { 0.0f, 0.0f, 1.0f, 0.0f },
		XMFLOAT4 _DirectionVS = { 0.0f, 0.0f, 1.0f, 0.0f },
		std::string Color = "WHITE",
		FLOAT ambient = 0.01f, 
		FLOAT diffuseIntensity = 1.00f,
		FLOAT specularIntensity = 1.00f) :
		SpotLight({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, Color, ambient, diffuseIntensity, specularIntensity, 1, 0, 0)
	{
		DirectionWS = _DirectionWS;
		DirectionVS = _DirectionVS;
	}
	virtual std::string LightType() { return "DirectionalLight"; }
};

struct GPUPointLight
{
	DirectX::XMFLOAT4 PositionWS;
	DirectX::XMFLOAT4 PositionVS;
	DirectX::XMFLOAT4 Color;

	float Ambient;
	float DiffuseIntensity;
	float SpecularIntensity;
	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;
	DirectX::XMFLOAT2 Padding;
};
static_assert(sizeof(GPUPointLight) == 80);

struct GPUSpotLight
{
	DirectX::XMFLOAT4 PositionWS;
	DirectX::XMFLOAT4 PositionVS;

	DirectX::XMFLOAT4 DirectionWS;
	DirectX::XMFLOAT4 DirectionVS;

	DirectX::XMFLOAT4 Color;

	float Ambient;
	float DiffuseIntensity;
	float SpecularIntensity;
	float SpotAngle;
	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;
	float Padding;
};
static_assert(sizeof(GPUSpotLight) == 112);

struct GPUDirectionalLight
{
	DirectX::XMFLOAT4 DirectionWS;
	DirectX::XMFLOAT4 DirectionVS;

	DirectX::XMFLOAT4 Color;

	float Ambient;
	float DiffuseIntensity;
	float SpecularIntensity;
	float Padding;
};
static_assert(sizeof(GPUDirectionalLight) == 64);