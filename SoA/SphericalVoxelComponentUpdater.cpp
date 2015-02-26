#include "stdafx.h"
#include "SphericalVoxelComponentUpdater.h"

#include "ChunkManager.h"
#include "GameSystem.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SpaceSystemComponents.h"

void SphericalVoxelComponentUpdater::update(SpaceSystem* spaceSystem, const GameSystem* gameSystem, const SoaState* soaState) {
    if (spaceSystem->m_sphericalVoxelCT.getComponentListSize() > 1) {

        // TODO(Ben): This is temporary hard coded player stuff.
        auto& playerPosCmp = gameSystem->voxelPosition.getFromEntity(soaState->playerEntity);
        auto& playerFrustumCmp = gameSystem->frustum.getFromEntity(soaState->playerEntity);
        

        for (auto& it : spaceSystem->m_sphericalVoxelCT) {
            if (it.second.chunkManager) {
                it.second.chunkManager->update(playerPosCmp.gridPosition.pos, &playerFrustumCmp.frustum);
            }
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
}

void SphericalVoxelComponentUpdater::updateComponent(SphericalVoxelComponent& svc, const f64v3& position, const Frustum* frustum) {

}

void SphericalVoxelComponentUpdater::updatePhysics(const Camera* camera) {

}
