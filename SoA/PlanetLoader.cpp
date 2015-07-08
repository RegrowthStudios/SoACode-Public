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

#include "Errors.h"

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

    typedef ui32 BiomeColorCode;
    // Lookup Maps
    std::map<BiomeColorCode, Biome*> m_baseBiomeLookupMap; ///< To lookup biomes via color code
    BiomeColorCode colorCodes[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH];

    // Load yaml data
    int i = 0;
    auto baseParser = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        // Parse based on type
        if (key == "baseLookupMap") {
            vpath texPath;
            m_iom->resolvePath(keg::convert<nString>(value), texPath);
            vg::BitmapResource bitmap = vg::ImageIO().load(texPath.getString(), vg::ImageIOFormat::RGB_UI8);
            if (!bitmap.data) {
                pError("Failed to load " + keg::convert<nString>(value));
            }
            if (bitmap.width != BIOME_MAP_WIDTH || bitmap.height != BIOME_MAP_WIDTH) {
                pError("loadBiomes() error: width and height of " + keg::convert<nString>(value) +" must be " + std::to_string(BIOME_MAP_WIDTH));
            }

            for (int i = 0; i < BIOME_MAP_WIDTH * BIOME_MAP_WIDTH; i++) {
                ui8v3& color = bitmap.bytesUI8v3[i];
                BiomeColorCode colorCode = ((ui32)color.r << 16) | ((ui32)color.g << 8) | (ui32)color.b;
                colorCodes[i / BIOME_MAP_WIDTH][i % BIOME_MAP_WIDTH] = colorCode;
            }
        } else { // It is a base biome
            Biome* biome = new Biome;
            genData->biomes.push_back(biome);
            biome->id = key;
            // Parse it
            keg::Error error = keg::parse((ui8*)biome, value, context, &KEG_GLOBAL_TYPE(Biome));
            if (error != keg::Error::NONE) {
                fprintf(stderr, "Keg error %d in loadBiomes()\n", (int)error);
                return;
            }
            // Get the color code
            BiomeColorCode colorCode = ((ui32)biome->mapColor.r << 16) | ((ui32)biome->mapColor.g << 8) | (ui32)biome->mapColor.b;
            m_baseBiomeLookupMap[colorCode] = biome;
        }
    });
    context.reader.forAllInMap(node, baseParser);
    delete baseParser;
    context.reader.dispose();

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
