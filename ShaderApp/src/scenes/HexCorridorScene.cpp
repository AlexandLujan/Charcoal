#include "pch.h"
#include "scenes/HexCorridorScene.h"

#include <DirectXMath.h>
#include <cmath>

using namespace DirectX;

namespace
{
    void AddHexPrism(
        SceneGeometry& scene,
        float centerX,
        float centerZ,
        float radius,
        float bottomY,
        float topY)
    {
        // Dark top face.
        const XMFLOAT3 topColor =
        {
            0.05f,
            0.05f,
            0.07f
        };

        // Bright "glow" wall colors.
        const XMFLOAT3 wallTopColor =
        {
            1.00f,
            0.18f,
            0.05f
        };

        const XMFLOAT3 wallBottomColor =
        {
            0.35f,
            0.02f,
            0.00f
        };

        const XMFLOAT3 topNormal =
        {
            0.0f,
            1.0f,
            0.0f
        };

        const uint32_t baseVertex =
            static_cast<uint32_t>(scene.Vertices.size());

        //
        // TOP FACE VERTICES
        // Separate from wall vertices so the top can stay dark.
        //

        // Top center
        scene.Vertices.push_back(
            {
                { centerX, topY, centerZ },
                topNormal,
                topColor
            });

        // Top ring for top face (6 verts)
        for (int i = 0; i < 6; ++i)
        {
            float angle =
                XMConvertToRadians(30.0f + (60.0f * i));

            float x =
                centerX + radius * std::cos(angle);

            float z =
                centerZ + radius * std::sin(angle);

            scene.Vertices.push_back(
                {
                    { x, topY, z },
                    topNormal,
                    topColor
                });
        }

        //
        // TOP FACE INDICES
        // baseVertex + 0 = center
        // baseVertex + 1..6 = ring
        //
        for (uint32_t i = 0; i < 6; ++i)
        {
            uint32_t next = (i + 1) % 6;

            scene.Indices.push_back(baseVertex + 0);
            scene.Indices.push_back(baseVertex + 1 + i);
            scene.Indices.push_back(baseVertex + 1 + next);
        }

        //
        // SIDE WALLS
        //
        // Each wall gets its own four vertices.
        // This lets every wall have one flat normal instead of
        // sharing corner vertices between adjacent faces.
        //

        for (uint32_t i = 0; i < 6; ++i)
        {
            uint32_t next = (i + 1) % 6;

            float angle0 =
                XMConvertToRadians(30.0f + (60.0f * i));

            float angle1 =
                XMConvertToRadians(30.0f + (60.0f * next));

            float x0 =
                centerX + radius * std::cos(angle0);

            float z0 =
                centerZ + radius * std::sin(angle0);

            float x1 =
                centerX + radius * std::cos(angle1);

            float z1 =
                centerZ + radius * std::sin(angle1);

            //
            // Calculate the outward-facing wall normal.
            //

            float edgeX = x1 - x0;
            float edgeZ = z1 - z0;

            XMFLOAT3 wallNormal =
            {
                edgeZ,
                0.0f,
                -edgeX
            };

            float normalLength =
                std::sqrt(
                    wallNormal.x * wallNormal.x +
                    wallNormal.z * wallNormal.z
                );

            if (normalLength > 0.0f)
            {
                wallNormal.x /= normalLength;
                wallNormal.z /= normalLength;
            }

            const uint32_t wallBase =
                static_cast<uint32_t>(scene.Vertices.size());

            //
            // Four unique vertices for this wall.
            //

            // Top current.
            scene.Vertices.push_back(
                {
                    { x0, topY, z0 },
                    wallNormal,
                    wallTopColor
                });

            // Bottom current.
            scene.Vertices.push_back(
                {
                    { x0, bottomY, z0 },
                    wallNormal,
                    wallBottomColor
                });

            // Top next.
            scene.Vertices.push_back(
                {
                    { x1, topY, z1 },
                    wallNormal,
                    wallTopColor
                });

            // Bottom next.
            scene.Vertices.push_back(
                {
                    { x1, bottomY, z1 },
                    wallNormal,
                    wallBottomColor
                });

            //
            // Two triangles forming the wall quad.
            //

            scene.Indices.push_back(wallBase + 0);
            scene.Indices.push_back(wallBase + 1);
            scene.Indices.push_back(wallBase + 2);

            scene.Indices.push_back(wallBase + 2);
            scene.Indices.push_back(wallBase + 1);
            scene.Indices.push_back(wallBase + 3);
        }
    }
}

SceneGeometry BuildHexCorridorScene()
{
    SceneGeometry scene;

    constexpr float radius = 1.0f;
    constexpr float bottomY = -0.4f;

    constexpr int columns = 64;
    constexpr int rows = 48;

    const float horizontalSpacing =
        radius * 1.7320508f * 1.05f;

    const float verticalSpacing =
        radius * 1.5f * 1.05f;

    for (int row = 0; row < rows; ++row)
    {
        const float rowOffset =
            (row % 2 == 0)
            ? 0.0f
            : horizontalSpacing * 0.5f;

        for (int column = 0; column < columns; ++column)
        {
            float x =
                (column - (columns - 1) * 0.5f) *
                horizontalSpacing;

            x += rowOffset;

            float z =
                (row - (rows - 1) * 0.5f) *
                verticalSpacing;

            float topY = 0.4f;

            const int pattern =
                (row * 7 + column * 11) % 13;

            if (pattern == 0)
            {
                topY = 1.25f;
            }
            else if (pattern <= 2)
            {
                topY = 0.85f;
            }
            else if (pattern == 3)
            {
                topY = 0.15f;
            }

            AddHexPrism(
                scene,
                x,
                z,
                radius,
                bottomY,
                topY);
        }
    }

    return scene;
}