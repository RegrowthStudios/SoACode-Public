#include "stdafx.h"
#include "PlanetLoader.h"
#include "PlanetGenData.h"

#include <random>

#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/Events.hpp>
#include <Vorb/io/YAML.h>
#include <Vorb/RPC.h>
#include <Vorb/Timing.h>

#include "Errors.h"

typedef ui32 BiomeColorCode;

struct BiomeKegProperties {
    Array<BiomeKegProperties> children;
    Array<BlockLayer> blockLayers;
    BiomeAxisType xAxis = BiomeAxisType::NOISE;
    BiomeAxisType yAxis = BiomeAxisType::NOISE;
    BiomeID id = "";
    ColorRGB8 mapColor = ColorRGB8(255, 255, 255);
    NoiseBase biomeMapNoise; ///< For sub biome determination
    NoiseBase terrainNoise; ///< Modifies terrain directly
    NoiseBase xNoise;
    NoiseBase yNoise;
    f32v2 heightScale = f32v2(0.0f, 200.0f); ///< Scales height for BIOME_AXIS_TYPE::HEIGHT
    nString displayName = "Unknown";
    nString childColorMap = "";
};
KEG_TYPE_DECL(BiomeKegProperties);
KEG_TYPE_DEF_SAME_NAME(BiomeKegProperties, kt) {
    using namespace keg;
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, id, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, displayName, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, childColorMap, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, mapColor, UI8_V3);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeKegProperties, heightScale, F32_V2);
    kt.addValue("blockLayers", Value::array(offsetof(BiomeKegProperties, blockLayers), Value::custom(0, "BlockLayer")));
    kt.addValue("xAxis", keg::Value::custom(offsetof(BiomeKegProperties, xAxis), "BiomeAxisType", true));
    kt.addValue("yAxis", keg::Value::custom(offsetof(BiomeKegProperties, yAxis), "BiomeAxisType", true));
    kt.addValue("xNoise", keg::Value::custom(offsetof(BiomeKegProperties, xNoise), "NoiseBase", false));
    kt.addValue("yNoise", keg::Value::custom(offsetof(BiomeKegProperties, yNoise), "NoiseBase", false));
    kt.addValue("terrainNoise", Value::custom(offsetof(BiomeKegProperties, terrainNoise), "NoiseBase", false));
    kt.addValue("biomeMapNoise", Value::custom(offsetof(BiomeKegProperties, biomeMapNoise), "NoiseBase", false));
    kt.addValue("children", Value::array(offsetof(BiomeKegProperties, children), Value::custom(0, "BiomeKegProperties", false)));
}

PlanetLoader::PlanetLoader(vio::IOManager* ioManager) :
    m_iom(ioManager),
    m_textureCache(m_iom) {
    // Empty
}

PlanetLoader::~PlanetLoader() {
}

PlanetGenData* PlanetLoader::loadPlanet(const nString& filePath, vcore::RPCManager* glrpc /* = nullptr */) {
    m_glRpc = glrpc;
    nString data;
    m_iom->readFileToString(filePath.c_str(), data);

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to load " + filePath;
        context.reader.dispose();
        return false;
    }

    PlanetGenData* genData = new PlanetGenData;
    genData->filePath = filePath;

    nString biomePath = "";
    bool didLoadBiomes = false;

    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& type, keg::Node value) {
        // Parse based on type
        if (type == "biomes") {
            didLoadBiomes = true;
            loadBiomes(keg::convert<nString>(value), genData);
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

    if (!didLoadBiomes) {
        // Set default biome
        for (int y = 0; y < BIOME_MAP_WIDTH; y++) {
            for (int x = 0; x < BIOME_MAP_WIDTH; x++) {
                genData->baseBiomeLookup[y][x] = &DEFAULT_BIOME;
            }
        }
    }
    return genData;
}

PlanetGenData* PlanetLoader::getDefaultGenData(vcore::RPCManager* glrpc /* = nullptr */) {
    // Lazily construct default data
    if (!m_defaultGenData) {
        // Allocate data
        m_defaultGenData = new PlanetGenData;
    }
    return m_defaultGenData;
}

PlanetGenData* PlanetLoader::getRandomGenData(f32 radius, vcore::RPCManager* glrpc /* = nullptr */) {
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
            genData->grassTexture = m_textureCache.addTexture("_shared/terrain_b.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            genData->rockTexture = m_textureCache.addTexture("_shared/terrain_a.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            genData->liquidTexture = m_textureCache.addTexture("_shared/water_a.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        });
        glrpc->invoke(&rpc, true);
    } else {
        genData->grassTexture = m_textureCache.addTexture("_shared/terrain_b.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        genData->rockTexture = m_textureCache.addTexture("_shared/terrain_a.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        genData->liquidTexture = m_textureCache.addTexture("_shared/water_a.png", vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
    }

    // Set default biome
    for (int y = 0; y < BIOME_MAP_WIDTH; y++) {
        for (int x = 0; x < BIOME_MAP_WIDTH; x++) {
            genData->baseBiomeLookup[y][x] = &DEFAULT_BIOME;
        }
    }

    return genData;
}

AtmosphereKegProperties PlanetLoader::getRandomAtmosphere() {
    static std::mt19937 generator(2636);
    static std::uniform_real_distribution<f32> randomWav(0.4f, 0.8f);
    AtmosphereKegProperties props;
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
                         BiomeKegProperties& kp,
                         ui32& biomeCounter,
                         PlanetGenData* genData,
                         std::map<BiomeColorCode, Biome*>& biomeLookup,
                         vio::IOManager* iom) {
    // Get the color code and add to map
    BiomeColorCode colorCode = ((ui32)kp.mapColor.r << 16) | ((ui32)kp.mapColor.g << 8) | (ui32)kp.mapColor.b;
    biomeLookup[colorCode] = &biome;

    // Copy all biome data
    biome.id = kp.id;
    biome.blockLayers.resize(kp.blockLayers.size());
    for (size_t i = 0; i < kp.blockLayers.size(); i++) {
        biome.blockLayers[i] = kp.blockLayers[i];
    }
    biome.displayName = kp.displayName;
    biome.mapColor = kp.mapColor;
    biome.heightScale = kp.heightScale;
    biome.terrainNoise = kp.terrainNoise;
    biome.biomeMapNoise = kp.biomeMapNoise;
    biome.xNoise = kp.xNoise;
    biome.yNoise = kp.yNoise;
    biome.axisTypes[0] = kp.xAxis;
    biome.axisTypes[1] = kp.yAxis;

    // Recurse children
    std::map<BiomeColorCode, Biome*> nextBiomeLookup;
    for (size_t i = 0; i < kp.children.size(); i++) {
        Biome& nextBiome = genData->biomes[biomeCounter++];
        recursiveInitBiomes(nextBiome, kp.children[i], biomeCounter, genData, nextBiomeLookup, iom);
    }

    // Load and parse child lookup map
    if (kp.childColorMap.size()) {
        vpath texPath;
        iom->resolvePath(kp.childColorMap, texPath);
        vg::ScopedBitmapResource rs = vg::ImageIO().load(texPath.getString(), vg::ImageIOFormat::RGB_UI8, true);
        if (!rs.data) {
            fprintf(stderr, "Warning: Failed to load %s\n", kp.childColorMap.c_str());
            return;
        }
        // Using a set now for the blurring pass for fast lookups to eliminate duplicate
        std::vector<BiomeInfluence> biomeMap;
        // Check for 1D biome map
        if (rs.width == BIOME_MAP_WIDTH && rs.height == 1) {
            biomeMap.resize(BIOME_MAP_WIDTH);
        } else {
            // Error check
            if (rs.width != BIOME_MAP_WIDTH || rs.height != BIOME_MAP_WIDTH) {
                pError("loadBiomes() error: width and height of " + kp.childColorMap + " must be " + std::to_string(BIOME_MAP_WIDTH));
            }
            biomeMap.resize(BIOME_MAP_WIDTH * BIOME_MAP_WIDTH);
        }
        // Fill biome map
        for (size_t i = 0; i < biomeMap.size(); i++) {
            ui8v3& color = rs.bytesUI8v3[i];
            BiomeColorCode code = ((ui32)color.r << 16) | ((ui32)color.g << 8) | (ui32)color.b;
            auto& it = nextBiomeLookup.find(code);
            if (it != nextBiomeLookup.end()) {
                biomeMap[i].b = it->second;
                biomeMap[i].weight = 1.0f;
            } else {
                biomeMap[i].b = nullptr;
            }
        }

        biome.biomeMap.resize(biomeMap.size());
        for (size_t i = 0; i < biomeMap.size(); i++) {
            if (biomeMap[i].b) biome.biomeMap[i].emplace_back(biomeMap[i]);
        }

        //std::vector<std::set<BiomeInfluence>> outMap;
        //blurBiomeMap(biomeMap, outMap);
        //// Convert to BiomeInfluenceMap
        //biome.biomeMap.resize(outMap.size());
        //for (size_t i = 0; i < outMap.size(); i++) {
        //    biome.biomeMap[i].resize(outMap[i].size());
        //    int j = 0;
        //    for (auto& b : outMap[i]) {
        //        biome.biomeMap[i][j++] = b;
        //    }
        //}

    }
}

void PlanetLoader::loadBiomes(const nString& filePath, PlanetGenData* genData) {
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
       
        // Copy all the data over
        recursiveInitBiomes(biome, kp, biomeCounter, genData, m_baseBiomeLookupMap, m_iom);
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

void PlanetLoader::parseTerrainFuncs(NoiseBase* terrainFuncs, keg::ReadContext& context, keg::Node node) {
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

void PlanetLoader::parseLiquidColor(keg::ReadContext& context, keg::Node node, PlanetGenData* genData) {
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
                m_textureCache.freeTexture(kegProps.colorPath);
                genData->liquidColorMap = m_textureCache.addTexture(kegProps.colorPath,
                                                                    genData->liquidColorPixels,
                                                                    vg::ImageIOFormat::RGB_UI8,
                                                                    vg::TextureTarget::TEXTURE_2D,
                                                                    &vg::SamplerState::LINEAR_CLAMP,
                                                                    vg::TextureInternalFormat::RGB8,
                                                                    vg::TextureFormat::RGB, true);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            m_textureCache.freeTexture(kegProps.colorPath);
            genData->liquidColorMap = m_textureCache.addTexture(kegProps.colorPath,
                                                                genData->liquidColorPixels,
                                                                vg::ImageIOFormat::RGB_UI8,
                                                                vg::TextureTarget::TEXTURE_2D,
                                                                &vg::SamplerState::LINEAR_CLAMP,
                                                                vg::TextureInternalFormat::RGB8,
                                                                vg::TextureFormat::RGB, true);
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
                genData->liquidTexture = m_textureCache.addTexture(kegProps.texturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            genData->liquidTexture = m_textureCache.addTexture(kegProps.texturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        }
    }
    genData->liquidFreezeTemp = kegProps.freezeTemp;
    genData->liquidDepthScale = kegProps.depthScale;
    genData->liquidTint = kegProps.tint;
}

void PlanetLoader::parseTerrainColor(keg::ReadContext& context, keg::Node node, PlanetGenData* genData) {
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
                m_textureCache.freeTexture(kegProps.colorPath);
                genData->terrainColorMap = m_textureCache.addTexture(kegProps.colorPath,
                                                                     genData->terrainColorPixels,
                                                                     vg::ImageIOFormat::RGB_UI8,
                                                                     vg::TextureTarget::TEXTURE_2D,
                                                                     &vg::SamplerState::LINEAR_CLAMP,
                                                                     vg::TextureInternalFormat::RGB8,
                                                                     vg::TextureFormat::RGB, true);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            m_textureCache.freeTexture(kegProps.colorPath);
            genData->terrainColorMap = m_textureCache.addTexture(kegProps.colorPath,
                                                                 genData->terrainColorPixels,
                                                                 vg::ImageIOFormat::RGB_UI8,
                                                                 vg::TextureTarget::TEXTURE_2D,
                                                                 &vg::SamplerState::LINEAR_CLAMP,
                                                                 vg::TextureInternalFormat::RGB8,
                                                                 vg::TextureFormat::RGB, true);
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
                genData->grassTexture = m_textureCache.addTexture(kegProps.grassTexturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            genData->grassTexture = m_textureCache.addTexture(kegProps.grassTexturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        }
    }
    if (kegProps.rockTexturePath.size()) {
        // Handle RPC for texture upload
        if (m_glRpc) {
            vcore::RPC rpc;
            rpc.data.f = makeFunctor<Sender, void*>([&](Sender s, void* userData) {
                genData->rockTexture = m_textureCache.addTexture(kegProps.rockTexturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
            });
            m_glRpc->invoke(&rpc, true);
        } else {
            genData->rockTexture = m_textureCache.addTexture(kegProps.rockTexturePath, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
        }
    }
    genData->terrainTint = kegProps.tint;
}

void PlanetLoader::parseBlockLayers(keg::ReadContext& context, keg::Node node, PlanetGenData* genData) {
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
