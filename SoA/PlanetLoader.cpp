#include "stdafx.h"
#include "PlanetLoader.h"
#include "PlanetData.h"
#include "BlockPack.h"

#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/io/IOManager.h>

#include "Errors.h"

PlanetLoader::PlanetLoader(vio::IOManager* ioManager) :
    m_iom(ioManager),
    m_textureCache(m_iom) {
    // Empty
}

PlanetLoader::~PlanetLoader() {
}

PlanetGenData* PlanetLoader::loadPlanet(const nString& filePath, vcore::RPCManager* glrpc /* = nullptr */) {
    nString data;
    m_iom->readFileToString(filePath.c_str(), data);

    keg::YAMLReader reader;
    reader.init(data.c_str());
    keg::Node node = reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to load " + filePath;
        reader.dispose();
        return false;
    }

    PlanetGenData* genData = new PlanetGenData;

    TerrainFuncs baseTerrainFuncs;
    TerrainFuncs tempTerrainFuncs;
    TerrainFuncs humTerrainFuncs;
    nString biomePath = "";

    auto f = createDelegate<const nString&, keg::Node>([&] (Sender, const nString& type, keg::Node value) {
        // Parse based on type
        if (type == "biomes") {
            loadBiomes(keg::convert<nString>(value), genData);
        } else if (type == "terrainColor") {
            parseTerrainColor(reader, value, genData);
        } else if (type == "liquidColor") {
            parseLiquidColor(reader, value, genData);
        } else if (type == "tempLatitudeFalloff") {
            genData->tempLatitudeFalloff = keg::convert<f32>(value);
        } else if (type == "humLatitudeFalloff") {
            genData->humLatitudeFalloff = keg::convert<f32>(value);
        } else if (type == "baseHeight") {
            parseTerrainFuncs(&baseTerrainFuncs, reader, value);
        } else if (type == "temperature") {
            parseTerrainFuncs(&tempTerrainFuncs, reader, value);
        } else if (type == "humidity") {
            parseTerrainFuncs(&humTerrainFuncs, reader, value);
        } else if (type == "blockLayers") {
            parseBlockLayers(reader, value, genData);
        } else if (type == "liquidBlock") {
            genData->liquidBlock = Blocks[keg::convert<nString>(value)].ID;
        } else if (type == "surfaceBlock") {
            genData->surfaceBlock = Blocks[keg::convert<nString>(value)].ID;
        }
    });
    reader.forAllInMap(node, f);
    reader.dispose();
    
    // Generate the program
    vg::GLProgram* program = m_shaderGenerator.generateProgram(genData,
                                               baseTerrainFuncs,
                                               tempTerrainFuncs,
                                               humTerrainFuncs,
                                               glrpc);

    if (program != nullptr) {
        genData->program = program;
        return genData;
    } 
    delete genData;
    return nullptr;
}

PlanetGenData* PlanetLoader::getDefaultGenData(vcore::RPCManager* glrpc /* = nullptr */) {
    // Lazily construct default data
    if (!m_defaultGenData) {
        // Allocate data
        m_defaultGenData = new PlanetGenData;

        m_defaultGenData->program = m_shaderGenerator.getDefaultProgram(glrpc);

    }
    return m_defaultGenData;
}

void PlanetLoader::loadBiomes(const nString& filePath, PlanetGenData* genData) {
    nString data;
    m_iom->readFileToString(filePath.c_str(), data);

    keg::YAMLReader reader;
    reader.init(data.c_str());
    keg::Node node = reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to load " + filePath;
        reader.dispose();
        return;
    }

    baseBiomeLookupTexture.resize(LOOKUP_TEXTURE_SIZE, 0);

    Keg::Error error;

    // Load yaml data
    auto f = createDelegate<const nString&, keg::Node>([&] (Sender, const nString& type, keg::Node value) {
        // Parse based on type
        if (type == "baseLookupMap") {
            std::vector<ui8> data;
            ui32 rWidth, rHeight;
            vpath texPath; m_iom->resolvePath(keg::convert<nString>(value), texPath);
            if (!vio::ImageIO().loadPng(texPath.getString(), data, rWidth, rHeight)) {
                pError("Failed to load " + keg::convert<nString>(value));
            }
            if (rWidth != 256 || rHeight != 256) {
                pError("loadBiomes() error: width and height of " + keg::convert<nString>(value) + " must be 256");
            }

            for (int i = 0; i < LOOKUP_TEXTURE_SIZE; i++) {
                int index = i * 4;
                ui32 colorCode = ((ui32)data[index] << 16) | ((ui32)data[index + 1] << 8) | (ui32)data[index + 2];
                addBiomePixel(colorCode, i);
            }
        } else {
            // It is a biome
            genData->biomes.emplace_back();
            
            error = Keg::parse((ui8*)&genData->biomes.back(), value, reader, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Biome));
            if (error != Keg::Error::NONE) {
                fprintf(stderr, "Keg error %d in loadBiomes()\n", (int)error);
                return;
            }
        }
    });
    reader.forAllInMap(node, f);
    delete f;
    reader.dispose();

    if (m_biomeLookupMap.size()) {

        // Generate base biome lookup texture
        genData->baseBiomeLookupTexture = vg::GpuMemory::uploadTexture(baseBiomeLookupTexture.data(),
                                                                   LOOKUP_TEXTURE_WIDTH, LOOKUP_TEXTURE_WIDTH,
                                                                   &SamplerState::POINT_CLAMP,
                                                                   vg::TextureInternalFormat::R8,
                                                                   vg::TextureFormat::RED, 0);
        // Generate array textures
        glGenTextures(1, &genData->biomeArrayTexture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, genData->biomeArrayTexture);
        //Allocate the storage.
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, LOOKUP_TEXTURE_WIDTH, LOOKUP_TEXTURE_WIDTH, m_biomeLookupMap.size(), 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        // Set up base lookup textures
        for (auto& it : m_biomeLookupMap) {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, it.second.index, LOOKUP_TEXTURE_WIDTH, LOOKUP_TEXTURE_WIDTH,
                            1, GL_RED, GL_UNSIGNED_BYTE, it.second.data.data());
        }
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    // Free memory
    std::map<ui32, BiomeLookupTexture>().swap(m_biomeLookupMap);
    std::vector<ui8>().swap(baseBiomeLookupTexture);
}

void PlanetLoader::addBiomePixel(ui32 colorCode, int index) {
    auto& it = m_biomeLookupMap.find(colorCode);
    if (it == m_biomeLookupMap.end()) {
        BiomeLookupTexture& tex = m_biomeLookupMap[colorCode];
        tex.index = m_biomeCount;
        baseBiomeLookupTexture[index] = m_biomeCount;
        tex.data[index] = 255;
        m_biomeCount++;
    } else {
        baseBiomeLookupTexture[index] = m_biomeCount - 1;
        it->second.data[index] = 255;
    }
}

void PlanetLoader::parseTerrainFuncs(TerrainFuncs* terrainFuncs, keg::YAMLReader& reader, keg::Node node) {
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to parse node";
        return;
    }

    Keg::Error error;

    auto f = createDelegate<const nString&, keg::Node>([&] (Sender, const nString& type, keg::Node value) {
        if (type == "base") {
            terrainFuncs->baseHeight += keg::convert<f32>(value);
            return;
        }

        terrainFuncs->funcs.push_back(TerrainFuncKegProperties());
        if (type == "noise") {
            terrainFuncs->funcs.back().func = TerrainFunction::NOISE;
        } else if (type == "ridgedNoise") {
            terrainFuncs->funcs.back().func = TerrainFunction::RIDGED_NOISE;
        }

        error = Keg::parse((ui8*)&terrainFuncs->funcs.back(), value, reader, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(TerrainFuncKegProperties));
        if (error != Keg::Error::NONE) {
            fprintf(stderr, "Keg error %d in parseTerrainFuncs()\n", (int)error); 
            return;
        }
    });
    reader.forAllInMap(node, f);
    delete f;
}

void PlanetLoader::parseLiquidColor(keg::YAMLReader& reader, keg::Node node, PlanetGenData* genData) {
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to parse node";
        return;
    }

    LiquidColorKegProperties kegProps;

    Keg::Error error;
    error = Keg::parse((ui8*)&kegProps, node, reader, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(LiquidColorKegProperties));
    if (error != Keg::Error::NONE) {
        fprintf(stderr, "Keg error %d in parseLiquidColor()\n", (int)error);
        return;
    }

    if (kegProps.colorPath.size()) {
        genData->liquidColorMap = m_textureCache.addTexture(kegProps.colorPath, &SamplerState::LINEAR_CLAMP);
    }
    if (kegProps.texturePath.size()) {
        genData->liquidTexture = m_textureCache.addTexture(kegProps.texturePath, &SamplerState::LINEAR_WRAP_MIPMAP);
    }
    genData->liquidFreezeTemp = kegProps.freezeTemp;
    genData->liquidDepthScale = kegProps.depthScale;
    genData->liquidTint = kegProps.tint;
}

void PlanetLoader::parseTerrainColor(keg::YAMLReader& reader, keg::Node node, PlanetGenData* genData) {
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to parse node";
        return;
    }

    TerrainColorKegProperties kegProps;

    Keg::Error error;
    error = Keg::parse((ui8*)&kegProps, node, reader, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(TerrainColorKegProperties));
    if (error != Keg::Error::NONE) {
        fprintf(stderr, "Keg error %d in parseLiquidColor()\n", (int)error);
        return;
    }

    if (kegProps.colorPath.size()) {
        genData->terrainColorMap = m_textureCache.addTexture(kegProps.colorPath, &SamplerState::LINEAR_CLAMP);
    }
    if (kegProps.texturePath.size()) {
        genData->terrainTexture = m_textureCache.addTexture(kegProps.texturePath, &SamplerState::LINEAR_WRAP_MIPMAP);
    }
    genData->terrainTint = kegProps.tint;
}

void PlanetLoader::parseBlockLayers(keg::YAMLReader& reader, keg::Node node, PlanetGenData* genData) {
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to parse node in parseBlockLayers. Should be MAP";
        return;
    }

    auto f = createDelegate<const nString&, keg::Node>([&](Sender, const nString& name, keg::Node value) {
        // Add a block
        genData->blockLayers.emplace_back();
        BlockLayer& l = genData->blockLayers.back();

        // Set name to key
        l.block = Blocks[name].ID;

        // Load data
        Keg::parse((ui8*)&l, value, reader, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(BlockLayer));
    });
    reader.forAllInMap(node, f);
    delete f;

    // Set starts for binary search application
    int start = 0;
    for (auto& l : genData->blockLayers) {
        l.start = start;
        start += l.width;
    }
}
