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
#include "FileSystem.h"
#include "InputManager.h"
#include "Inputs.h"
#include "Options.h"
#include "Particles.h"
#include "PhysicsEngine.h"
#include "Rendering.h"
#include "Sound.h"
#include "TerrainGenerator.h"
#include "TexturePackLoader.h"
#include "VRayHelper.h"
#include "WSO.h"
#include "WSOAtlas.h"
#include "WSOData.h"
#include "WSOScanner.h"

#include "VoxelEditor.h"

bool GameManager::gameInitialized = false;
bool GameManager::_systemsInitialized = false;
float GameManager::fogStart, GameManager::fogEnd;
Uint32 GameManager::maxLodTicks = 8;

VoxelEditor* GameManager::voxelEditor = nullptr;
SoundEngine *GameManager::soundEngine = nullptr;
WSOAtlas* GameManager::wsoAtlas = nullptr;
WSOScanner* GameManager::wsoScanner = nullptr;
TexturePackLoader* GameManager::texturePackLoader = nullptr;
vg::TextureCache* GameManager::textureCache = nullptr;

void GameManager::initializeSystems() {
    if (_systemsInitialized == false) {
        voxelEditor = new VoxelEditor();
        soundEngine = new SoundEngine();
        wsoAtlas = new WSOAtlas();
        wsoAtlas->load("Data\\WSO\\test.wso");
        wsoScanner = new WSOScanner(wsoAtlas);
        textureCache = new vg::TextureCache();
        texturePackLoader = new TexturePackLoader(textureCache);

        _systemsInitialized = true;
    }
}

void GameManager::registerTexturesForLoad() {

    texturePackLoader->registerTexture("FarTerrain/location_marker.png");
    texturePackLoader->registerTexture("FarTerrain/terrain_texture.png", &vg::SamplerState::LINEAR_WRAP_MIPMAP);
    texturePackLoader->registerTexture("FarTerrain/normal_leaves_billboard.png");
    texturePackLoader->registerTexture("FarTerrain/pine_leaves_billboard.png");
    texturePackLoader->registerTexture("FarTerrain/mushroom_cap_billboard.png");
    texturePackLoader->registerTexture("FarTerrain/tree_trunk_1.png");
    texturePackLoader->registerTexture("Blocks/Liquids/water_normal_map.png", &vg::SamplerState::LINEAR_WRAP_MIPMAP);

    texturePackLoader->registerTexture("Sky/StarSkybox/front.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/right.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/top.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/left.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/bottom.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/back.png");

    texturePackLoader->registerTexture("FarTerrain/water_noise.png", &vg::SamplerState::LINEAR_WRAP_MIPMAP);
    texturePackLoader->registerTexture("Particle/ball_mask.png");

    texturePackLoader->registerTexture("GUI/crosshair.png");
}

void GameManager::getTextureHandles() {

    markerTexture = textureCache->findTexture("FarTerrain/location_marker.png");
    terrainTexture = textureCache->findTexture("FarTerrain/terrain_texture.png");

    normalLeavesTexture = textureCache->findTexture("FarTerrain/normal_leaves_billboard.png");
    pineLeavesTexture = textureCache->findTexture("FarTerrain/pine_leaves_billboard.png");
    mushroomCapTexture = textureCache->findTexture("FarTerrain/mushroom_cap_billboard.png");
    treeTrunkTexture1 = textureCache->findTexture("FarTerrain/tree_trunk_1.png");
    waterNormalTexture = textureCache->findTexture("Blocks/Liquids/water_normal_map.png");

    skyboxTextures[0] = textureCache->findTexture("Sky/StarSkybox/front.png");
    skyboxTextures[1] = textureCache->findTexture("Sky/StarSkybox/right.png");
    skyboxTextures[2] = textureCache->findTexture("Sky/StarSkybox/top.png");
    skyboxTextures[3] = textureCache->findTexture("Sky/StarSkybox/left.png");
    skyboxTextures[4] = textureCache->findTexture("Sky/StarSkybox/bottom.png");
    skyboxTextures[5] = textureCache->findTexture("Sky/StarSkybox/back.png");

    waterNoiseTexture = textureCache->findTexture("FarTerrain/water_noise.png");
    ballMaskTexture = textureCache->findTexture("Particle/ball_mask.png");
    crosshairTexture = textureCache->findTexture("GUI/crosshair.png");

    // TODO(Ben): Parallelize this
    logoTexture = textureCache->addTexture("Textures/logo.png");
    sunTexture = textureCache->addTexture("Textures/sun_texture.png");
    BlankTextureID = textureCache->addTexture("Textures/blank.png", &vg::SamplerState::POINT_CLAMP);
    explosionTexture = textureCache->addTexture("Textures/explosion.png");
    fireTexture = textureCache->addTexture("Textures/fire.png");
}

void GameManager::initializeSound() {
    soundEngine->Initialize();
    soundEngine->LoadAllSounds();
}

void GameManager::saveState() {
    savePlayerState();
  //  saveOptions();
  //  voxelWorld->getChunkManager().saveAllChunks();
}

void GameManager::savePlayerState() {
   // fileManager.savePlayerFile(player);
}

void BindVBOIndicesID() {
    std::vector<GLuint> indices;
    indices.resize(589824);

    int j = 0;
    for (Uint32 i = 0; i < indices.size() - 12; i += 6) {
        indices[i] = j;
        indices[i + 1] = j + 1;
        indices[i + 2] = j + 2;
        indices[i + 3] = j + 2;
        indices[i + 4] = j + 3;
        indices[i + 5] = j;
        j += 4;
    }

    if (Chunk::vboIndicesID != 0) {
        glDeleteBuffers(1, &(Chunk::vboIndicesID));
    }
    glGenBuffers(1, &(Chunk::vboIndicesID));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (Chunk::vboIndicesID));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 500000 * sizeof(GLuint), NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 500000 * sizeof(GLuint), &(indices[0])); //arbitrarily set to 300000
}

bool isSolidBlock(const i32& blockID) {
    return blockID && (blockID < LOWWATER || blockID > FULLWATER);
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