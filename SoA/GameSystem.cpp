#include "stdafx.h"
#include "GameSystem.h"

GameSystem::GameSystem() {
    addComponentTable("AABB Collidable", &aabbCollidableCT);
    addComponentTable("Free MoveFree", &freeMoveInputCT);
    addComponentTable("Physics", &physicsCT);
    addComponentTable("Space Position", &spacePositionCT);
    addComponentTable("Voxel Position", &voxelPositionCT);
}