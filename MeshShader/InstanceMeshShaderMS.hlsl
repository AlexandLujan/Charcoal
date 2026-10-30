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
#include "MyShaderCommon.hlsli"

#define ROOT_SIG "CBV(b0), \
                  RootConstants(b1, num32bitconstants=2), \
                  SRV(t0), \
                  SRV(t1), \
                  SRV(t2), \
                  SRV(t3), \
                  SRV(t4), \
                  SRV(t5), \
                  SRV(t6), \
                  SRV(t7)"

ConstantBuffer<Constants> Globals : register(b0);
ConstantBuffer<MeshInfo> meshInfo : register(b1);
StructuredBuffer<Vertex> Vertices : register(t0);
StructuredBuffer<Meshlet> Meshlets : register(t1);
StructuredBuffer<uint> UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);
StructuredBuffer<MaterialBuffer> materialProperty : register(t4);
StructuredBuffer<CullingData> cullingData : register(t5);
StructuredBuffer<Light> Lights : register(t6);
StructuredBuffer<InstanceData> InstanceBuffer : register(t7);

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

VertexOut GetVertexAttributes(uint meshletIndex, uint vertexIndex, float4x4 world)
{
    Vertex v = Vertices[vertexIndex];

    VertexOut vout;
    float4x4 WorldView = mul(world, Globals.WorldView);
    float4x4 WorldViewProj = mul(world, Globals.WorldViewProj);
    //vout.PositionVS = mul(float4(v.Position, 1.0f), Globals.WorldView).xyz;
    //vout.PositionHS = mul(float4(v.Position, 1.0f), Globals.WorldViewProj);
    //vout.Normal = mul(float4(v.Normal, 0), Globals.World).xyz;
    //vout.Texture = mul(float4(v.Texture, 0, 0), Globals.World).xy;
    vout.PositionVS = mul(float4(v.Position, 1.0f), WorldView).xyz;
    vout.PositionHS = mul(float4(v.Position, 1.0f), WorldViewProj);
    vout.Normal = mul(float4(v.Normal, 0), world).xyz;
    vout.Texture = mul(float4(v.Texture, 0, 0), world).xy;
    vout.Color = v.Color;
    return vout;
}

struct PrimitiveOut
{
    uint prim : SV_PrimitiveID;
};

//groupshared ThePayload payload;

[RootSignature(ROOT_SIG)]
[NumThreads(64, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    in payload ThePayload p,
    out vertices VertexOut verts[256],
    out indices uint3 tris[256]
)
{    
//    Meshlet m = Meshlets[MeshInfo.MeshletOffset + gid];
    Meshlet m = Meshlets[p.BaseMeshletID+gid];

    SetMeshOutputCounts(m.VertCount, m.PrimCount);

    if (gtid < m.PrimCount)
    {
        uint3 tri = GetPrimitive(m, gtid);
        tris[gtid] = tri;
        uint i0 = GetVertexIndex(m, tri.x);
        uint i1 = GetVertexIndex(m, tri.y);
        uint i2 = GetVertexIndex(m, tri.z);
        verts[tri.x] = GetVertexAttributes(gid, i0, p.world);
        verts[tri.y] = GetVertexAttributes(gid, i1, p.world);
        verts[tri.z] = GetVertexAttributes(gid, i2, p.world);
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
