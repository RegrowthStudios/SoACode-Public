#include "stdafx.h"
#include "GameManager.h"

#include <direct.h> //for mkdir windows
#include <time.h>

#include "BlockData.h"
#include "CAEngine.h"
#include "Chunk.h"
#include "ChunkIOManager.h"
#include "ChunkManager.h"
#include "DebugRenderer.h"
#include "FileSystem.h"
#include "InputManager.h"
#include "Inputs.h"
#include "MessageManager.h"
#include "Options.h"
#include "Particles.h"
#include "PhysicsEngine.h"
#include "Planet.h"
#include "Player.h"
#include "Rendering.h"
#include "Sound.h"
#include "TerrainGenerator.h"
#include "TexturePackLoader.h"
#include "Threadpool.h"
#include "VRayHelper.h"
#include "WSO.h"
#include "WSOAtlas.h"
#include "WSOData.h"
#include "WSOScanner.h"
#include "EventManager.h"

#include "utils.h"
#include "VoxelEditor.h"
#include "voxelWorld.h"

bool GameManager::gameInitialized = false;
bool GameManager::_systemsInitialized = false;
float GameManager::fogStart, GameManager::fogEnd;
Uint32 GameManager::maxLodTicks = 8;

VoxelWorld *GameManager::voxelWorld = nullptr;
VoxelEditor* GameManager::voxelEditor = nullptr;
PhysicsEngine *GameManager::physicsEngine = nullptr;
CAEngine *GameManager::caEngine = nullptr;
SoundEngine *GameManager::soundEngine = nullptr;
ChunkManager *GameManager::chunkManager = nullptr;
InputManager *GameManager::inputManager = nullptr;
ChunkIOManager* GameManager::chunkIOManager = nullptr;
MessageManager* GameManager::messageManager = nullptr;
WSOAtlas* GameManager::wsoAtlas = nullptr;
WSOScanner* GameManager::wsoScanner = nullptr;
DebugRenderer* GameManager::debugRenderer = nullptr;
vcore::GLProgramManager* GameManager::glProgramManager = new vcore::GLProgramManager();
TexturePackLoader* GameManager::texturePackLoader = nullptr;
vg::TextureCache* GameManager::textureCache = nullptr;
TerrainGenerator* GameManager::terrainGenerator = nullptr;
EventManager* GameManager::eventManager = nullptr;

Player *GameManager::player;
vector <Marker> GameManager::markers;
Planet *GameManager::planet = nullptr;
nString GameManager::saveFilePath = "";
GameStates GameManager::gameState = GameStates::MAINMENU;

void GameManager::initializeSystems() {
    if (_systemsInitialized == false) {
        voxelWorld = new VoxelWorld();
        voxelEditor = new VoxelEditor();
        physicsEngine = new PhysicsEngine();
        caEngine = new CAEngine();
        soundEngine = new SoundEngine();
        chunkManager = &voxelWorld->getChunkManager();
        chunkIOManager = new ChunkIOManager();
        messageManager = new MessageManager();
        wsoAtlas = new WSOAtlas();
        wsoAtlas->load("Data\\WSO\\test.wso");
        wsoScanner = new WSOScanner(wsoAtlas);
        textureCache = new vg::TextureCache();
        texturePackLoader = new TexturePackLoader(textureCache);
        terrainGenerator = new TerrainGenerator();
        
        debugRenderer = new DebugRenderer();
 
        _systemsInitialized = true;
    }
}

void GameManager::registerTexturesForLoad() {

    texturePackLoader->registerTexture("FarTerrain/location_marker.png");
    texturePackLoader->registerTexture("FarTerrain/terrain_texture.png", &SamplerState::LINEAR_WRAP_MIPMAP);
    texturePackLoader->registerTexture("FarTerrain/normal_leaves_billboard.png");
    texturePackLoader->registerTexture("FarTerrain/pine_leaves_billboard.png");
    texturePackLoader->registerTexture("FarTerrain/mushroom_cap_billboard.png");
    texturePackLoader->registerTexture("FarTerrain/tree_trunk_1.png");
    texturePackLoader->registerTexture("Blocks/Liquids/water_normal_map.png", &SamplerState::LINEAR_WRAP_MIPMAP);

    texturePackLoader->registerTexture("Sky/StarSkybox/front.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/right.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/top.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/left.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/bottom.png");
    texturePackLoader->registerTexture("Sky/StarSkybox/back.png");

    texturePackLoader->registerTexture("FarTerrain/water_noise.png", &SamplerState::LINEAR_WRAP_MIPMAP);
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

    starboxTextures[0] = textureCache->findTexture("Sky/StarSkybox/front.png");
    starboxTextures[1] = textureCache->findTexture("Sky/StarSkybox/right.png");
    starboxTextures[2] = textureCache->findTexture("Sky/StarSkybox/top.png");
    starboxTextures[3] = textureCache->findTexture("Sky/StarSkybox/left.png");
    starboxTextures[4] = textureCache->findTexture("Sky/StarSkybox/bottom.png");
    starboxTextures[5] = textureCache->findTexture("Sky/StarSkybox/back.png");

    waterNoiseTexture = textureCache->findTexture("FarTerrain/water_noise.png");
    ballMaskTexture = textureCache->findTexture("Particle/ball_mask.png");
    crosshairTexture = textureCache->findTexture("GUI/crosshair.png");

    // TODO(Ben): Parallelize this
    logoTexture = textureCache->addTexture("Textures/logo.png");
    sunTexture = textureCache->addTexture("Textures/sun_texture.png");
    BlankTextureID = textureCache->addTexture("Textures/blank.png", &SamplerState::POINT_CLAMP);
    explosionTexture = textureCache->addTexture("Textures/explosion.png");
    fireTexture = textureCache->addTexture("Textures/fire.png");
}

void GameManager::initializeSound() {
    soundEngine->Initialize();
    soundEngine->LoadAllSounds();
}

void GameManager::saveState() {
    savePlayerState();
    saveOptions();
    voxelWorld->getChunkManager().saveAllChunks();
}

void GameManager::savePlayerState() {
    fileManager.savePlayerFile(player);
}

int GameManager::newGame(string saveName) {
    string dirPath = "Saves/";

    for (size_t i = 0; i < saveName.size(); i++) {
        if (!((saveName[i] >= '0' && saveName[i] <= '9') || (saveName[i] >= 'a' && saveName[i] <= 'z') || (saveName[i] >= 'A' && saveName[i] <= 'Z') || (saveName[i] == ' ') || (saveName[i] == '_'))) {
            return 1;
        }
    }
    cout << "CREATING " << saveName << endl;

    if (_mkdir(dirPath.c_str()) == 0){ //Create the save directory if it's not there. Will fail if it is, but that's ok. Should probably be in CreateSaveFile.
        cout << "Save directory " + dirPath + " did not exist and was created!" << endl;
    }

    int rv = fileManager.createSaveFile(dirPath + saveName);
    if (rv != 2) {
        fileManager.createWorldFile(dirPath + saveName + "/World/");
        saveFilePath = dirPath + saveName;
        chunkIOManager->saveVersionFile();
    }

    return rv;
}

int GameManager::loadGame(string saveName) {
    cout << "LOADING " << saveName << endl;

    //SaveFileInput(); //get the save file for the game

    string dirPath = "Saves/";
    fileManager.makeSaveDirectories(dirPath + saveName);
    if (fileManager.setSaveFile(dirPath + saveName) != 0) {
        cout << "Could not set save file.\n";
        return 1;
    }
    string planetName = fileManager.getWorldString(dirPath + saveName + "/World/");
    if (planetName == "") {
        cout << "NO PLANET NAME";
        return 1;
    }

    saveFilePath = dirPath + saveName;
    chunkIOManager->checkVersion();

    return 0;
}

void BindVBOIndicesID() {
    vector<GLuint> indices;
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

void GameManager::loadPlanet(string filePath) {
    //DrawLoadingScreen("Initializing planet...");
    if (planet) {
        delete planet;
        planet = NULL;
    }
    Planet *newPlanet = new Planet;

    newPlanet->initialize(filePath);

    planet = newPlanet;

    GLuint startTimer = SDL_GetTicks();

    debugTicksDone = 0;
    debugTicks = SDL_GetTicks();

    BindVBOIndicesID();

}

void GameManager::initializePlanet(const glm::dvec3 cameraPos) {
    if (!planet) {
        showMessage("Tried to initialize planet before loading!");
        exit(334);
    }
    planet->initializeTerrain(cameraPos);
}

void GameManager::initializeVoxelWorld(Player *playr) {
    gameInitialized = 1;

    bool atSurface = 1;
    player = playr;

    if (player) {
        if (fileManager.loadPlayerFile(player)) {
            atSurface = 0; //dont need to set height
        }
    }

    voxelWorld->initialize(player->facePosition, &player->voxelMapData, planet, 0);

    if (atSurface) player->facePosition.y = 0;// voxelWorld->getCenterY();

    player->gridPosition = player->facePosition;

    player->setNearestPlanet(planet->scaledRadius, planet->atmosphere.radius, planet->facecsGridWidth);

    double dist = player->facePosition.y + planet->radius;
    player->update(1, planet->getGravityAccel(dist), planet->getAirFrictionForce(dist, glm::length(player->velocity)));
}

int ticksArray2[10];
int ticksArrayIndex2 = 0;

void GameManager::update() {
    static int saveStateTicks = SDL_GetTicks();

    if (gameInitialized) {
        if (player->isOnPlanet) {

            HeightData tmpHeightData;
            if (!voxelWorld->getChunkManager().getPositionHeightData((int)player->headPosition.x, (int)player->headPosition.z, tmpHeightData)) {
                player->currBiome = tmpHeightData.biome;
                player->currTemp = tmpHeightData.temperature;
                player->currHumidity = tmpHeightData.rainfall;
            } else {
                player->currBiome = NULL;
                player->currTemp = -1;
                player->currHumidity = -1;
            }

            player->currCh = NULL;
            if (player->currCh != NULL) {
                if (player->currCh->isAccessible) {
                    int x = player->headPosition.x - player->currCh->gridPosition.x;
                    int y = player->headPosition.y - player->currCh->gridPosition.y;
                    int z = player->headPosition.z - player->currCh->gridPosition.z;
                    int c = y * CHUNK_WIDTH * CHUNK_WIDTH + z * CHUNK_WIDTH + x;
                    player->headInBlock = player->currCh->getBlockData(c);
                    player->headVoxelLight = player->currCh->getLampLight(c) - 8;
                    player->headSunLight = player->currCh->getSunlight(c) - 8;
                    if (player->headVoxelLight < 0) player->headVoxelLight = 0;
                    if (player->headSunLight < 0) player->headSunLight = 0;
                }
            }

            voxelWorld->update(player->headPosition, glm::dvec3(player->chunkDirection()));

            if (inputManager->getKey(INPUT_BLOCK_SCANNER)) {
                player->scannedBlock = voxelWorld->getChunkManager().getBlockFromDir(glm::dvec3(player->chunkDirection()), player->headPosition);
            } else {
                player->scannedBlock = NONE;
            }

        } else {
            //    closestLodDistance = (glm::length(player->worldPosition) - chunkManager.planet->radius - 20000)*0.7;
            player->currBiome = NULL;
            player->currTemp = -1;
            player->currHumidity = -1;
        }
    }

    voxelWorld->getPlanet()->rotationUpdate();

    updatePlanet(player->worldPosition, maxLodTicks);   //SOMETIMES TAKING A LONG TIME!

    if (gameInitialized) {
        particleEngine.update();

        if (SDL_GetTicks() - saveStateTicks >= 20000) {
            saveStateTicks = SDL_GetTicks();
            savePlayerState();
        }
    }

}

void GameManager::updatePlanet(glm::dvec3 worldPosition, GLuint maxTicks) {
    planet->updateLODs(worldPosition, maxTicks);
}

void GameManager::drawSpace(glm::mat4 &VP, bool connectedToPlanet) {
    glm::mat4 IMVP;
    if (connectedToPlanet) {
        IMVP = VP * GameManager::planet->invRotationMatrix;
    } else {
        IMVP = VP;
    }

    glDepthMask(GL_FALSE);
    if (!drawMode) DrawStars((float)0, IMVP);
    DrawSun((float)0, IMVP);
    glDepthMask(GL_TRUE);
}

void GameManager::drawPlanet(glm::dvec3 worldPos, glm::mat4 &VP, const glm::mat4 &V, float ambVal, glm::vec3 lightPos, float fadeDist, bool connectedToPlanet) {

    GameManager::planet->draw(0, VP, V, lightPos, worldPos, ambVal, fadeDist, connectedToPlanet);

    if (connectedToPlanet) {
        if (!drawMode) GameManager::planet->atmosphere.draw((float)0, VP, glm::vec3((GameManager::planet->invRotationMatrix) * glm::vec4(lightPos, 1.0)), worldPos);
    } else {
        if (!drawMode) GameManager::planet->atmosphere.draw((float)0, VP, lightPos, worldPos);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);
    GameManager::planet->drawTrees(VP, worldPos, ambVal);
}

void GameManager::addMarker(glm::dvec3 pos, string name, glm::vec3 color) {
    markers.push_back(Marker(pos, name, color));
    markers.back().num = markers.size() - 1;
}

bool isSolidBlock(const i32& blockID) {
    return blockID && (blockID < LOWWATER || blockID > FULLWATER);
}

void GameManager::clickDragRay(bool isBreakRay) {
#define MAX_RANGE 120.0f

    VoxelRayQuery rq;
    if (isBreakRay) {
        // Obtain The Simple Query
        rq = VRayHelper::getQuery(player->getChunkCamera().position(), player->chunkDirection(), MAX_RANGE, chunkManager, isSolidBlock);

        // Check If Something Was Picked
        if (rq.distance > MAX_RANGE || rq.id == NONE) return;
    } else {
        // Obtain The Full Query
        VoxelRayFullQuery rfq = VRayHelper::getFullQuery(player->getChunkCamera().position(), player->chunkDirection(), MAX_RANGE, chunkManager, isSolidBlock);

        // Check If Something Was Picked
        if (rfq.inner.distance > MAX_RANGE || rfq.inner.id == NONE) return;

        // We Want This Indexing Information From The Query
        rq = rfq.outer;
    }

    if (rq.chunk == nullptr) {
        return;
    }

    i32v3 position = rq.location;
    if (voxelEditor->isEditing() == false) {
        voxelEditor->setStartPosition(position);
        voxelEditor->setEndPosition(position);
    } else {
        voxelEditor->setEndPosition(position);
    }
}
void GameManager::scanWSO() {

#define SCAN_MAX_DISTANCE 20.0
    VoxelRayQuery rq = VRayHelper::getQuery(
        player->getChunkCamera().position(),
        player->getChunkCamera().direction(),
        SCAN_MAX_DISTANCE,
        GameManager::chunkManager,
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
    }
}

void GameManager::onQuit() {
    GLuint st = SDL_GetTicks();
    saveState();
    voxelWorld->endSession();
    markers.clear();
}

void GameManager::endSession() {
    player->isUnderWater = 0;
    onQuit();
    physicsEngine->clearAll();
#ifdef _DEBUG 
    //_CrtDumpMemoryLeaks();
#endif
}