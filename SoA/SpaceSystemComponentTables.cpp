#include "stdafx.h"
#include "SpaceSystemComponentTables.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/ShaderManager.h>

#include "ChunkAllocator.h"
#include "ChunkIOManager.h"
#include "FarTerrainPatch.h"
#include "ChunkGrid.h"
#include "PlanetGenData.h"
#include "SphericalHeightmapGenerator.h"
#include "TerrainPatch.h"
#include "TerrainPatchMeshManager.h"

void SphericalVoxelComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID VORB_UNUSED) {
    SphericalVoxelComponent& cmp = _components[cID].second;
    // Let the threadpool finish
    while (cmp.threadPool->getTasksSizeApprox() > 0);
    delete cmp.chunkIo;
    delete[] cmp.chunkGrids;
    cmp = _components[0].second;
}

void SphericalTerrainComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID VORB_UNUSED) {
    SphericalTerrainComponent& cmp = _components[cID].second;
    if (cmp.patches) {
        delete[] cmp.patches;
        cmp.patches = nullptr;
    }
    if (cmp.planetGenData) {
        delete cmp.meshManager;
        delete cmp.cpuGenerator;
    }
    // TODO(Ben): Memory leak
    delete cmp.sphericalTerrainData;
}

void FarTerrainComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID VORB_UNUSED) {
    FarTerrainComponent& cmp = _components[cID].second;  
    if (cmp.patches) {
        delete[] cmp.patches;
        cmp.patches = nullptr;
    }
}

void OrbitComponentTable::disposeComponent(vecs::ComponentID cID, vecs::EntityID eID VORB_UNUSED) {
    OrbitComponent& cmp = _components[cID].second;
    if (cmp.vbo) {
        vg::GpuMemory::freeBuffer(cmp.vbo);
    }
}
