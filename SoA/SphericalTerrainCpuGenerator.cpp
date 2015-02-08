#include "stdafx.h"
#include "SphericalTerrainCpuGenerator.h"



SphericalTerrainCpuGenerator::SphericalTerrainCpuGenerator(SphericalTerrainMeshManager* meshManager,
                                                           PlanetGenData* planetGenData) :
    m_mesher(meshManager, planetGenData) {
    // Empty
}


SphericalTerrainCpuGenerator::~SphericalTerrainCpuGenerator() {
    // Empty
}

void SphericalTerrainCpuGenerator::generateTerrainPatch(OUT SphericalTerrainMesh* mesh, const f32v3& startPos, WorldCubeFace cubeFace, float width) {

}
