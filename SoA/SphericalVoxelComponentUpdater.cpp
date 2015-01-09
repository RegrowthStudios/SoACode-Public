#include "stdafx.h"
#include "SphericalVoxelComponentUpdater.h"

#include "ChunkManager.h"
#include "SpaceSystem.h"
#include "SphericalVoxelComponent.h"

void SphericalVoxelComponentUpdater::update(SpaceSystem* spaceSystem, const Camera* voxelCamera) {
    for (auto& it : spaceSystem->m_sphericalVoxelCT) {
        it.second.chunkManager->update(voxelCamera);
    }
}

void SphericalVoxelComponentUpdater::glUpdate(SpaceSystem* spaceSystem) {

}

void SphericalVoxelComponentUpdater::getClosestChunks(SphericalVoxelComponent* cmp, glm::dvec3 &coord, Chunk **chunks) {

}

void SphericalVoxelComponentUpdater::endSession(SphericalVoxelComponent* cmp) {
    delete cmp->physicsEngine;
    cmp->physicsEngine = nullptr;

    delete cmp->chunkManager;
    cmp->chunkManager = nullptr;

    delete cmp->chunkIo;
    cmp->chunkIo = nullptr;

    delete cmp->particleEngine;
    cmp->particleEngine = nullptr;

    delete cmp->voxelPlanetMapper;
    cmp->voxelPlanetMapper = nullptr;
}

void SphericalVoxelComponentUpdater::destroyVoxels() {

}

void SphericalVoxelComponentUpdater::updatePhysics(const Camera* camera) {

}
