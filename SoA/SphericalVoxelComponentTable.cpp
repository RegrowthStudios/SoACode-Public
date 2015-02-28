#include "stdafx.h"
#include "SphericalVoxelComponentTable.h"

#include "ChunkGrid.h"
#include "ChunkIOManager.h"
#include "ChunkListManager.h"
#include "ChunkMemoryManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"

void SphericalVoxelComponentTable::disposeComponent(vcore::ComponentID cID, vcore::EntityID eID) {
    SphericalVoxelComponent& cmp = _components[cID].second;
    cmp.threadPool->destroy();
    delete cmp.threadPool;
    delete cmp.physicsEngine;
    delete cmp.chunkListManager;
    delete cmp.chunkGrid;
    delete cmp.chunkMemoryManager;
    delete cmp.chunkIo;
    delete cmp.particleEngine;
    cmp = _components[0].second;
}
