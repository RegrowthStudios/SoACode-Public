#include "stdafx.h"
#include "SphericalVoxelComponentUpdater.h"

#include "ChunkManager.h"
#include "SpaceSystem.h"
#include "SphericalVoxelComponent.h"

void SphericalVoxelComponentUpdater::update(SpaceSystem* spaceSystem, const Camera* voxelCamera) {
    for (auto& it : spaceSystem->m_sphericalVoxelCT) {
        it.second.m_chunkManager->update(voxelCamera);
    }
}

void SphericalVoxelComponentUpdater::glUpdate(SpaceSystem* spaceSystem) {

}

void SphericalVoxelComponentUpdater::getClosestChunks(SphericalVoxelComponent* cmp, glm::dvec3 &coord, Chunk **chunks) {

}

void SphericalVoxelComponentUpdater::endSession(SphericalVoxelComponent* cmp) {
    delete cmp->m_physicsEngine;
    cmp->m_physicsEngine = nullptr;

    delete cmp->m_chunkManager;
    cmp->m_chunkManager = nullptr;

    delete cmp->m_chunkIo;
    cmp->m_chunkIo = nullptr;

    delete cmp->m_particleEngine;
    cmp->m_particleEngine = nullptr;

    delete cmp->m_voxelPlanetMapper;
    cmp->m_voxelPlanetMapper = nullptr;
}

void SphericalVoxelComponentUpdater::destroyVoxels() {

}

void SphericalVoxelComponentUpdater::updatePhysics(const Camera* camera) {

}
