#include "stdafx.h"
#include "TerrainRpcDispatcher.h"

#include "SphericalTerrainGpuGenerator.h"

void TerrainGenDelegate::invoke(Sender sender, void* userData) {
    generator->generateTerrainPatch(this);
}

void TerrainGenDelegate::release() {
    inUse = false;
}


TerrainPatchMesh* TerrainRpcDispatcher::dispatchTerrainGen(const f32v3& startPos,
                                                               float width,
                                                               int lod,
                                                               WorldCubeFace cubeFace,
                                                               bool isSpherical) {
    TerrainPatchMesh* mesh = nullptr;
    // Check if there is a free generator
    if (!m_generators[counter].inUse) {
        auto& gen = m_generators[counter];
        // Mark the generator as in use
        gen.inUse = true;
        gen.rpc.data.f = &gen;
        mesh = new TerrainPatchMesh(cubeFace);
        // Set the data
        gen.startPos = startPos;
        gen.cubeFace = cubeFace;
        gen.mesh = mesh;
        gen.width = width;
        gen.isSpherical = isSpherical;
        // Invoke generator
        m_generator->invokePatchTerrainGen(&gen.rpc);
        // Go to next generator
        counter++;
        if (counter == NUM_GENERATORS) counter = 0;
    }
    return mesh;
}