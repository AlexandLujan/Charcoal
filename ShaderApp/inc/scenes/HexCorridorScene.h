#pragma once

#include "SceneGeometry.h"

struct HexCorridorGeometry
{
	SceneGeometry Regular;
	SceneGeometry Emissive;
};

HexCorridorGeometry BuildHexCorridorScene();