#pragma once

#include "SceneGeometry.h"

struct HexCorridorGeometry
{
	SceneGeometry RegularTop;
	SceneGeometry RegularSides;
	SceneGeometry Emissive;

	std::vector<DirectX::XMFLOAT4> EmissiveLightPositions;
};

HexCorridorGeometry BuildHexCorridorScene();