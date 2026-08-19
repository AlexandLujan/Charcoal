#if 0
#include "pch.h"
#include "ShaderAppDlg.h"
#include <Scene.h>
#include <SceneNode.h>
#include <Mesh.h>
#include <Material.h>
#include <IndexBuffer.h>
#include <VertexBuffer.h>
#include <CommandList.h>
#include "camry.h"
XMFLOAT3 Normal(XMFLOAT3 P1, XMFLOAT3 P2, XMFLOAT3 P3) // return the normal
{
    float aX = P2.x - P1.x;
    float aY = P2.y - P1.y;
    float aZ = P2.z - P1.z;
    float bX = P3.x - P1.x;
    float bY = P3.y - P1.y;
    float bZ = P3.z - P1.z;
    float x_ = (aY * bZ) - (aZ * bY);
    float y_ = (aX * bZ) - (aZ * bX);
    //		y_ = (aZ * bX) - (aX * bZ);
    float z_ = (aX * bY) - (aY * bX);
    float n = sqrtf((x_ * x_) + (y_ * y_) + (z_ * z_));
    x_ /= n;
    y_ /= n;
    z_ /= n;
	return XMFLOAT3(x_, y_, z_);
}
using namespace dx12lib;
using VertexCollection = std::vector<dx12lib::VertexPositionNormalTangentBitangentTexture>; // VertexPosition
using IndexCollection = std::vector<uint32_t>;

std::shared_ptr<Scene> BuildTriangleStructure(XMFLOAT3 g_Vertices[], UINT g_Indicies[], map<string, structure_section> SectionMap,list<string>sectionList)
{
    auto scene = std::make_shared<Scene>();
    XMFLOAT3 t[4] = { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 } };
    for(string name : sectionList)
    {
        VertexCollection vertices;
        IndexCollection  indices;

        structure_section section = SectionMap[name];
        for (int i = 0; i < section.index_count; i+=3)
        {
			UINT i0 = g_Indicies[i + section.base_index];
			UINT i1 = g_Indicies[i + section.base_index + 1];
			UINT i2 = g_Indicies[i + section.base_index + 2];
			// Process triangle (v0, v1, v2)
			XMFLOAT3 v0 = g_Vertices[i0];
			XMFLOAT3 v1 = g_Vertices[i1];
			XMFLOAT3 v2 = g_Vertices[i2];
			XMFLOAT3 N = Normal(v0, v1, v2);
            vertices.emplace_back(v0, N, t[0]);
            vertices.emplace_back(v1, N, t[1]);
            vertices.emplace_back(v2, N, t[2]);
            // First triangle.
            indices.emplace_back(i + 0);
            indices.emplace_back(i + 1);
            indices.emplace_back(i + 2);
        }
    
//		auto node = CreateSceneMesh(vertices, indices, section.color);
//        scene->SetRootNode(node);
    }
    return scene;
}

#endif

