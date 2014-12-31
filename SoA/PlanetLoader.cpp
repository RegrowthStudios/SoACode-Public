#include "stdafx.h"
#include "PlanetLoader.h"

#include "ImageLoader.h"

#include "NoiseShaderCode.hpp"
#include "Errors.h"

#include <GpuMemory.h>
#include <IOManager.h>

enum class TerrainFunction {
    NOISE,
    RIDGED_NOISE
};
KEG_ENUM_INIT_BEGIN(TerrainFunction, TerrainFunction, e);
e->addValue("noise", TerrainFunction::RIDGED_NOISE);
e->addValue("ridgedNoise", TerrainFunction::RIDGED_NOISE);
KEG_ENUM_INIT_END

class TerrainFuncKegProperties {
public:
    TerrainFunction func;
    int octaves = 1;
    float persistence = 1.0f;
    float frequency = 1.0f;
    float low = -1.0f;
    float high = 1.0f;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(TerrainFuncKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, I32, octaves);
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, F32, persistence);
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, F32, frequency);
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, F32, low);
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, F32, high);
KEG_TYPE_INIT_END

class LiquidColorKegProperties {
public:
    nString colorPath = "";
    nString texturePath = "";
    ColorRGB8 tint = ColorRGB8(255, 255, 255);
    float depthScale = 1000.0f;
    float freezeTemp = -1.0f;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(LiquidColorKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(LiquidColorKegProperties, STRING, colorPath);
KEG_TYPE_INIT_ADD_MEMBER(LiquidColorKegProperties, STRING, texturePath);
KEG_TYPE_INIT_ADD_MEMBER(LiquidColorKegProperties, UI8_V3, tint); 
KEG_TYPE_INIT_ADD_MEMBER(LiquidColorKegProperties, F32, depthScale);
KEG_TYPE_INIT_ADD_MEMBER(LiquidColorKegProperties, F32, freezeTemp);
KEG_TYPE_INIT_END

class TerrainColorKegProperties {
public:
    nString colorPath = "";
    nString texturePath = "";
    ColorRGB8 tint = ColorRGB8(255, 255, 255);
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(TerrainColorKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(TerrainColorKegProperties, STRING, colorPath);
KEG_TYPE_INIT_ADD_MEMBER(TerrainColorKegProperties, STRING, texturePath);
KEG_TYPE_INIT_ADD_MEMBER(TerrainColorKegProperties, UI8_V3, tint);
KEG_TYPE_INIT_END

class TerrainFuncs {
public:
    std::vector<TerrainFuncKegProperties> funcs;
    float baseHeight = 0.0f;
};

PlanetLoader::PlanetLoader(IOManager* ioManager) :
    m_iom(ioManager),
    m_textureCache(m_iom) {
    // Empty
}


PlanetLoader::~PlanetLoader() {
}

PlanetGenData* PlanetLoader::loadPlanet(const nString& filePath) {
    nString data;
    m_iom->readFileToString(filePath.c_str(), data);

    YAML::Node node = YAML::Load(data.c_str());
    if (node.IsNull() || !node.IsMap()) {
        std::cout << "Failed to load " + filePath;
        return false;
    }

    PlanetGenData* genData = new PlanetGenData;

    TerrainFuncs baseTerrainFuncs;
    TerrainFuncs tempTerrainFuncs;
    TerrainFuncs humTerrainFuncs;
    nString biomePath = "";

    for (auto& kvp : node) {
        nString type = kvp.first.as<nString>();
        // Parse based on type
        if (type == "biomes") {
            loadBiomes(kvp.second.as<nString>(), genData);
        } else if (type == "terrainColor") {
            parseTerrainColor(kvp.second, genData);
        } else if (type == "liquidColor") {
            parseLiquidColor(kvp.second, genData);
        } else if (type == "tempLatitudeFalloff") {
            genData->tempLatitudeFalloff = kvp.second.as<float>();
        } else if (type == "humLatitudeFalloff") {
            genData->humLatitudeFalloff = kvp.second.as<float>();
        } else if (type == "baseHeight") {
            parseTerrainFuncs(&baseTerrainFuncs, kvp.second);
        } else if (type == "temperature") {
            parseTerrainFuncs(&tempTerrainFuncs, kvp.second);
        } else if (type == "humidity") {
            parseTerrainFuncs(&humTerrainFuncs, kvp.second);
        }
    }

    
    // Generate the program
    vg::GLProgram* program = generateProgram(genData,
                                             baseTerrainFuncs,
                                             tempTerrainFuncs,
                                             humTerrainFuncs);

    if (program != nullptr) {
        genData->program = program;
        return genData;
    } 
    delete genData;
    return nullptr;
}

PlanetGenData* PlanetLoader::getDefaultGenData() {
    // Lazily construct default data
    if (!m_defaultGenData) {
        // Allocate data
        m_defaultGenData = new PlanetGenData;

        // Build string
        nString fSource = NOISE_SRC_FRAG;
        fSource.reserve(fSource.size() + 128);
        // Use pos so uniforms don't get optimized out
        fSource += N_HEIGHT + "= pos.x*0.000001;";
        fSource += N_TEMP + "= 0;";
        fSource += N_HUM + "= 0; }";

        // Create the shader
        vg::GLProgram* program = new vg::GLProgram;
        program->init();
        program->addShader(vg::ShaderType::VERTEX_SHADER, NOISE_SRC_VERT.c_str());
        program->addShader(vg::ShaderType::FRAGMENT_SHADER, fSource.c_str());
        program->bindFragDataLocation(0, N_HEIGHT.c_str());
        program->bindFragDataLocation(1, N_TEMP.c_str());
        program->bindFragDataLocation(2, N_HUM.c_str());
        program->link();

        if (!program->getIsLinked()) {
            std::cout << fSource << std::endl;
            showMessage("Failed to generate default program");
            return nullptr;
        }

        program->initAttributes();
        program->initUniforms();

        m_defaultGenData->program = program;

    }
    return m_defaultGenData;
}

void PlanetLoader::loadBiomes(const nString& filePath, PlanetGenData* genData) {
    nString data;
    m_iom->readFileToString(filePath.c_str(), data);

    YAML::Node node = YAML::Load(data.c_str());
    if (node.IsNull() || !node.IsMap()) {
        std::cout << "Failed to load " + filePath;
        return;
    }

    baseBiomeLookupTexture.resize(LOOKUP_TEXTURE_SIZE, 0);

    Keg::Error error;

    // Load yaml data
    for (auto& kvp : node) {
        nString type = kvp.first.as<nString>();
        // Parse based on type
        if (type == "baseLookupMap") {
            std::vector<ui8> data;
            ui32 rWidth, rHeight;
            if (!vg::ImageLoader::loadPng(kvp.second.as<nString>().c_str(), data,
                rWidth, rHeight, m_iom)) {
                pError("Failed to load " + kvp.second.as<nString>());
            }
            if (rWidth != 256 || rHeight != 256) {
                pError("loadBiomes() error: width and height of " + kvp.second.as<nString>() + " must be 256");
            }

            for (int i = 0; i < LOOKUP_TEXTURE_SIZE; i++) {
                int index = i * 4;
                ui32 colorCode = ((ui32)data[index] << 16) | ((ui32)data[index + 1] << 8) | (ui32)data[index + 2];
                addBiomePixel(colorCode, i);
            }
        } else {
            // It is a biome
            genData->biomes.emplace_back();
            
            error = Keg::parse((ui8*)&genData->biomes.back(), kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Biome));
            if (error != Keg::Error::NONE) {
                fprintf(stderr, "Keg error %d in loadBiomes()\n", (int)error);
                return;
            }
        }
    }

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
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, LOOKUP_TEXTURE_WIDTH, LOOKUP_TEXTURE_WIDTH, m_biomeLookupMap.size());
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

void PlanetLoader::parseTerrainFuncs(TerrainFuncs* terrainFuncs, YAML::Node& node) {
    if (node.IsNull() || !node.IsMap()) {
        std::cout << "Failed to parse node";
        return;
    }

    Keg::Error error;

    for (auto& kvp : node) {
        nString type = kvp.first.as<nString>();
        if (type == "base") {
            terrainFuncs->baseHeight += kvp.second.as<float>();
            continue;
        }

        terrainFuncs->funcs.push_back(TerrainFuncKegProperties());
        if (type == "noise") {
            terrainFuncs->funcs.back().func = TerrainFunction::NOISE;
        } else if (type == "ridgedNoise") {
            terrainFuncs->funcs.back().func = TerrainFunction::RIDGED_NOISE;
        }

        error = Keg::parse((ui8*)&terrainFuncs->funcs.back(), kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(TerrainFuncKegProperties));
        if (error != Keg::Error::NONE) {
            fprintf(stderr, "Keg error %d in parseTerrainFuncs()\n", (int)error); 
            return;
        }
    }
}

void PlanetLoader::parseLiquidColor(YAML::Node& node, PlanetGenData* genData) {
    if (node.IsNull() || !node.IsMap()) {
        std::cout << "Failed to parse node";
        return;
    }

    LiquidColorKegProperties kegProps;

    Keg::Error error;
    error = Keg::parse((ui8*)&kegProps, node, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(LiquidColorKegProperties));
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

void PlanetLoader::parseTerrainColor(YAML::Node& node, PlanetGenData* genData) {
    if (node.IsNull() || !node.IsMap()) {
        std::cout << "Failed to parse node";
        return;
    }

    TerrainColorKegProperties kegProps;

    Keg::Error error;
    error = Keg::parse((ui8*)&kegProps, node, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(TerrainColorKegProperties));
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

vg::GLProgram* PlanetLoader::generateProgram(PlanetGenData* genData,
                                             TerrainFuncs& baseTerrainFuncs,
                                             TerrainFuncs& tempTerrainFuncs,
                                             TerrainFuncs& humTerrainFuncs) {
    // Build initial string
    nString fSource = NOISE_SRC_FRAG;
    fSource.reserve(fSource.size() + 8192);

    // Set initial values
    fSource += N_HEIGHT + "=" + std::to_string(baseTerrainFuncs.baseHeight) + ";";
    fSource += N_TEMP + "=" + std::to_string(tempTerrainFuncs.baseHeight) + ";";
    fSource += N_HUM + "=" + std::to_string(humTerrainFuncs.baseHeight) + ";";

    // Add all the noise functions
    addNoiseFunctions(fSource, N_HEIGHT, baseTerrainFuncs);
    addNoiseFunctions(fSource, N_TEMP, tempTerrainFuncs);
    addNoiseFunctions(fSource, N_HUM, humTerrainFuncs);

    // Add biome code
    addBiomes(fSource, genData);

    // Add final brace
    fSource += "}";

    // Create the shader
    vg::GLProgram* program = new vg::GLProgram;
    program->init();
    program->addShader(vg::ShaderType::VERTEX_SHADER, NOISE_SRC_VERT.c_str());
    program->addShader(vg::ShaderType::FRAGMENT_SHADER, fSource.c_str());
    program->bindFragDataLocation(0, N_HEIGHT.c_str());
    program->bindFragDataLocation(1, N_TEMP.c_str());
    program->bindFragDataLocation(2, N_HUM.c_str());
    program->link();
    std::cout << fSource << std::endl;
    if (!program->getIsLinked()) {
        showMessage("Failed to generate shader program");
        return nullptr;
    }

    program->initAttributes();
    program->initUniforms();

    return program;
}

void PlanetLoader::addNoiseFunctions(nString& fSource, const nString& variable, const TerrainFuncs& funcs) {
#define TS(x) (std::to_string(x))
    // Conditional scaling code. Generates (total / maxAmplitude) * (high - low) * 0.5 + (high + low) * 0.5;
#define SCALE_CODE ((fn.low != -1.0f || fn.high != 1.0f) ? \
    fSource += variable + "+= (total / maxAmplitude) * (" + \
        TS(fn.high) + " - " + TS(fn.low) + ") * 0.5 + (" + TS(fn.high) + " + " + TS(fn.low) + ") * 0.5;" :\
    fSource += variable + "+= total / maxAmplitude;")
    
    for (auto& fn : funcs.funcs) {
        switch (fn.func) {
            case TerrainFunction::NOISE:
                fSource += R"(
                total = 0.0;
                amplitude = 1.0;
                maxAmplitude = 0.0;
                frequency = )" + TS(fn.frequency) + R"(;

                for (int i = 0; i < )" + TS(fn.octaves) + R"(; i++) {
                    total += snoise(pos * frequency) * amplitude;

                    frequency *= 2.0;
                    maxAmplitude += amplitude;
                    amplitude *= )" + TS(fn.persistence) + R"(;
                }
                )";
                SCALE_CODE;
                break;
            case TerrainFunction::RIDGED_NOISE:
                fSource += R"(
                total = 0.0;
                amplitude = 1.0;
                maxAmplitude = 0.0;
                frequency = )" + TS(fn.frequency) + R"(;

                for (int i = 0; i < )" + TS(fn.octaves) + R"(; i++) {
                    total += (1.0 - abs(snoise(pos * frequency) * amplitude)) * 2.0 - 1.0;

                    frequency *= 2.0;
                    maxAmplitude += amplitude;
                    amplitude *= )" + TS(fn.persistence) + R"(;
                }
                )";
                SCALE_CODE;
                break;
            default:
                break;
        }
    }
}

void PlanetLoader::addBiomes(nString& fSource, PlanetGenData* genData) {
    
    // Base biome lookup
    fSource += "float biomeIndex = texture(unBaseBiomes, " + N_TEMP_HUM_V2 + " / 255.0   ).x * 255.0f; ";
    fSource += N_BIOME + " = biomeIndex;";
    fSource += "float baseMult = 1.0;";

    for (int i = 0; i < genData->biomes.size(); i++) {
        // Add if
        if (i == 0) {
            fSource += "if ";
        } else {
            fSource += "else if ";
        }
        // Add conditional
        fSource += "(biomeIndex <" + std::to_string((float)i + 0.01) + ") {";
        // Mult lookup
        fSource += "baseMult = texture(unBiomes, vec3(" + N_TEMP + "," + N_HUM + "," + std::to_string(i) + ")).x;";
        // Closing curly brace
        fSource += "}";
    }
}
