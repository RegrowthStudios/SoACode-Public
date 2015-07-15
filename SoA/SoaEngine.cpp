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

#define SET_RANGE(a, b, name) a.##name.min = b.##name.x; a.##name.max = b.##name.y;

#define TRY_SET_BLOCK(id, bp, name) bp = blocks.hasBlock(name); if (bp) id = bp->ID;

void setTreeFruitProperties(TreeFruitProperties& fp, const FruitKegProperties& kp, const PlanetGenData* genData) {
    SET_RANGE(fp, kp, chance);
    auto& it = genData->floraMap.find(kp.flora);
    if (it != genData->floraMap.end()) {
        fp.flora = it->second;
    }
}

void setTreeLeafProperties(TreeLeafProperties& lp, const LeafKegProperties& kp, const PlanetGenData* genData, const BlockPack& blocks) {
    lp.type = kp.type;
    setTreeFruitProperties(lp.fruitProps, kp.fruitProps, genData);
    const Block* b;
    switch (lp.type) {
        case TreeLeafType::ROUND:
            SET_RANGE(lp, kp, round.radius);
            TRY_SET_BLOCK(lp.round.blockID, b, kp.roundBlock);
            break;
        case TreeLeafType::CLUSTER:
            SET_RANGE(lp, kp, cluster.width);
            SET_RANGE(lp, kp, cluster.height);
            TRY_SET_BLOCK(lp.cluster.blockID, b, kp.clusterBlock);
            break;
        case TreeLeafType::PINE:
            SET_RANGE(lp, kp, pine.thickness);
            TRY_SET_BLOCK(lp.pine.blockID, b, kp.pineBlock);
            break;
        case TreeLeafType::MUSHROOM:
            SET_RANGE(lp, kp, mushroom.lengthMod);
            SET_RANGE(lp, kp, mushroom.curlLength);
            SET_RANGE(lp, kp, mushroom.capThickness);
            SET_RANGE(lp, kp, mushroom.gillThickness);
            TRY_SET_BLOCK(lp.mushroom.gillBlockID, b, kp.mushGillBlock);
            TRY_SET_BLOCK(lp.mushroom.capBlockID, b, kp.mushCapBlock);
            break;
        case TreeLeafType::NONE:
            fprintf(stderr, "Warning: Missing tree leaf type in setTreeLeafProperties()\n");
            break;
    }
}

void setTreeBranchProperties(TreeBranchProperties& bp, const BranchKegProperties& kp, const PlanetGenData* genData, const BlockPack& blocks) {
    SET_RANGE(bp, kp, coreWidth);
    SET_RANGE(bp, kp, barkWidth);
    SET_RANGE(bp, kp, branchChance);
    const Block* b;
    TRY_SET_BLOCK(bp.coreBlockID, b,kp.coreBlock);
    TRY_SET_BLOCK(bp.barkBlockID, b, kp.barkBlock);
    setTreeFruitProperties(bp.fruitProps, kp.fruitProps, genData);
    setTreeLeafProperties(bp.leafProps, kp.leafProps, genData, blocks);
}

void SoaEngine::initVoxelGen(PlanetGenData* genData, const BlockPack& blocks) {
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
        genData->flora.resize(blockInfo.floraBlockNames.size());
        for (size_t i = 0; i < blockInfo.floraBlockNames.size(); i++) {
            const Block* b = blocks.hasBlock(blockInfo.floraBlockNames[i]);
            if (b) {
                genData->floraMap[blockInfo.floraBlockNames[i]] = i;
                genData->flora[i].block = b->ID;
            }
        }
        std::vector<BlockIdentifier>().swap(blockInfo.floraBlockNames);

        // Set biomes flora
        for (auto& biome : genData->biomes) {
            auto& it = blockInfo.biomeFlora.find(&biome);
            if (it != blockInfo.biomeFlora.end()) {
                auto& kList = it->second;
                biome.flora.resize(kList.size());
                // Iterate through keg properties
                for (size_t i = 0; i < kList.size(); i++) {
                    auto& kp = kList[i];
                    auto& mit = genData->floraMap.find(kp.id);
                    if (mit != genData->floraMap.end()) {
                        biome.flora[i].chance = kp.chance;
                        biome.flora[i].data = genData->flora[mit->second];
                        biome.flora[i].id = i;
                    } else {
                        fprintf(stderr, "Failed to find flora id %s", kp.id.c_str());
                    }
                }
            }
        }
        std::map<const Biome*, std::vector<BiomeFloraKegProperties>>().swap(blockInfo.biomeFlora);
    
        // Set tree datas
        genData->trees.resize(blockInfo.trees.size());
        for (size_t i = 0; i < blockInfo.trees.size(); ++i) {
            TreeData& td = genData->trees[i];
            const TreeKegProperties& kp = blockInfo.trees[i];
            // Add to lookup map
            genData->treeMap[kp.id] = i;
            // Set height range
            SET_RANGE(td, kp, heightRange);

            // Set trunk properties
            td.trunkProps.resize(kp.trunkProps.size());
            for (size_t j = 0; j < kp.trunkProps.size(); j++) {
                TreeTrunkProperties& tp = td.trunkProps[j];
                const TrunkKegProperties& tkp = kp.trunkProps[j];
                const Block* b;
                // Blocks
                TRY_SET_BLOCK(tp.barkBlockID, b, tkp.barkBlock);
                TRY_SET_BLOCK(tp.coreBlockID, b, tkp.coreBlock);
                // Set ranges
                SET_RANGE(tp, tkp, coreWidth);
                SET_RANGE(tp, tkp, barkWidth);
                SET_RANGE(tp, tkp, branchChance);
                tp.slope.min.min = tkp.slope[0].x;
                tp.slope.min.max = tkp.slope[0].y;
                tp.slope.max.min = tkp.slope[1].x;
                tp.slope.max.max = tkp.slope[1].y;
                setTreeFruitProperties(tp.fruitProps, tkp.fruitProps, genData);
                setTreeBranchProperties(tp.branchProps, tkp.branchProps, genData, blocks);
                setTreeLeafProperties(tp.leafProps, tkp.leafProps, genData, blocks);
            }
        }

        // Set biomes trees
    }
}
#undef SET_RANGE

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

