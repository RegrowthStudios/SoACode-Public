#include "stdafx.h"
#include "SpaceSystemComponentTables.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/ShaderManager.h>

#include "ChunkGrid.h"
#include "ChunkIOManager.h"
#include "ChunkListManager.h"
#include "ChunkMemoryManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "PlanetData.h"
#include "SphericalTerrainGpuGenerator.h"

void SphericalVoxelComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) {
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

void SphericalTerrainComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) {
    SphericalTerrainComponent& cmp = _components[cID].second;
    vg::GLProgram* genProgram = cmp.gpuGenerator->getPlanetGenData()->program;
    if (genProgram) {
        // TODO(Ben): Memory leak here
        genProgram->dispose();
    }
    delete cmp.meshManager;
    delete cmp.gpuGenerator;
    delete cmp.cpuGenerator;
    delete cmp.rpcDispatcher;
    delete cmp.sphericalTerrainData;
}

void OrbitComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) {
    OrbitComponent& cmp = _components[cID].second;
    if (cmp.vbo) {
        vg::GpuMemory::freeBuffer(cmp.vbo);
    }
}
