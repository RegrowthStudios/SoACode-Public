#include "stdafx.h"
#include "SphericalVoxelComponentUpdater.h"

#include "ChunkManager.h"
#include "GameSystem.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SpaceSystemComponents.h"

void SphericalVoxelComponentUpdater::update(SpaceSystem* spaceSystem, const GameSystem* gameSystem, const SoaState* soaState) {
    if (spaceSystem->m_sphericalVoxelCT.getComponentListSize() > 1) {

        auto& playerPosCmp = gameSystem->voxelPositionCT.getFromEntity(soaState->playerEntity);
        for (auto& it : spaceSystem->m_sphericalVoxelCT) {
            it.second.chunkManager->update(playerPosCmp.position);
        }
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
