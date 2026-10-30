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
#include <windows.h>
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
	FLOAT ConstantAttenuation;
	FLOAT LinearAttenuation;
	FLOAT QuadraticAttenuation;
	bool status = false;
	static FLOAT Nothing;
	static XMFLOAT4 Nothing4;
	//----------------------------------- (16 byte boundary)
	// Total:  
	//                           16 * 4 = 64 bytes

	Light(XMFLOAT4 _PositionWS, XMFLOAT4 _PositionVS, std::string _Color,
		float _Ambient, float _ConstantAttenuation, float _LinearAttenuation, float _QuadraticAttenuation) :
		PositionWS(_PositionWS),
		PositionVS(_PositionVS),
		colorName(_Color),
		ambient(_Ambient),
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
	FLOAT& Y() { return PositionWS.x; }
	FLOAT& Z() { return PositionWS.x; }
	FLOAT& Ambient() { return ambient; }
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
		float ConstantAttenuation = 1.0f,
		float LinearAttenuation = 0.0f,
		float QuadraticAttenuation = 0.0f) :
		Light(PositionWS, PositionVS, Color, Ambient, ConstantAttenuation, LinearAttenuation, QuadraticAttenuation)
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
		float _SpotAngle = XM_PIDIV2,
		float ConstantAttenuation = 1.0f,
		float LinearAttenuation = 0.0f,
		float QuadraticAttenuation = 0.0f) :
		Light(PositionWS, PositionVS, Color, Ambient, ConstantAttenuation, LinearAttenuation, QuadraticAttenuation),
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
					 FLOAT ambient = 0.01f) :
		SpotLight({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, Color, ambient, 1, 0, 0)
	{
		DirectionWS = _DirectionWS;
		DirectionVS = _DirectionVS;
	}
	virtual std::string LightType() { return "DirectionalLight"; }
};


#ifdef PIXELSHADER


// Constant Buffers defining material and light properties
cbuffer MaterialBuffer : register(b0)
{
	float4 DiffuseColor;
	float3 SpecularColor;
	float SpecularPower;
};

cbuffer LightBuffer : register(b1)
{
	float3 LightPos;
	float4 LightColor;
	float3 EyePos;
};

// Data passed from the Vertex Shader
struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 worldPos : TEXCOORD0;
	float3 normal : NORMAL;
};

float4 main(PixelInputType input) : SV_TARGET
{
	// 1. Ambient Light
	float4 ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);

	// 2. Diffuse Light (N dot L)
	float3 lightDir = normalize(LightPos - input.worldPos);
	float3 normal = normalize(input.normal);
	float diffuseFactor = max(dot(normal, lightDir), 0.0f);
	float4 diffuse = diffuseFactor * LightColor * DiffuseColor;

	// 3. Specular Light (Blinn-Phong using Halfway Vector)
	float3 viewDir = normalize(EyePos - input.worldPos);
	float3 halfwayDir = normalize(lightDir + viewDir);
	float specFactor = pow(max(dot(normal, halfwayDir), 0.0f), SpecularPower);
	float4 specular = specFactor * LightColor * float4(SpecularColor, 1.0f);

	// Final output color
	return saturate(ambient + diffuse + specular);
}



#endif