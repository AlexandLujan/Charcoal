#include "pch.h"
#include "scenes/HexCorridorScene.h"

#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

using namespace DirectX;

namespace
{
    void AddWallQuad(
        SceneGeometry& geometry,
        float x0,
        float z0,
        float x1,
        float z1,
        float bottomY,
        float topY,
        const XMFLOAT3& normal,
        const XMFLOAT3& tangent,
        const XMFLOAT3& bitangent)
    {
        const uint32_t baseVertex =
            static_cast<uint32_t>(geometry.Vertices.size());

        geometry.Vertices.push_back(
            {
                { x0, topY, z0 },
                normal,
                tangent,
                bitangent,
                { 0.0f, 0.0f, 0.0f }
            });

        geometry.Vertices.push_back(
            {
                { x0, bottomY, z0 },
                normal,
                tangent,
                bitangent,
                { 0.0f, 1.0f, 0.0f }
            });

        geometry.Vertices.push_back(
            {
                { x1, topY, z1 },
                normal,
                tangent,
                bitangent,
                { 1.0f, 0.0f, 0.0f }
            });

        geometry.Vertices.push_back(
            {
                { x1, bottomY, z1 },
                normal,
                tangent,
                bitangent,
                { 1.0f, 1.0f, 0.0f }
            });

        geometry.Indices.push_back(baseVertex + 0);
        geometry.Indices.push_back(baseVertex + 1);
        geometry.Indices.push_back(baseVertex + 2);

        geometry.Indices.push_back(baseVertex + 2);
        geometry.Indices.push_back(baseVertex + 1);
        geometry.Indices.push_back(baseVertex + 3);
    }

    void AddHexPrism(
        SceneGeometry& regularGeometry,
        SceneGeometry& emissiveGeometry,
        float centerX,
        float centerZ,
        float radius,
        float bottomY,
        float topY,
        bool emissiveWalls)
    {

        const XMFLOAT3 topNormal =
        {
            0.0f,
            1.0f,
            0.0f
        };

        const XMFLOAT3 topTangent =
        {
            1.0f,
            0.0f,
            0.0f
        };

        const XMFLOAT3 topBitangent =
        {
            0.0f,
            0.0f,
            1.0f
        };

        const uint32_t baseVertex =
            static_cast<uint32_t>(regularGeometry.Vertices.size());

        //
        // TOP FACE VERTICES
        // Separate from wall vertices so the top can stay dark.
        //

        // Top center
        regularGeometry.Vertices.push_back(
            {
                { centerX, topY, centerZ }, // Position
                topNormal,                  // Normal
                topTangent,                 // Tangent
                topBitangent,               // Bitangent
                { 0.5f, 0.5f, 0.0f }        // TexCoord
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

            float u =
                0.5f + ((x - centerX) / (2.0f * radius));

            float v =
                0.5f + ((z - centerZ) / (2.0f * radius));

            regularGeometry.Vertices.push_back(
                {
                    { x, topY, z },
                    topNormal,
                    topTangent,
                    topBitangent,
                    { u, v, 0.0f }
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

            regularGeometry.Indices.push_back(baseVertex + 0);
            regularGeometry.Indices.push_back(baseVertex + 1 + i);
            regularGeometry.Indices.push_back(baseVertex + 1 + next);
        }

        //
        // SIDE WALLS
        //
        // Each wall gets its own four vertices.
        // This lets every wall have one flat normal instead of
        // sharing corner vertices between adjacent faces.
        //

        float wallHeight = topY - bottomY;
        float centerY = (bottomY + topY) * 0.5f;

        // total strip thickness = 12% of wall height
        float emissiveThickness = wallHeight * 0.25f;
        float emissiveBottomY = centerY - emissiveThickness * 0.5f;
        float emissiveTopY = centerY + emissiveThickness * 0.5f;

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

            XMFLOAT3 wallTangent =
            {
                edgeX,
                0.0f,
                edgeZ
            };

            float tangentLength =
                std::sqrt(
                    wallTangent.x * wallTangent.x +
                    wallTangent.z * wallTangent.z
                );

            if (tangentLength > 0.0f)
            {
                wallTangent.x /= tangentLength;
                wallTangent.z /= tangentLength;
            }

            const XMFLOAT3 wallBitangent =
            {
                0.0f,
                1.0f,
                0.0f
            };

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

            if (emissiveWalls)
            {
                // Upper regular wall section.
                AddWallQuad(
                    regularGeometry,
                    x0,
                    z0,
                    x1,
                    z1,
                    emissiveTopY,
                    topY,
                    wallNormal,
                    wallTangent,
                    wallBitangent
                );

                // Thin emissive strip.
                AddWallQuad(
                    emissiveGeometry,
                    x0,
                    z0,
                    x1,
                    z1,
                    emissiveBottomY,
                    emissiveTopY,
                    wallNormal,
                    wallTangent,
                    wallBitangent
                );

                // Lower regular wall section.
                AddWallQuad(
                    regularGeometry,
                    x0,
                    z0,
                    x1,
                    z1,
                    bottomY,
                    emissiveBottomY,
                    wallNormal,
                    wallTangent,
                    wallBitangent
                );
            }
            else
            {
                // Outer shell: one solid non-emissive wall.
                AddWallQuad(
                    regularGeometry,
                    x0,
                    z0,
                    x1,
                    z1,
                    bottomY,
                    topY,
                    wallNormal,
                    wallTangent,
                    wallBitangent
                );
            }
        }
    }
}

HexCorridorGeometry BuildHexCorridorScene()
{
    HexCorridorGeometry scene;

    constexpr float radius = 1.0f;
    constexpr float bottomY = -0.55f;

    constexpr int columns = 80;
    constexpr int rows = 60;

    constexpr float fieldRadius = 42.0f;
    constexpr float shellThickness = 9.0f;

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

            const float distanceSquared = x * x + z * z;

            const float fieldRadiusSquared = fieldRadius * fieldRadius;

            if (distanceSquared > fieldRadiusSquared)
                continue;

            const float shellStartRadius = fieldRadius - shellThickness;
            const float shellStartRadiusSquared = shellStartRadius * shellStartRadius;
            const bool isOuterShell = distanceSquared >= shellStartRadiusSquared;

            //
            // CCreate radial ripple heigh variation across the field.
            //
            const float distanceFromCenter =
                std::sqrt(distanceSquared);

            constexpr float baseHeight = 1.5f;
            constexpr float rippleAmplitude = 1.25f;
            constexpr float rippleFrequency = 0.45f;

            float topY =
                baseHeight +
                std::sin(distanceFromCenter * rippleFrequency)
                * rippleAmplitude;

            const float variation =
                static_cast<float>(
                    ((row * 37 + column * 73) % 1000)
                    ) / 1000.0f;

            topY += (variation - 0.5f) * 0.5f;

            if (isOuterShell)
            {
                constexpr float shellRingWidth = 1.5f;
                constexpr float shellBaseHeight = 4.0f;
                constexpr float shellGrowth = 1.35f;

                const float depthIntoShell =
                    distanceFromCenter - shellStartRadius;

                const int shellRing =
                    static_cast<int>(
                        depthIntoShell / shellRingWidth
                        );

                const float shellHeight =
                    shellBaseHeight *
                    std::pow(
                        shellGrowth,
                        static_cast<float>(shellRing)
                    );

                const float shellVariation =
                    static_cast<float>(
                        ((row * 53 + column * 97) % 1000)
                        ) / 1000.0f;

                topY =
                    shellHeight +
                    (shellVariation - 0.5f) * 0.5f;
            }

            AddHexPrism(
                scene.Regular,
                scene.Emissive,
                x,
                z,
                radius,
                bottomY,
                topY,
                !isOuterShell);

            if (!isOuterShell)
            {
                constexpr int lightSpacing = 20;

                if ((row % lightSpacing == 0) &&
                    (column % lightSpacing == 0))
                {
                    scene.EmissiveLightPositions.push_back(
                        {
                            x,
                            topY + 0.5f,
                            z,
                            1.0f
                        });
                }
            }
        }
    }

    return scene;
}