
//#include "Shared.h"

struct Constants
{
    float4x4 World;
    float4x4 WorldView;
    float4x4 WorldViewProj;
    float4 camera_position;
    float4 camera_direction;
    float ambient;
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

struct MaterialBuffer
{
    float4 Emissive;
    float4 Specular;
    float Reflectance;
    float SpecularPower;
    float Transparency;
    float IndexOfRefraction;
    float Intensity;
};

struct CullingData
{
    float3 BoundingSphereCenter;
    float BoundingSphereRadius;
    uint MeshletID;
};

struct InstanceData
{
    uint flags; // e.g., for culling options
    uint meshletIndex; // e.g., for culling options
    uint meshletCount; // e.g., for culling options
    float scale;
    float4x4 worldMatrix;
};

struct ThePayload
{
    uint InstanceID;
    uint BaseMeshletID;
    float4x4 world;
};

struct Light
{
    float4 positionW;
    float4 positionV;
    float4 directionW;
    float4 directionV;
    float4 color;
    float intensity;
    float range;
    uint status;
    uint pad0;
};
