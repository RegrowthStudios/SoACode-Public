#include "stdafx.h"
#include "SphericalVoxelComponentUpdater.h"

#include "ChunkGrid.h"
#include "ChunkManager.h"
#include "ChunkMemoryManager.h"
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
            if (it.second.chunkGrid) {
                updateComponent(it.second, playerPosCmp.gridPosition.pos, &playerFrustumCmp.frustum);
            }
        }
    }
}

void SphericalVoxelComponentUpdater::glUpdate(SpaceSystem* spaceSystem) {

}

void SphericalVoxelComponentUpdater::getClosestChunks(SphericalVoxelComponent* cmp, glm::dvec3 &coord, Chunk **chunks) {

}

// TODO(Ben): unneeded?
void SphericalVoxelComponentUpdater::endSession(SphericalVoxelComponent* cmp) {
    delete cmp->physicsEngine;
    cmp->physicsEngine = nullptr;

    delete cmp->chunkIo;
    cmp->chunkIo = nullptr;

    delete cmp->particleEngine;
    cmp->particleEngine = nullptr;
}

void SphericalVoxelComponentUpdater::updateComponent(SphericalVoxelComponent& svc, const f64v3& position, const Frustum* frustum) {
    static i32 k = 0;

    sonarDt += 0.003f*physSpeedFactor;
    if (sonarDt > 1.0f) sonarDt = 0.0f;

    updateChunks(svc, position, frustum);

    updateLoadList(4);

    if (k >= 8 || (k >= 4 && physSpeedFactor >= 2.0)) {
        svc.chunkListManager->sortLists();
        k = 0;
    }
    k++;
    // std::cout << "TASKS " << _threadPool.getFinishedTasksSizeApprox() << std::endl;
    updateLoadedChunks(4);
    updateTreesToPlace(3);
    updateMeshList(4);
    updateSetupList(4);
    updateGenerateList();

    //This doesn't function correctly
    //caveOcclusion(position);

    Chunk* ch;
    for (size_t i = 0; i < _freeWaitingChunks.size();) {
        ch = _freeWaitingChunks[i];
        if (ch->inSaveThread == false && ch->inLoadThread == false &&
            !ch->lastOwnerTask && !ch->_chunkListPtr && ch->chunkDependencies == 0 &&
            !(ch->mesh && ch->mesh->refCount)) {
            freeChunk(_freeWaitingChunks[i]);
            _freeWaitingChunks[i] = _freeWaitingChunks.back();
            _freeWaitingChunks.pop_back();
        } else {
            i++;
        }
    }

    processFinishedTasks();
    //change the parameter to true to print out the timings

    static int g = 0;
    if (++g == 10) {
        //   globalAccumulationTimer.printAll(false);
        //  std::cout << "\n";
        globalAccumulationTimer.clear();
        g = 0;
    }
}

void SphericalVoxelComponentUpdater::updateChunks(SphericalVoxelComponent& svc, const f64v3& position, const Frustum* frustum) {

    i32v3 intPosition(position);

    //ui32 sticks = SDL_GetTicks();

    static ui32 saveTicks = SDL_GetTicks();

    bool save = 0;

#define MS_PER_MINUTE 60000

    // Reset the counter for number of container compressions
    vvox::clearContainerCompressionsCounter();

    if (SDL_GetTicks() - saveTicks >= MS_PER_MINUTE) { //save once per minute
        save = 1;
        std::cout << "SAVING\n";
        saveTicks = SDL_GetTicks();
    }

    for (auto& chunk : svc.chunkMemoryManager->m_chunkMemory) { //update distances for all chunks
        Chunk& chunk = m_chunks[i];

        chunk->calculateDistance2(intPosition);

        globalAccumulationTimer.start("Update Containers");
        if (chunk->_state > ChunkStates::TREES && !chunk->lastOwnerTask) {
            chunk->updateContainers();
        }
        globalAccumulationTimer.stop();
        if (chunk->distance2 > (graphicsOptions.voxelRenderDistance + 36) * (graphicsOptions.voxelRenderDistance + 36)) { //out of maximum range

            // Only remove it if it isn't needed by its neighbors
            if (!chunk->lastOwnerTask && !chunk->chunkDependencies) {
                if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
                    m_chunkIo->addToSaveList(chunk);
                }

                freeChunk(chunk);

                m_chunks[i] = m_chunks.back();
                m_chunks.pop_back();

                globalAccumulationTimer.stop();
            }
        } else { //inside maximum range

            // Check if it is in the view frustum
            chunk->inFrustum = frustum->sphereInFrustum(f32v3(f64v3(chunk->voxelPosition + (CHUNK_WIDTH / 2)) - position), 28.0f);

            // See if neighbors need to be added
            if (chunk->numNeighbors != 6 && chunk->needsNeighbors) {
                globalAccumulationTimer.start("Update Neighbors");
                updateChunkNeighbors(chunk, intPosition);
                globalAccumulationTimer.stop();
            }
            globalAccumulationTimer.start("Rest of Update");
            // Calculate the LOD as a power of two
            int newLOD = (int)(sqrt(chunk->distance2) / graphicsOptions.voxelLODThreshold) + 1;
            //  newLOD = 2;
            if (newLOD > 6) newLOD = 6;
            if (newLOD != chunk->getLevelOfDetail()) {
                chunk->setLevelOfDetail(newLOD);
                chunk->changeState(ChunkStates::MESH);
            }

            if (isWaterUpdating && chunk->mesh != nullptr) ChunkUpdater::randomBlockUpdates(this, m_physicsEngine, chunk);

            // Check to see if it needs to be added to the mesh list
            if (!chunk->_chunkListPtr && !chunk->lastOwnerTask) {
                switch (chunk->_state) {
                    case ChunkStates::WATERMESH:
                    case ChunkStates::MESH:
                        addToMeshList(chunk);
                        globalAccumulationTimer.stop();
                        break;
                    default:
                        globalAccumulationTimer.stop();
                        break;
                }
            }

            // save if its been a minute
            if (save && chunk->dirty) {
                m_chunkIo->addToSaveList(chunk);
            }
            globalAccumulationTimer.stop();
        }
    }
}

void SphericalVoxelComponentUpdater::updatePhysics(const Camera* camera) {

}
