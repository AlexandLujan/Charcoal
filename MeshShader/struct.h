#pragma once
#include <cstdint>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <windows.h>

using namespace DirectX;
using namespace Microsoft::WRL;

struct Attribute
{
	enum EType : uint32_t
	{
		Position,
		Normal,
		TexCoord,
		Tangent,
		Bitangent,
		Color,
		Uses,
		Count
	};

	EType    Type;
	uint32_t Offset;
};

struct Subset
{
	uint32_t Offset;
	uint32_t Count;
};

struct MeshInfo
{
	uint32_t IndexSize;
	uint32_t MeshletCount;

//	uint32_t LastMeshletVertCount;
//	uint32_t LastMeshletPrimCount;
};

struct meshletInfo
{
	std::string name;
	UINT meshletStart;
	UINT meshletCount;
};

struct Meshlet
{
	uint32_t VertCount;
	uint32_t VertOffset;
	uint32_t PrimCount;
	uint32_t PrimOffset;
};

struct PackedTriangle
{
	uint32_t i0 : 10;
	uint32_t i1 : 10;
	uint32_t i2 : 10;
};

struct CullData
{
//	DirectX::XMFLOAT4 BoundingSphere; // xyz = center, w = radius
//	uint8_t           NormalCone[4];  // xyz = axis, w = -cos(a + 90)
//	float             ApexOffset;     // apex = center - axis * offset
	DirectX::XMFLOAT3		BoundingSphereCenter;
	float					BoundingSphereRadius;
	uint32_t				MeshletID;
};

typedef struct VertexPositionNormalTextureMaterial VertexPositionNormalTextureMaterial;
struct VertexPositionNormalTextureMaterial
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 Texture;
	UINT	 Material;
	UINT	 uses;
};

struct InstanceData
{
	UINT flags; // e.g., for culling options
	UINT meshletIndex; // e.g., for culling options
	UINT meshletCount; // e.g., for culling options
	FLOAT scale;
	XMMATRIX worldMatrix;
};

struct MeshStatistics
{
	UINT meshOffset;
	UINT meshCount;
	BOOL IsTransparent;
	XMFLOAT3 meshMin;
	XMFLOAT3 meshMax;
};

struct MaterialData
{
	XMFLOAT4 Emissive;
	XMFLOAT4 Specular;
	FLOAT Reflectance;
	FLOAT SpecularPower;
	FLOAT Transparency;
	FLOAT IndexOfRefraction;
	FLOAT Intensity;
};

#define GENERIC_LIGHT		2
#define POINT_LIGHT			4
#define SPOT_LIGHT			6
#define DIRECTIONAL_LIGHT	8

struct LightData
{
	XMFLOAT4 positionW;
	XMFLOAT4 positionV;
	XMFLOAT4 directionW;
	XMFLOAT4 directionV;
	XMFLOAT4 color;
	FLOAT	 intensity;
	FLOAT    range;
	UINT	 status;
	UINT	 pad0;
};
