#include "stdafx.h"
#include "GameManager.h"

#include <direct.h> //for mkdir windows
#include <ctime>

#include <Vorb/Threadpool.h>
#include <Vorb/utils.h>

#include "BlockData.h"
#include "CAEngine.h"
#include "Chunk.h"
#include "ChunkIOManager.h"
#include "DebugRenderer.h"
#include "InputMapper.h"
#include "Inputs.h"
#include "SoaOptions.h"
#include "Particles.h"
#include "PhysicsEngine.h"
#include "Rendering.h"
#include "VRayHelper.h"
#include "WSO.h"
#include "WSOAtlas.h"
#include "WSOData.h"
#include "WSOScanner.h"

#include "VoxelEditor.h"

bool GameManager::gameInitialized = false;
bool GameManager::_systemsInitialized = false;
float GameManager::fogStart, GameManager::fogEnd;

VoxelEditor* GameManager::voxelEditor = nullptr;
WSOAtlas* GameManager::wsoAtlas = nullptr;
WSOScanner* GameManager::wsoScanner = nullptr;
vg::TextureCache* GameManager::textureCache = nullptr;

void GameManager::initializeSystems() {
    if (_systemsInitialized == false) {
        voxelEditor = new VoxelEditor();
        wsoAtlas = new WSOAtlas();
        wsoAtlas->load("Data\\WSO\\test.wso");
        wsoScanner = new WSOScanner(wsoAtlas);
        textureCache = new vg::TextureCache();

        _systemsInitialized = true;
    }
}

void GameManager::registerTexturesForLoad() {

    //texturePackLoader->registerTexture("FarTerrain/location_marker.png");
    //texturePackLoader->registerTexture("FarTerrain/terrain_texture.png", &vg::SamplerState::LINEAR_WRAP_MIPMAP);
    //texturePackLoader->registerTexture("FarTerrain/normal_leaves_billboard.png");
    //texturePackLoader->registerTexture("FarTerrain/pine_leaves_billboard.png");
    //texturePackLoader->registerTexture("FarTerrain/mushroom_cap_billboard.png");
    //texturePackLoader->registerTexture("FarTerrain/tree_trunk_1.png");
    //texturePackLoader->registerTexture("Blocks/Liquids/water_normal_map.png", &vg::SamplerState::LINEAR_WRAP_MIPMAP);

    //texturePackLoader->registerTexture("Sky/StarSkybox/front.png");
    //texturePackLoader->registerTexture("Sky/StarSkybox/right.png");
    //texturePackLoader->registerTexture("Sky/StarSkybox/top.png");
    //texturePackLoader->registerTexture("Sky/StarSkybox/left.png");
    //texturePackLoader->registerTexture("Sky/StarSkybox/bottom.png");
    //texturePackLoader->registerTexture("Sky/StarSkybox/back.png");

    //texturePackLoader->registerTexture("FarTerrain/water_noise.png", &vg::SamplerState::LINEAR_WRAP_MIPMAP);
    //texturePackLoader->registerTexture("Particle/ball_mask.png");

    //texturePackLoader->registerTexture("GUI/crosshair.png");
}

void GameManager::getTextureHandles() {

}

void GameManager::saveState() {
    savePlayerState();
  //  saveOptions();
  //  voxelWorld->getChunkManager().saveAllChunks();
}

void GameManager::savePlayerState() {
   // fileManager.savePlayerFile(player);
}

bool isSolidBlock(const i32& blockID) {
    return true; // return blockID && (blockID < LOWWATER || blockID > FULLWATER);
}

void GameManager::clickDragRay(ChunkManager* chunkManager, Player* player, bool isBreakRay) {
#define MAX_RANGE 120.0f

    //VoxelRayQuery rq;
    //if (isBreakRay) {
    //    // Obtain The Simple Query
    //    rq = VRayHelper::getQuery(player->getChunkCamera().getPosition(), player->chunkDirection(), MAX_RANGE, chunkManager, isSolidBlock);

    //    // Check If Something Was Picked
    //    if (rq.distance > MAX_RANGE || rq.id == NONE) return;
    //} else {
    //    // Obtain The Full Query
    //    VoxelRayFullQuery rfq = VRayHelper::getFullQuery(player->getChunkCamera().getPosition(), player->chunkDirection(), MAX_RANGE, chunkManager, isSolidBlock);

    //    // Check If Something Was Picked
    //    if (rfq.inner.distance > MAX_RANGE || rfq.inner.id == NONE) return;

    //    // We Want This Indexing Information From The Query
    //    rq = rfq.outer;
    //}

    //if (rq.chunk == nullptr) {
    //    return;
    //}

    //i32v3 position = rq.location;
    //if (voxelEditor->isEditing() == false) {
    //    voxelEditor->setStartPosition(position);
    //    voxelEditor->setEndPosition(position);
    //} else {
    //    voxelEditor->setEndPosition(position);
    //}
}
void GameManager::scanWSO(ChunkManager* chunkManager, Player* player) {

#define SCAN_MAX_DISTANCE 20.0
    /* VoxelRayQuery rq = VRayHelper::getQuery(
         player->getChunkCamera().getPosition(),
         player->getChunkCamera().getDirection(),
         SCAN_MAX_DISTANCE,
         chunkManager,
         isSolidBlock
         );
         if (rq.distance > SCAN_MAX_DISTANCE || rq.id == 0) return;

         auto wsos = wsoScanner->scanWSOs(rq.location, chunkManager);

         for (i32 i = 0; i < wsos.size(); i++) {
         f32v3 wsoPos(wsos[i]->position);
         f32v3 wsoSize(wsos[i]->data->size);
         wsoPos += wsoSize * 0.5f;

         debugRenderer->drawCube(wsoPos, wsoSize + 0.3f, f32v4(1, 1, 0, 0.1f), 2.0);

         delete wsos[i];
         }*/
}

void GameManager::onQuit() {
    GLuint st = SDL_GetTicks();
    saveState();
}

void GameManager::endSession() {
    onQuit();
#ifdef _DEBUG 
    //_CrtDumpMemoryLeaks();
#endif
}