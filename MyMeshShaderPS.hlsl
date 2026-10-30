//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct Constants
{
    float4x4 World;
    float4x4 WorldView;
    float4x4 WorldViewProj;
    uint DrawMeshlets;
};

struct MaterialProperty
{
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
    float4 Ambient;
    float4 Reflectance;
    float Opacity; // If Opacity < 1, then the material is transparent.
    float SpecularPower;
    float IndexOfRefraction; // For transparent materials, IOR > 0.
    float BumpIntensity; // When using bump textures (height maps) we need
    uint HasAmbientTexture;
    uint HasEmissiveTexture;
    uint HasDiffuseTexture;
    uint HasSpecularTexture;
    uint HasSpecularPowerTexture;
    uint HasNormalTexture;
    uint HasBumpTexture;
    uint HasOpacityTexture;
};

struct VertexOut
{
    float4 PositionHS : SV_Position; // 12
    float3 PositionVS : POSITION0; // 16
    float3 Normal : NORMAL0; //12
    float2 Texture : TEXCOORD0; // 8
    uint Color : COLOR0; // 4 - will be index into material array
    uint2 Pad0 : PAD0; // 8
};

ConstantBuffer<Constants> Globals : register(b0);
StructuredBuffer<MaterialProperty> materialProperty : register(t4);

float4 main(VertexOut input) : SV_TARGET
{
    float3 surfaceColor = materialProperty[input.Color].Emissive.rgb;
    float3 ambient = materialProperty[input.Color].Ambient.rgb * 0.2; // will be replace with program value
    float3 diffuseColor = materialProperty[input.Color].Diffuse.rgb;
    float3 lightDir = -normalize(float3(0, 1, 1));
    float shininess = materialProperty[input.Color].Reflectance.x;
    if (!Globals.DrawMeshlets)
    {
        diffuseColor = float3(0.8, 0.8, 0.8);
        shininess = 64.0;
    }

    float3 normal = normalize(input.Normal);

    // Do some fancy Blinn-Phong shading!
    float cosAngle = saturate(dot(normal, lightDir));
    float3 viewDir = -normalize(input.PositionVS);
    float3 halfAngle = normalize(lightDir + viewDir);

    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, shininess);
    float3 amb = ambient + surfaceColor; //    ((cosAngle + blinnTerm + 0.1) * surfaceColor);
    float4 finalColor = float4(amb,1);
    return finalColor;
}