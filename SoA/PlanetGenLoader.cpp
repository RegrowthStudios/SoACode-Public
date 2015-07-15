#include "stdafx.h"
#include "PlanetGenLoader.h"
#include "PlanetGenData.h"

#include <random>

#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/Events.hpp>
#include <Vorb/io/YAML.h>
#include <Vorb/RPC.h>
#include <Vorb/Timing.h>

#include "BlockPack.h"
#include "Errors.h"
#include "Biome.h"

typedef ui32 BiomeColorCode;

PlanetGenData* PlanetGenLoader::m_defaultGenData = nullptr;

struct BiomeKegProperties {
    Array<BiomeKegProperties> children;
    Array<BlockLayer> blockLayers;
    Array<BiomeFloraKegProperties> flora;
    Array<BiomeTreeKegProperties> trees;
    BiomeID id = "";
    ColorRGB8 mapColor = ColorRGB8(255, 255, 255);
    NoiseBase childNoise; ///< For sub biome determination
    NoiseBase terrainNoise; ///< Modifies terrain directly
    f64v2 heightRange = f64v2(0.0, 1000.0);
    f64v2 heightScale = f64v2(0.01, 0.01);
    f64v2 noiseRange = f64v2(-1.0, 1.0);
    f64v2 noiseScale = f64v2(10.0, 10.0);
    nString displayName = "Unknown";
};
KEG_TYPE_DECL(BiomeKegProperties);
KEG_TYPE_DEF_SAME_NAME(BiomeKegProperties, kt) {
    using namespace keg;
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, id, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, displayName, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, mapColor, UI8_V3);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, heightRange, F64_V2);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, heightScale, F64_V2);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, noiseRange, F64_V2);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, noiseScale, F64_V2);
    kt.addValue("blockLayers", Value::array(offsetof(BiomeKegProperties, blockLayers), Value::custom(0, "BlockLayer")));
    kt.addValue("terrainNoise", Value::custom(offsetof(BiomeKegProperties, terrainNoise), "NoiseBase", false));
    kt.addValue("flora", Value::array(offsetof(BiomeKegProperties, flora), Value::custom(0, "BiomeFloraKegProperties")));
    kt.addValue("trees", Value::array(offsetof(BiomeKegProperties, trees), Value::custom(0, "BiomeTreeKegProperties")));
    kt.addValue("childNoise", Value::custom(offsetof(BiomeKegProperties, childNoise), "NoiseBase", false));
    kt.addValue("children", Value::array(offsetof(BiomeKegProperties, children), Value::custom(0, "BiomeKegProperties", false)));
}

struct FloraKegProperties {
    nString block = "";
};
KEG_TYPE_DECL(FloraKegProperties);
KEG_TYPE_DEF_SAME_NAME(FloraKegProperties, kt) {
    using namespace keg;
    KEG_TYPE_INIT_ADD_MEMBER(kt, FloraKegProperties, block, STRING);
}

void PlanetGenLoader::init(vio::IOManager* ioManager) {
    m_iom = ioManager;
    m_textureCache.init(ioManager);
}

CALLER_DELETE PlanetGenData* PlanetGenLoader::loadPlanetGenData(const nString& terrainPath) {
    nString data;
    m_iom->readFileToString(terrainPath.c_str(), data);

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to load " + terrainPath;
        context.reader.dispose();
        return false;
    }

    PlanetGenData* genData = new PlanetGenData;
    genData->terrainFilePath = terrainPath;

    nString biomePath = "";
    nString floraPath = "";
    nString treesPath = "";
    bool didLoadBiomes = false;

    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& type, keg::Node value) {
        // Parse based on type
        if (type == "biomes") {
            biomePath = keg::convert<nString>(value);
        } else if (type == "flora") {
            floraPath = keg::convert<nString>(value);
        } else if (type == "trees") {
            treesPath = keg::convert<nString>(value);
        } else if (type == "terrainColor") {
            parseTerrainColor(context, value, genData);
        } else if (type == "liquidColor") {
            parseLiquidColor(context, value, genData);
        } else if (type == "tempLatitudeFalloff") {
            genData->tempLatitudeFalloff = keg::convert<f32>(value);
        } else if (type == "humLatitudeFalloff") {
            genData->humLatitudeFalloff = keg::convert<f32>(value);
        } else if (type == "tempHeightFalloff") {
            genData->tempHeightFalloff = keg::convert<f32>(value);
        } else if (type == "humHeightFalloff") {
            genData->humHeightFalloff = keg::convert<f32>(value);
        } else if (type == "baseHeight") {
            parseTerrainFuncs(&genData->baseTerrainFuncs, context, value);
        } else if (type == "temperature") {
            parseTerrainFuncs(&genData->tempTerrainFuncs, context, value);
        } else if (type == "humidity") {
            parseTerrainFuncs(&genData->humTerrainFuncs, context, value);
        } else if (type == "blockLayers") {
            parseBlockLayers(context, value, genData);
        } else if (type == "liquidBlock") {
            genData->blockInfo.liquidBlockName = keg::convert<nString>(value);
        } else if (type == "surfaceBlock") {
            genData->blockInfo.surfaceBlockName = keg::convert<nString>(value);
        }
    });
    context.reader.forAllInMap(node, f);
    context.reader.dispose();
    delete f;

    if (floraPath.size()) {
        loadFlora(floraPath, genData);
    }
    if (treesPath.size()) {
        loadTrees(treesPath, genData);
    }

    if (biomePath.size()) {
        loadBiomes(biomePath, genData);
    } else {
        // Set default biome
        for (int y = 0; y < BIOME_MAP_WIDTH; y++) {
            for (int x = 0; x < BIOME_MAP_WIDTH; x++) {
                genData->baseBiomeLookup[y][x] = &DEFAULT_BIOME;
            }
        }
    }
    return genData;
}

PlanetGenData* PlanetGenLoader::getDefaultGenData(vcore::RPCManager* glrpc /* = nullptr */) {
    // Lazily construct default data
    if (!m_defaultGenData) {
        // Allocate data
        m_defaultGenData = new PlanetGenData;
    }
    return m_defaultGenData;
}

CALLER_DELETE PlanetGenData* PlanetGenLoader::getRandomGenData(f32 radius, vcore::RPCManager* glrpc /* = nullptr */) {
    // Lazily construct default data

    // Allocate data
    PlanetGenData* genData = m_planetGenerator.generateRandomPlanet(SpaceObjectType::PLANET, glrpc);
    // TODO(Ben): Radius is temporary hacky fix for small planet darkness!
    if (radius < 15.0) {
        genData->baseTerrainFuncs.funcs.setData();
    }

    // Load textures
    if (glrpc) {
        vcore::RPC rpc;
        rpc.data.f = makeFunctor<Sender, void*>([&](Sender s, void* userData) {
            //genData->grassTexture = m_textureCache.addTexture("_shared/terrain_b.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            //genData->rockTexture = m_textureCache.addTexture("_shared/terrain_a.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            //genData->liquidTexture = m_textureCache.addTexture("_shared/water_a.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        });
        glrpc->invoke(&rpc, true);
    } else {
        //genData->grassTexture = m_textureCache.addTexture("_shared/terrain_b.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        //genData->rockTexture = m_textureCache.addTexture("_shared/terrain_a.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        //genData->liquidTexture = m_textureCache.addTexture("_shared/water_a.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
    }

    // Set default biome
    for (int y = 0; y < BIOME_MAP_WIDTH; y++) {
        for (int x = 0; x < BIOME_MAP_WIDTH; x++) {
            genData->baseBiomeLookup[y][x] = &DEFAULT_BIOME;
        }
    }

    return genData;
}

AtmosphereProperties PlanetGenLoader::getRandomAtmosphere() {
    static std::mt19937 generator(2636);
    static std::uniform_real_distribution<f32> randomWav(0.4f, 0.8f);
    AtmosphereProperties props;
    props.waveLength.r = randomWav(generator);
    props.waveLength.g = randomWav(generator);
    props.waveLength.b = randomWav(generator);
    return props;
}

// Helper function for loadBiomes
ui32 recursiveCountBiomes(const BiomeKegProperties& props) {
    ui32 rv = 1;
    for (size_t i = 0; i < props.children.size(); i++) {
        rv += recursiveCountBiomes(props.children[i]);
    }
    return rv;
}

const int FILTER_SIZE = 5;
const int FILTER_OFFSET = FILTER_SIZE / 2;

float blurFilter[FILTER_SIZE][FILTER_SIZE] = {
    0.04f, 0.04f, 0.04f, 0.04f, 0.04f,
    0.04f, 0.04f, 0.04f, 0.04f, 0.04f,
    0.04f, 0.04f, 0.04f, 0.04f, 0.04f,
    0.04f, 0.04f, 0.04f, 0.04f, 0.04f,
    0.04f, 0.04f, 0.04f, 0.04f, 0.04f
};

void blurBiomeMap(const std::vector<BiomeInfluence>& bMap, OUT std::vector<std::set<BiomeInfluence>>& outMap) {
    /* Very simple blur function with 5x5 box kernel */

    outMap.resize(bMap.size());

    // Loop through the map
    for (int y = 0; y < BIOME_MAP_WIDTH; y++) {
        for (int x = 0; x < BIOME_MAP_WIDTH; x++) {
            auto& b = bMap[y * BIOME_MAP_WIDTH + x];
            if (b.b) {
                // Loop through box filter
                for (int j = 0; j < FILTER_SIZE; j++) {
                    for (int k = 0; k < FILTER_SIZE; k++) {
                        int xPos = (x - FILTER_OFFSET + k);
                        int yPos = (y - FILTER_OFFSET + j);
                        // Bounds checking
                        if (xPos < 0) {
                            xPos = 0;
                        } else if (xPos >= BIOME_MAP_WIDTH) {
                            xPos = BIOME_MAP_WIDTH - 1;
                        }
                        if (yPos < 0) {
                            yPos = 0;
                        } else if (yPos >= BIOME_MAP_WIDTH) {
                            yPos = BIOME_MAP_WIDTH - 1;
                        }
                        // Get the list of biomes currently in the blurred map
                        auto& biomes = outMap[yPos * BIOME_MAP_WIDTH + xPos];
                        // See if the current biome is already there
                        auto& it = biomes.find(b);
                        // Force modify weight in set with const cast.
                        // It's ok since weight doesn't affect set position, promise!
                        if (it == biomes.end()) {
                            // Add biome and modify weight
                            const_cast<BiomeInfluence&>(*biomes.insert(b).first).weight = blurFilter[j][k] * b.weight;
                        } else {
                            // Modify existing biome weight
                            const_cast<BiomeInfluence&>(*it).weight += blurFilter[j][k] * b.weight;
                        }
                    }
                }
            }
        }
    }
}

void blurBaseBiomeMap(const Biome* baseBiomeLookup[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH], OUT std::vector<std::set<BiomeInfluence>>& outMap) {
    /* Very simple blur function with 5x5 box kernel */

    outMap.resize(BIOME_MAP_WIDTH * BIOME_MAP_WIDTH);

    // Loop through the map
    for (int y = 0; y < BIOME_MAP_WIDTH; y++) {
        for (int x = 0; x < BIOME_MAP_WIDTH; x++) {
            auto& b = baseBiomeLookup[y][x];
            // Loop through box filter
            for (int j = 0; j < FILTER_SIZE; j++) {
                for (int k = 0; k < FILTER_SIZE; k++) {
                    int xPos = (x - FILTER_OFFSET + k);
                    int yPos = (y - FILTER_OFFSET + j);
                    // Bounds checking
                    if (xPos < 0) {
                        xPos = 0;
                    } else if (xPos >= BIOME_MAP_WIDTH) {
                        xPos = BIOME_MAP_WIDTH - 1;
                    }
                    if (yPos < 0) {
                        yPos = 0;
                    } else if (yPos >= BIOME_MAP_WIDTH) {
                        yPos = BIOME_MAP_WIDTH - 1;
                    }
                    // Get the list of biomes currently in the blurred map
                    auto& biomes = outMap[yPos * BIOME_MAP_WIDTH + xPos];
                    // See if the current biome is already there
                    // TODO(Ben): Better find
                    auto& it = biomes.find({ b, 1.0f });
                    // Force modify weight in set with const cast.
                    // It's ok since weight doesn't affect set position, promise!
                    if (it == biomes.end()) {
                        // Add biome and set
                        biomes.emplace(b, blurFilter[j][k]);
                    } else {
                        // Modify existing biome weight
                        const_cast<BiomeInfluence&>(*it).weight += blurFilter[j][k];
                    }
                }
            }
        }
    }
}

void recursiveInitBiomes(Biome& biome,
                         const BiomeKegProperties& kp,
                         ui32& biomeCounter,
                         PlanetGenData* genData) {

    // Copy all biome data
    biome.id = kp.id;
    biome.blockLayers.resize(kp.blockLayers.size());
    for (size_t i = 0; i < kp.blockLayers.size(); i++) {
        biome.blockLayers[i] = kp.blockLayers[i];
    }
    biome.displayName = kp.displayName;
    biome.mapColor = kp.mapColor;
    biome.heightRange = kp.heightRange;
    biome.heightScale = kp.heightScale;
    biome.noiseRange = kp.noiseRange;
    biome.noiseScale = kp.noiseScale;
    biome.terrainNoise = kp.terrainNoise;
    biome.childNoise = kp.childNoise;

    // Construct vectors in place for flora and trees
    auto& floraPropList = genData->blockInfo.biomeFlora.insert(
        std::make_pair(&biome, std::vector<BiomeFloraKegProperties>())).first->second;
    auto& treePropList = genData->blockInfo.biomeTrees.insert(
        std::make_pair(&biome, std::vector<BiomeTreeKegProperties>())).first->second;
    // Copy flora data over
    floraPropList.resize(kp.flora.size());
    for (size_t i = 0; i < kp.flora.size(); i++) {
        floraPropList[i] = kp.flora[i];
    }
    // Copy tree data over
    treePropList.resize(kp.trees.size());
    for (size_t i = 0; i < kp.trees.size(); i++) {
        treePropList[i] = kp.trees[i];
    }

    // Recurse children
    biome.children.resize(kp.children.size());
    for (size_t i = 0; i < kp.children.size(); i++) {
        Biome& nextBiome = genData->biomes[biomeCounter++];
        recursiveInitBiomes(nextBiome, kp.children[i], biomeCounter, genData);
        biome.children[i] = &nextBiome;
    }
}

void PlanetGenLoader::loadFlora(const nString& filePath, PlanetGenData* genData) {
    // Read in the file
    nString data;
    m_iom->readFileToString(filePath.c_str(), data);

    // Get the read context and error check
    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to load " + filePath;
        context.reader.dispose();
        return;
    }

    auto baseParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        FloraKegProperties properties;
        keg::parse((ui8*)&properties, value, context, &KEG_GLOBAL_TYPE(FloraKegProperties));
        
        genData->blockInfo.floraBlockNames.push_back(properties.block);
    });
    context.reader.forAllInMap(node, baseParser);
    delete baseParser;
    context.reader.dispose();
}

// Conditionally parses a value so it can be either a v2 or a single value
// When its single, it sets both values of the v2 to that value
#define PARSE_V2(type, v) \
if (keg::getType(value) == keg::NodeType::VALUE) { \
    keg::evalData((ui8*)&##v, &##type##Val, value, context); \
    ##v.y = ##v.x; \
} else { \
    keg::evalData((ui8*)&##v, &##type##v2Val, value, context); \
} 

void PlanetGenLoader::loadTrees(const nString& filePath, PlanetGenData* genData) {
    // Read in the file
    nString data;
    m_iom->readFileToString(filePath.c_str(), data);

    // Get the read context and error check
    // TODO(Ben): Too much copy paste
    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to load " + filePath;
        context.reader.dispose();
        return;
    }

    TreeKegProperties* treeProps;
    // Handles
    TrunkKegProperties* trunkProps = nullptr;
    FruitKegProperties* fruitProps = nullptr;
    LeafKegProperties* leafProps = nullptr;
    // Custom values, must match PARSE_V2 syntax
    keg::Value ui32v2Val = keg::Value::basic(0, keg::BasicType::UI32_V2);
    keg::Value ui32Val = keg::Value::basic(0, keg::BasicType::UI32);
    keg::Value i32v2Val = keg::Value::basic(0, keg::BasicType::I32_V2);
    keg::Value i32Val = keg::Value::basic(0, keg::BasicType::I32);
    keg::Value f32v2Val = keg::Value::basic(0, keg::BasicType::F32_V2);
    keg::Value f32Val = keg::Value::basic(0, keg::BasicType::F32);
    keg::Value stringVal = keg::Value::basic(0, keg::BasicType::STRING);
    keg::Value leafTypeVal = keg::Value::custom(0, "TreeLeafType", true);

    /************************************************************************/
    /* The following code is ugly because it must be custom parsed with     */
    /* PARSE_V2. It is *IMPERATIVE* that any added properties be set in     */
    /* SoaEngine::initVoxelGen as well.                                     */
    /************************************************************************/

    // Parses fruit field
    auto fruitParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        if (key == "id") {
            keg::evalData((ui8*)&fruitProps->flora, &stringVal, value, context);
        } else if (key == "max") {
            PARSE_V2(f32, fruitProps->chance);
        }
    });

    // Parses leaf field
    auto leafParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        // TODO(Ben): Other leaf types
        if (key == "type") {
            keg::evalData((ui8*)&leafProps->type, &leafTypeVal, value, context);
        } else if (key == "width") {
            PARSE_V2(ui32, leafProps->cluster.width);
        } else if (key == "height") {
            PARSE_V2(ui32, leafProps->cluster.height);
        } else if (key == "block") {
            keg::evalData((ui8*)&leafProps->clusterBlock, &stringVal, value, context);
        } else if (key == "fruit") {
            fruitProps = &leafProps->fruitProps;
            context.reader.forAllInMap(value, fruitParser);
        }
    });

    // Parses branch field
    auto branchParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        if (key == "coreWidth") {
            PARSE_V2(ui32, trunkProps->branchProps.coreWidth);
        } else if (key == "barkWidth") {
            PARSE_V2(ui32, trunkProps->branchProps.barkWidth);
        } else if (key == "branchChance") {
            PARSE_V2(f32, trunkProps->branchProps.branchChance);
        } else if (key == "coreBlock") {
            keg::evalData((ui8*)&trunkProps->coreBlock, &stringVal, value, context);
        } else if (key == "barkBlock") {
            keg::evalData((ui8*)&trunkProps->barkBlock, &stringVal, value, context);
        } else if (key == "fruit") {
            fruitProps = &trunkProps->branchProps.fruitProps;
            context.reader.forAllInMap(value, fruitParser);
        } else if (key == "leaves") {
            leafProps = &trunkProps->branchProps.leafProps;
            context.reader.forAllInMap(value, leafParser);
        }
    });

    // Parses slope field
    auto slopeParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        if (key == "min") {
            PARSE_V2(i32, trunkProps->slope[0]);
        } else if (key == "max") {
            PARSE_V2(i32, trunkProps->slope[1]);
        }
    });

    // Parses fourth level
    auto trunkDataParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        if (key == "loc") {
            keg::evalData((ui8*)&trunkProps->loc, &f32Val, value, context);
        } else if (key == "coreWidth") {
            PARSE_V2(ui32, trunkProps->coreWidth);
        } else if (key == "barkWidth") {
            PARSE_V2(ui32, trunkProps->barkWidth);
        } else if (key == "branchChance") {
            PARSE_V2(f32, trunkProps->branchChance);
        } else if (key == "slope") {
            context.reader.forAllInMap(value, slopeParser);
        } else if (key == "coreBlock") {
            keg::evalData((ui8*)&trunkProps->coreBlock, &stringVal, value, context);
        } else if (key == "barkBlock") {
            keg::evalData((ui8*)&trunkProps->barkBlock, &stringVal, value, context);
        } else if (key == "fruit") {
            fruitProps = &trunkProps->fruitProps;
            context.reader.forAllInMap(value, fruitParser);
        } else if (key == "branches") {
            context.reader.forAllInMap(value, branchParser);
        } else if (key == "leaves") {
            leafProps = &trunkProps->leafProps;
            context.reader.forAllInMap(value, leafParser);
        }
    });

    // Parses third level
    auto trunkParser = makeFunctor<Sender, size_t, keg::Node>([&](Sender, size_t size, keg::Node value) {
        treeProps->trunkProps.emplace_back();
        // Get our handle
        trunkProps = &treeProps->trunkProps.back();
        context.reader.forAllInMap(value, trunkDataParser);
    });

    // Parses second level
    auto treeParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        if (key == "height") {
            PARSE_V2(ui32, treeProps->heightRange);
        } else if (key == "trunk") {
            context.reader.forAllInSequence(value, trunkParser);
        }
    });

    // Parses top level
    auto baseParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        genData->blockInfo.trees.emplace_back();
        treeProps = &genData->blockInfo.trees.back();
        treeProps->id = key;
        context.reader.forAllInMap(value, treeParser);
    });
    context.reader.forAllInMap(node, baseParser);
    delete fruitParser;
    delete leafParser;
    delete branchParser;
    delete slopeParser;
    delete trunkParser;
    delete trunkDataParser;
    delete treeParser;
    delete baseParser;
    context.reader.dispose();
}
#undef PARSE_V2

void PlanetGenLoader::loadBiomes(const nString& filePath, PlanetGenData* genData) {
    // Read in the file
    nString data;
    m_iom->readFileToString(filePath.c_str(), data);

    // Get the read context and error check
    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to load " + filePath;
        context.reader.dispose();
        return;
    }

    // Lookup Maps
    std::map<BiomeColorCode, Biome*> m_baseBiomeLookupMap; ///< To lookup biomes via color code
    BiomeColorCode colorCodes[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH];

    std::vector<BiomeKegProperties> baseBiomes;

    // Load yaml data
    int i = 0;
    auto baseParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        // Parse based on type
        if (key == "baseLookupMap") {
            vpath texPath;
            m_iom->resolvePath(keg::convert<nString>(value), texPath);
            vg::ScopedBitmapResource rs = vg::ImageIO().load(texPath.getString(), vg::ImageIOFormat::RGB_UI8, true);
            if (!rs.data) {
                pError("Failed to load " + keg::convert<nString>(value));
            }
            if (rs.width != BIOME_MAP_WIDTH || rs.height != BIOME_MAP_WIDTH) {
                pError("loadBiomes() error: width and height of " + keg::convert<nString>(value) +" must be " + std::to_string(BIOME_MAP_WIDTH));
            }

            for (int i = 0; i < BIOME_MAP_WIDTH * BIOME_MAP_WIDTH; i++) {
                ui8v3& color = rs.bytesUI8v3[i];
                BiomeColorCode colorCode = ((ui32)color.r << 16) | ((ui32)color.g << 8) | (ui32)color.b;
                colorCodes[i / BIOME_MAP_WIDTH][i % BIOME_MAP_WIDTH] = colorCode;
            }
        } else { // It is a base biome
            baseBiomes.emplace_back();
            BiomeKegProperties& props = baseBiomes.back();
            props.id = key;
            // Parse it
            keg::Error error = keg::parse((ui8*)&props, value, context, &KEG_GLOBAL_TYPE(BiomeKegProperties));
            if (error != keg::Error::NONE) {
                fprintf(stderr, "Keg error %d in loadBiomes()\n", (int)error);
                return;
            }
        }
    });
    context.reader.forAllInMap(node, baseParser);
    delete baseParser;
    context.reader.dispose();

    // Get number of biomes
    ui32 numBiomes = 0;
    for (auto& kp : baseBiomes) {
        numBiomes += recursiveCountBiomes(kp);
    }
    // Set biome storage
    genData->biomes.resize(numBiomes);
    
    ui32 biomeCounter = 0;
    for (size_t i = 0; i < baseBiomes.size(); i++) {
        auto& kp = baseBiomes[i];
      
        // Get the biome
        Biome& biome = genData->biomes[biomeCounter++];
       
        // Get the color code and add to map
        BiomeColorCode colorCode = ((ui32)kp.mapColor.r << 16) | ((ui32)kp.mapColor.g << 8) | (ui32)kp.mapColor.b;
        m_baseBiomeLookupMap[colorCode] = &biome;

        // Copy all the data over
        recursiveInitBiomes(biome, kp, biomeCounter, genData);
    }
    assert(biomeCounter == genData->biomes.size());

    // Set base biomes
    memset(genData->baseBiomeLookup, 0, sizeof(genData->baseBiomeLookup));
    for (int y = 0; y < BIOME_MAP_WIDTH; y++) {
        for (int x = 0; x < BIOME_MAP_WIDTH; x++) {
            auto& it = m_baseBiomeLookupMap.find(colorCodes[y][x]);
            if (it != m_baseBiomeLookupMap.end()) {
                genData->baseBiomeLookup[y][x] = it->second;
            }
        }
    }
    // Blur base biome map for transition smoothing
    std::vector<std::set<BiomeInfluence>> outMap;
    blurBaseBiomeMap(genData->baseBiomeLookup, outMap);
    // Convert to influence map
    for (int y = 0; y < BIOME_MAP_WIDTH; y++) {
        for (int x = 0; x < BIOME_MAP_WIDTH; x++) {
            genData->baseBiomeInfluenceMap[y][x].resize(outMap[y * BIOME_MAP_WIDTH + x].size());
            int i = 0;
            for (auto& b : outMap[y * BIOME_MAP_WIDTH + x]) {
                genData->baseBiomeInfluenceMap[y][x][i++] = b;
            }
        }
    }
}

void PlanetGenLoader::parseTerrainFuncs(NoiseBase* terrainFuncs, keg::ReadContext& context, keg::Node node) {
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to parse node";
        return;
    }

    keg::Error error = keg::parse((ui8*)terrainFuncs, node, context, &KEG_GLOBAL_TYPE(NoiseBase));
    if (error != keg::Error::NONE) {
        fprintf(stderr, "Keg error %d in parseTerrainFuncs()\n", (int)error);
        return;
    }
}

void PlanetGenLoader::parseLiquidColor(keg::ReadContext& context, keg::Node node, PlanetGenData* genData) {
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to parse node";
        return;
    }

    LiquidColorKegProperties kegProps;

    keg::Error error;
    error = keg::parse((ui8*)&kegProps, node, context, &KEG_GLOBAL_TYPE(LiquidColorKegProperties));
    if (error != keg::Error::NONE) {
        fprintf(stderr, "Keg error %d in parseLiquidColor()\n", (int)error);
        return;
    }

    if (kegProps.colorPath.size()) {
        if (m_glRpc) {
            vcore::RPC rpc;
            rpc.data.f = makeFunctor<Sender, void*>([&](Sender s, void* userData) {
                //m_textureCache.freeTexture(kegProps.colorPath);
                //genData->liquidColorMap = m_textureCache.addTexture(kegProps.colorPath,
                //                                                    genData->liquidColorPixels,
                //                                                    vg::ImageIOFormat::RGB_UI8,
                //                                                    vg::TextureTarget::TEXTURE_2D,
                //                                                    &vg::SamplerState::LINEAR_CLAMP,
                //                                                    vg::TextureInternalFormat::RGB8,
                //                                                    vg::TextureFormat::RGB, true);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            //m_textureCache.freeTexture(kegProps.colorPath);
            //genData->liquidColorMap = m_textureCache.addTexture(kegProps.colorPath,
            //                                                    genData->liquidColorPixels,
            //                                                    vg::ImageIOFormat::RGB_UI8,
            //                                                    vg::TextureTarget::TEXTURE_2D,
            //                                                    &vg::SamplerState::LINEAR_CLAMP,
            //                                                    vg::TextureInternalFormat::RGB8,
            //                                                    vg::TextureFormat::RGB, true);
        }
        // Turn into a color map
        if (genData->liquidColorMap.id == 0) {
            vg::ImageIO::free(genData->liquidColorPixels);
        }
    }
    if (kegProps.texturePath.size()) {
        // Handle RPC for texture upload
        if (m_glRpc) {
            vcore::RPC rpc;
            rpc.data.f = makeFunctor<Sender, void*>([&](Sender s, void* userData) {
                //genData->liquidTexture = m_textureCache.addTexture(kegProps.texturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            //genData->liquidTexture = m_textureCache.addTexture(kegProps.texturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        }
    }
    genData->liquidFreezeTemp = kegProps.freezeTemp;
    genData->liquidDepthScale = kegProps.depthScale;
    genData->liquidTint = kegProps.tint;
}

void PlanetGenLoader::parseTerrainColor(keg::ReadContext& context, keg::Node node, PlanetGenData* genData) {
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to parse node";
        return;
    }

    TerrainColorKegProperties kegProps;

    keg::Error error;
    error = keg::parse((ui8*)&kegProps, node, context, &KEG_GLOBAL_TYPE(TerrainColorKegProperties));
    if (error != keg::Error::NONE) {
        fprintf(stderr, "Keg error %d in parseTerrainColor()\n", (int)error);
        return;
    }

    if (kegProps.colorPath.size()) {
        // Handle RPC for texture upload
        if (m_glRpc) {
            vcore::RPC rpc;
            rpc.data.f = makeFunctor<Sender, void*>([&](Sender s, void* userData) {
                //m_textureCache.freeTexture(kegProps.colorPath);
                //genData->terrainColorMap = m_textureCache.addTexture(kegProps.colorPath,
                //                                                     genData->terrainColorPixels,
                //                                                     vg::ImageIOFormat::RGB_UI8,
                //                                                     vg::TextureTarget::TEXTURE_2D,
                //                                                     &vg::SamplerState::LINEAR_CLAMP,
                //                                                     vg::TextureInternalFormat::RGB8,
                //                                                     vg::TextureFormat::RGB, true);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            //m_textureCache.freeTexture(kegProps.colorPath);
            //genData->terrainColorMap = m_textureCache.addTexture(kegProps.colorPath,
            //                                                     genData->terrainColorPixels,
            //                                                     vg::ImageIOFormat::RGB_UI8,
            //                                                     vg::TextureTarget::TEXTURE_2D,
            //                                                     &vg::SamplerState::LINEAR_CLAMP,
            //                                                     vg::TextureInternalFormat::RGB8,
            //                                                     vg::TextureFormat::RGB, true);
        }
        // Turn into a color map
        if (genData->terrainColorMap.id == 0) {
            vg::ImageIO::free(genData->terrainColorPixels);
        }
    }
    // TODO(Ben): stop being lazy and copy pasting
    if (kegProps.grassTexturePath.size()) {
        // Handle RPC for texture upload
        if (m_glRpc) {
            vcore::RPC rpc;
            rpc.data.f = makeFunctor<Sender, void*>([&](Sender s, void* userData) {
                //genData->grassTexture = m_textureCache.addTexture(kegProps.grassTexturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            //genData->grassTexture = m_textureCache.addTexture(kegProps.grassTexturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        }
    }
    if (kegProps.rockTexturePath.size()) {
        // Handle RPC for texture upload
        if (m_glRpc) {
            vcore::RPC rpc;
            rpc.data.f = makeFunctor<Sender, void*>([&](Sender s, void* userData) {
                //genData->rockTexture = m_textureCache.addTexture(kegProps.rockTexturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            //genData->rockTexture = m_textureCache.addTexture(kegProps.rockTexturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        }
    }
    genData->terrainTint = kegProps.tint;
}

void PlanetGenLoader::parseBlockLayers(keg::ReadContext& context, keg::Node node, PlanetGenData* genData) {
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to parse node in parseBlockLayers. Should be MAP";
        return;
    }

    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& name, keg::Node value) {
        // Add a block
        genData->blockInfo.blockLayerNames.emplace_back(name);
        genData->blockLayers.emplace_back();

        BlockLayer& l = genData->blockLayers.back();
        // Load data
        keg::parse((ui8*)&l, value, context, &KEG_GLOBAL_TYPE(BlockLayer));
    });
    context.reader.forAllInMap(node, f);
    delete f;

    // Set starts for binary search application
    int start = 0;
    for (auto& l : genData->blockLayers) {
        l.start = start;
        start += l.width;
    }
}
