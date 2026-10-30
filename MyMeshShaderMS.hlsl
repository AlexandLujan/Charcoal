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

#define ROOT_SIG "CBV(b0), \
                  RootConstants(b1, num32bitconstants=2), \
                  SRV(t0), \
                  SRV(t1), \
                  SRV(t2), \
                  SRV(t3), \
                  SRV(t4), \
                  SRV(t5)"

struct Constants
{
    float4x4 World;
    float4x4 WorldView;
    float4x4 WorldViewProj;
    uint DrawMeshlets;
};

struct MeshInfo
{
    uint IndexBytes;
    uint MeshletOffset;
};

struct Vertex
{
    float3 Position; // 12
    float3 Normal; // 12
    float2 Texture; // 8
    uint Color; // 4 - will be index into material array
    uint Pad0; // 4 
};

struct VertexOut
{
    float4 PositionHS : SV_Position; // 16
    float3 PositionVS : POSITION0; // 16
    float3 Normal : NORMAL0; //12
    float2 Texture : TEXCOORD0; // 8
    uint Color : COLOR0; // 4 - will be index into material array
    uint2 Pad0 : PAD0; // 8
};

struct Meshlet
{
    uint VertCount;
    uint VertOffset;
    uint PrimCount;
    uint PrimOffset;
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

struct CullingData
{
    float3 BoundingSphereCenter;
    float BoundingSphereRadius;
    uint MeshletID;
};

ConstantBuffer<Constants> Globals : register(b0);
ConstantBuffer<MeshInfo> meshInfo : register(b1);
StructuredBuffer<Vertex> Vertices : register(t0);
StructuredBuffer<Meshlet> Meshlets : register(t1);
StructuredBuffer<uint> UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);
StructuredBuffer<MaterialProperty> materialProperty : register(t4);
StructuredBuffer<CullingData> cullingData : register(t5);

/////
// Data Loaders

uint3 UnpackPrimitive(uint primitive)
{
    // Unpacks a 10 bits per index triangle from a 32-bit uint.
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

uint3 GetPrimitive(Meshlet m, uint index)
{
    return UnpackPrimitive(PrimitiveIndices[m.PrimOffset + index]);
}

uint GetVertexIndex(Meshlet m, uint localIndex)
{
    return UniqueVertexIndices[m.VertOffset + localIndex];
}

VertexOut GetVertexAttributes(uint meshletIndex, uint vertexIndex)
{
    Vertex v = Vertices[vertexIndex];

    VertexOut vout;
    vout.PositionVS = mul(float4(v.Position, 1.0f), Globals.WorldView).xyz;
    vout.PositionHS = mul(float4(v.Position, 1.0f), Globals.WorldViewProj);
    vout.Normal = mul(float4(v.Normal, 0), Globals.World).xyz;
    vout.Texture = mul(float4(v.Texture,0,0), Globals.World).xy;
    vout.Color = v.Color;
    return vout;
}

struct PrimitiveOut
{
    uint prim : SV_PrimitiveID;
};

[RootSignature(ROOT_SIG)]
[NumThreads(64, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    out vertices VertexOut verts[256],
    out indices uint3 tris[256]
)
{    
//    Meshlet m = Meshlets[MeshInfo.MeshletOffset + gid];
    Meshlet m = Meshlets[gid];

    SetMeshOutputCounts(m.VertCount, m.PrimCount);

    if (gtid < m.PrimCount)
    {
        uint3 tri = GetPrimitive(m, gtid);
        tris[gtid] = tri;
        uint i0 = GetVertexIndex(m, tri.x);
        uint i1 = GetVertexIndex(m, tri.y);
        uint i2 = GetVertexIndex(m, tri.z);
        verts[tri.x] = GetVertexAttributes(gid, i0);
        verts[tri.y] = GetVertexAttributes(gid, i1);
        verts[tri.z] = GetVertexAttributes(gid, i2);
    }
    //if (gtid < m.PrimCount)
    //{
    //    tris[gtid] = GetPrimitive(m, gtid);
    //}

    //if (gtid < m.VertCount)
    //{
    //    uint vertexIndex = GetVertexIndex(m, gtid);
    //    verts[gtid] = GetVertexAttributes(gid, vertexIndex);
    //}

}
