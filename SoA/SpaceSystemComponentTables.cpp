#include "stdafx.h"
#include "SpaceSystemComponentTables.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/ShaderManager.h>

#include "ChunkAllocator.h"
#include "ChunkIOManager.h"
#include "ChunkListManager.h"
#include "NChunkGrid.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "PlanetData.h"
#include "SphericalTerrainCpuGenerator.h"
#include "SphericalTerrainGpuGenerator.h"
#include "TerrainPatchMeshManager.h"
#include "TerrainRpcDispatcher.h"

void SphericalVoxelComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) {
    SphericalVoxelComponent& cmp = _components[cID].second;
    cmp.threadPool->destroy();
    delete cmp.threadPool;
    delete cmp.chunkListManager;
    delete cmp.chunkAllocator;
    delete cmp.chunkIo;
    delete[] cmp.chunkGrids;
    cmp = _components[0].second;
}

void SphericalTerrainComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) {
    SphericalTerrainComponent& cmp = _components[cID].second;
    if (cmp.planetGenData) {
        delete cmp.meshManager;
        delete cmp.gpuGenerator;
        delete cmp.cpuGenerator;
        delete cmp.rpcDispatcher;
    }
    delete cmp.sphericalTerrainData;
    //delete[] cmp.patches;
}

void OrbitComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) {
    OrbitComponent& cmp = _components[cID].second;
    if (cmp.vbo) {
        vg::GpuMemory::freeBuffer(cmp.vbo);
    }
}
