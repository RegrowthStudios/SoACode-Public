#include "stdafx.h"
#include "GameSystem.h"

GameSystem::GameSystem() : vcore::ECS() {
    addComponentTable("AABBCollidable", &aabbCollidableCT);
    addComponentTable("FreeMoveFree", &freeMoveInputCT);
    addComponentTable("Physics", &physicsCT);
    addComponentTable("SpacePosition", &spacePositionCT);
    addComponentTable("VoxelPosition", &voxelPositionCT);
}