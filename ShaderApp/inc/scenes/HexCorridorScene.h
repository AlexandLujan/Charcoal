#pragma once

#include "SceneGeometry.h"

struct HexCorridorGeometry
{
	SceneGeometry Regular;
	SceneGeometry Emissive;

	std::vector<DirectX::XMFLOAT4> EmissiveLightPositions;
};

HexCorridorGeometry BuildHexCorridorScene();