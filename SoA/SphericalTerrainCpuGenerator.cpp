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
