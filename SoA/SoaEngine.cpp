#include "stdafx.h"
#include "SoaEngine.h"

#include "BlockData.h"
#include "BlockPack.h"
#include "ChunkMeshManager.h"
#include "DebugRenderer.h"
#include "MeshManager.h"
#include "PlanetLoader.h"
#include "ProgramGenDelegate.h"
#include "SoAState.h"
#include "SpaceSystemAssemblages.h"

#define M_PER_KM 1000.0

OptionsController SoaEngine::optionsController;

void SoaEngine::initOptions(SoaOptions& options) {
    options.addOption(OPT_PLANET_DETAIL, "Planet Detail", OptionValue(1));
    options.addOption(OPT_VOXEL_RENDER_DISTANCE, "Voxel Render Distance", OptionValue(144.0f));
    options.addOption(OPT_HUD_MODE, "Hud Mode", OptionValue(0));
    options.addOption(OPT_TEXTURE_RES, "Texture Resolution", OptionValue(32));
    options.addOption(OPT_MOTION_BLUR, "Motion Blur", OptionValue(0));
    options.addOption(OPT_DEPTH_OF_FIELD, "Depth Of Field", OptionValue(0));
    options.addOption(OPT_MSAA, "MSAA", OptionValue(0));
    options.addOption(OPT_MAX_MSAA, "Max MSAA", OptionValue(8));
    options.addOption(OPT_SPECULAR_EXPONENT, "Specular Exponent", OptionValue(8.0f));
    options.addOption(OPT_SPECULAR_INTENSITY, "Specular Intensity", OptionValue(0.3f));
    options.addOption(OPT_HDR_EXPOSURE, "HDR Exposure", OptionValue(0.5f)); //TODO(Ben): Useless?
    options.addOption(OPT_GAMMA, "Gamma", OptionValue(1.0f));
    options.addOption(OPT_SEC_COLOR_MULT, "Sec Color Mult", OptionValue(0.1f)); //TODO(Ben): Useless?
    options.addOption(OPT_FOV, "FOV", OptionValue(70.0f));
    options.addOption(OPT_VSYNC, "VSYNC", OptionValue(true));
    options.addOption(OPT_MAX_FPS, "Max FPS", OptionValue(60.0f)); //TODO(Ben): appdata.config?
    options.addOption(OPT_VOXEL_LOD_THRESHOLD, "Voxel LOD Threshold", OptionValue(128.0f));
    options.addOption(OPT_MUSIC_VOLUME, "Music Volume", OptionValue(1.0f));
    options.addOption(OPT_EFFECT_VOLUME, "Effect Volume", OptionValue(1.0f));
    options.addOption(OPT_MOUSE_SENSITIVITY, "Mouse Sensitivity", OptionValue(1.0f));
    options.addOption(OPT_INVERT_MOUSE, "Invert Mouse", OptionValue(false));
    options.addOption(OPT_FULLSCREEN, "Fullscreen", OptionValue(false));
    options.addOption(OPT_BORDERLESS, "Borderless Window", OptionValue(false));
    options.addOption(OPT_SCREEN_WIDTH, "Screen Width", OptionValue(1280));
    options.addOption(OPT_SCREEN_HEIGHT, "Screen Height", OptionValue(720));
    options.addStringOption("Texture Pack", "Default");

    SoaEngine::optionsController.setDefault();
}

void SoaEngine::initState(SoaState* state) {
    state->gameSystem = new GameSystem;
    state->spaceSystem = new SpaceSystem;
    state->debugRenderer = new DebugRenderer;
    state->meshManager = new MeshManager;
    state->chunkMeshManager = new ChunkMeshManager;
    state->systemIoManager = new vio::IOManager;
    state->systemViewer = new MainMenuSystemViewer;
    state->planetLoader = new PlanetLoader;
    // TODO(Ben): This is also elsewhere?
    state->texturePathResolver.init("Textures/TexturePacks/" + soaOptions.getStringOption("Texture Pack").defaultValue + "/",
                                    "Textures/TexturePacks/" + soaOptions.getStringOption("Texture Pack").value + "/");
   
    // TODO(Ben): Don't hardcode this. Load a texture pack file
    state->blockTextures = new BlockTexturePack;
    state->blockTextures->init(32, 4096);
    state->blockTextureLoader.init(&state->texturePathResolver, state->blockTextures);

    { // Threadpool init
        size_t hc = std::thread::hardware_concurrency();
        // Remove two threads for the render thread and main thread
        if (hc > 1) hc--;
        if (hc > 1) hc--;

        // Drop a thread so we don't steal the hardware on debug
#ifdef DEBUG
        if (hc > 1) hc--;
#endif

        // Initialize the threadpool with hc threads
        state->threadPool = new vcore::ThreadPool<WorkerData>();
        state->threadPool->init(hc);
        // Give some time for the threads to spin up
        SDL_Delay(100);
    }
}

bool SoaEngine::loadSpaceSystem(SoaState* state, const nString& filePath) {

    AutoDelegatePool pool;
    vpath path = "SoASpace.log";
    vfile file;
    path.asFile(&file);

    state->planetLoader->init(state->systemIoManager);

    vfstream fs = file.open(vio::FileOpenFlags::READ_WRITE_CREATE);
    pool.addAutoHook(state->spaceSystem->onEntityAdded, [=] (Sender, vecs::EntityID eid) {
        fs.write("Entity added: %d\n", eid);
    });
    for (auto namedTable : state->spaceSystem->getComponents()) {
        auto table = state->spaceSystem->getComponentTable(namedTable.first);
        pool.addAutoHook(table->onEntityAdded, [=] (Sender, vecs::ComponentID cid, vecs::EntityID eid) {
            fs.write("Component \"%s\" added: %d -> Entity %d\n", namedTable.first.c_str(), cid, eid);
        });
    }

    // Load system
    SpaceSystemLoader spaceSystemLoader;
    spaceSystemLoader.init(state);
    spaceSystemLoader.loadStarSystem(filePath);

    pool.dispose();
    return true;
}

bool SoaEngine::loadGameSystem(SoaState* state) {
    // TODO(Ben): Implement
    
    return true;
}

void SoaEngine::setPlanetBlocks(PlanetGenData* genData, BlockPack& blocks) {
    PlanetBlockInitInfo& blockInfo = genData->blockInfo;
    if (genData) {
        // Set all block layers
        for (size_t i = 0; i < blockInfo.blockLayerNames.size(); i++) {
            ui16 blockID = blocks[blockInfo.blockLayerNames[i]].ID;
            genData->blockLayers[i].block = blockID;
        }
        std::vector<BlockIdentifier>().swap(blockInfo.blockLayerNames);
        // Set liquid block
        if (blockInfo.liquidBlockName.length()) {
            if (blocks.hasBlock(blockInfo.liquidBlockName)) {
                genData->liquidBlock = blocks[blockInfo.liquidBlockName].ID;
                nString().swap(blockInfo.liquidBlockName); // clear memory
            }
        }
        // Set surface block
        if (blockInfo.surfaceBlockName.length()) {
            if (blocks.hasBlock(blockInfo.surfaceBlockName)) {
                genData->surfaceBlock = blocks[blockInfo.surfaceBlockName].ID;
                nString().swap(blockInfo.surfaceBlockName); // clear memory
            }
        }
        // Set flora blocks
        for (size_t i = 0; i < blockInfo.floraNames.size(); i++) {
            const Block* b = blocks.hasBlock(blockInfo.floraNames[i]);
            if (b) {
                genData->flora[i].block = b->ID;
            }
        }
        std::vector<BlockIdentifier>().swap(blockInfo.floraNames);
    }
}

void SoaEngine::reloadSpaceBody(SoaState* state, vecs::EntityID eid, vcore::RPCManager* glRPC) {
    SpaceSystem* spaceSystem = state->spaceSystem;
    auto& stCmp = spaceSystem->sphericalTerrain.getFromEntity(eid);
    f64 radius = stCmp.radius;
    auto npCmpID = stCmp.namePositionComponent;
    auto arCmpID = stCmp.axisRotationComponent;
    auto ftCmpID = stCmp.farTerrainComponent;
    WorldCubeFace face;
    PlanetGenData* genData = stCmp.planetGenData;
    nString filePath = genData->terrainFilePath;

    if (ftCmpID) {
        face = spaceSystem->farTerrain.getFromEntity(eid).face;
        SpaceSystemAssemblages::removeFarTerrainComponent(spaceSystem, eid);
    }
    if (stCmp.sphericalVoxelComponent) {
        SpaceSystemAssemblages::removeSphericalVoxelComponent(spaceSystem, eid);
    }

    SpaceSystemAssemblages::removeSphericalTerrainComponent(spaceSystem, eid);
    //state->planetLoader->textureCache.freeTexture(genData->liquidColorMap);
   // state->planetLoader->textureCache.freeTexture(genData->terrainColorMap);
    genData = state->planetLoader->loadPlanetGenData(filePath);
    genData->radius = radius;

    auto stCmpID = SpaceSystemAssemblages::addSphericalTerrainComponent(spaceSystem, eid, npCmpID, arCmpID,
                                                         radius,
                                                         genData,
                                                         state->threadPool);
    if (ftCmpID) {
        auto ftCmpID = SpaceSystemAssemblages::addFarTerrainComponent(spaceSystem, eid, stCmp, face);
        stCmp.farTerrainComponent = ftCmpID;
       
    }

    // TODO(Ben): this doesn't work too well.
    auto& pCmp = state->gameSystem->spacePosition.getFromEntity(state->playerEntity);
    pCmp.parentSphericalTerrainID = stCmpID;
    pCmp.parentGravityID = spaceSystem->sphericalGravity.getComponentID(eid);
    pCmp.parentEntity = eid;
}

void SoaEngine::destroyAll(SoaState* state) {
    delete state->spaceSystem;
    delete state->gameSystem;
    delete state->debugRenderer;
    delete state->meshManager;
    delete state->chunkMeshManager;
    delete state->systemViewer;
    delete state->systemIoManager;
    delete state->planetLoader;
    delete state->options;
    delete state->blockTextures;
    destroyGameSystem(state);
    destroySpaceSystem(state);
}

void SoaEngine::destroyGameSystem(SoaState* state) {
    delete state->gameSystem;
}

void SoaEngine::destroySpaceSystem(SoaState* state) {
    delete state->spaceSystem;
}

