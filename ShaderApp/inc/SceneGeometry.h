#pragma once

#include <DirectXMath.h>
#include <cstdint>
#include <vector>

struct SceneVertex
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT3 Tangent;
    DirectX::XMFLOAT3 Bitangent;
    DirectX::XMFLOAT3 TexCoord;
};

struct SceneGeometry
{
    std::vector<SceneVertex> Vertices;
    std::vector<uint32_t> Indices;
};