#include "stdafx.h"
#include "GameSystem.h"

GameSystem::GameSystem() : vecs::ECS() {
    addComponentTable("AABBCollidable", &aabbCollidable);
    addComponentTable("FreeMove", &freeMoveInput);
    addComponentTable("Physics", &physics);
    addComponentTable("SpacePosition", &spacePosition);
    addComponentTable("VoxelPosition", &voxelPosition);
    addComponentTable("Frustum", &frustum);
    addComponentTable("Head", &head);
    addComponentTable("ChunkSphere", &chunkSphere);
}