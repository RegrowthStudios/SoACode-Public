#include "stdafx.h"
#include "PlanetLoader.h"

#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/io/IOManager.h>

#include "NoiseShaderCode.hpp"
#include "Errors.h"

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

PlanetLoader::PlanetLoader(vio::IOManager* ioManager) :
    m_iom(ioManager),
    m_textureCache(m_iom) {
    // Empty
}

PlanetLoader::~PlanetLoader() {
}

PlanetGenData* PlanetLoader::loadPlanet(const nString& filePath) {
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
        }
    });
    reader.forAllInMap(node, f);
    reader.dispose();
    
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

vg::GLProgram* PlanetLoader::generateProgram(PlanetGenData* genData,
                                             TerrainFuncs& baseTerrainFuncs,
                                             TerrainFuncs& tempTerrainFuncs,
                                             TerrainFuncs& humTerrainFuncs) {
    // Build initial string
    nString fSource = NOISE_SRC_FRAG;
    fSource.reserve(fSource.size() + 8192);

    // Set initial values
    fSource += N_HEIGHT + "=" + std::to_string(baseTerrainFuncs.baseHeight) + ";\n";
    fSource += N_TEMP + "=" + std::to_string(tempTerrainFuncs.baseHeight) + ";\n";
    fSource += N_HUM + "=" + std::to_string(humTerrainFuncs.baseHeight) + ";\n";

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
    if (!program->getIsLinked()) {
        dumpShaderCode(std::cout, fSource, true);
        // Show message
        showMessage("Failed to generate GPU gen program. See command prompt for details\n");
        return nullptr;
    } else {
        dumpShaderCode(std::cout, fSource, true);
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
        TS(fn.high) + " - " + TS(fn.low) + ") * 0.5 + (" + TS(fn.high) + " + " + TS(fn.low) + ") * 0.5;\n" :\
    fSource += variable + "+= total / maxAmplitude;\n")
    
    for (auto& fn : funcs.funcs) {
        switch (fn.func) {
            case TerrainFunction::NOISE:
                fSource = fSource + 
                    "total = 0.0;\n" +
                    "amplitude = 1.0;\n" +
                    "maxAmplitude = 0.0;\n" +
                    "frequency = " + TS(fn.frequency) + ";\n" +

                    "for (int i = 0; i < " + TS(fn.octaves) + "; i++) {\n" +
                    "  total += snoise(pos * frequency) * amplitude;\n" +

                    "  frequency *= 2.0;\n" +
                    "  maxAmplitude += amplitude;\n" +
                    "  amplitude *= " + TS(fn.persistence) + ";\n" +
                    "}\n";
                SCALE_CODE;
                break;
            case TerrainFunction::RIDGED_NOISE:
                fSource = fSource +
                    "total = 0.0;\n" +
                    "amplitude = 1.0;\n" +
                    "maxAmplitude = 0.0;\n" +
                    "frequency = " + TS(fn.frequency) + ";\n" +

                    "for (int i = 0; i < " + TS(fn.octaves) + "; i++) {\n" +
                    "  total += ((1.0 - abs(snoise(pos * frequency))) * 2.0 - 1.0) * amplitude;\n" +

                    "  frequency *= 2.0;\n" +
                    "  maxAmplitude += amplitude;\n" +
                    "  amplitude *= " + TS(fn.persistence) + ";\n" +
                    "}\n";
                SCALE_CODE;
                break;
            default:
                break;
        }
    }
}

void PlanetLoader::addBiomes(nString& fSource, PlanetGenData* genData) {
    
    // Base biome lookup
    fSource += "float biomeIndex = texture(unBaseBiomes, " + N_TEMP_HUM_V2 + " / 255.0   ).x * 255.0f;\n ";
    fSource += N_BIOME + " = biomeIndex;\n";
    fSource += "float baseMult = 1.0;\n";

    for (int i = 0; i < genData->biomes.size(); i++) {
        // Add if
        if (i == 0) {
            fSource += "if ";
        } else {
            fSource += "else if ";
        }
        // Add conditional
        fSource += "(biomeIndex <" + std::to_string((float)i + 0.01) + ") {\n";
        // Mult lookup
        fSource += "  baseMult = texture(unBiomes, vec3(" + N_TEMP + "," + N_HUM + "," + std::to_string(i) + ")).x;\n";
        // Closing curly brace
        fSource += "} ";
    }
    fSource += '\n';
}

void PlanetLoader::dumpShaderCode(std::ostream& stream, nString source, bool addLineNumbers) {

    // Auto-formatting
    int totalLines = 1;
    int indentLevel = 0;
    bool skipBrace = false;
    const int TAB = 2;
    for (size_t i = 0; i < source.size(); i++) {
        // Detect braces for indentation change
        if (source[i] == '{') {
            indentLevel++;
        } else if (source[i] == '}') {
            indentLevel--;
        } else if (source[i] == '\n') { // Add spaces as needed on newlines
            totalLines++;
            int t = ++i;
            // Count spaces
            while (i < source.size() && source[i] == ' ') i++;
            if (i == source.size()) break;
            // Check for case when next line starts with }
            if (source[t] == '}') {
                indentLevel--;
                skipBrace = true;
            }
            int nSpaces = i - t;
            // Make sure theres the right number of spaces
            if (nSpaces < indentLevel * TAB) {
                nString spaces = "";
                for (int s = 0; s < indentLevel * TAB - nSpaces; s++) spaces += ' ';
                source.insert(t, spaces);
                i = t + spaces.length() - 1;
            } else if (nSpaces > indentLevel * TAB) {
                source.erase(t, nSpaces - indentLevel * TAB);
                i = t - 1;
            } else {
                i = t - 1;
            }
            // Don't want to decrement indent twice
            if (skipBrace) {
                skipBrace = false;
                i++;
            }
        }
    }
    // Insert line numbers
    int width = log10(totalLines) + 1;
    if (addLineNumbers) {
        // See how much room we need for line numbers
        int width = log10(totalLines) + 1;
        char buf[32];
        int lineNum = 1;
        for (size_t i = 0; i < source.size(); i++) {
            if (source[i] == '\n') {
                sprintf(buf, "%*d| ", width, lineNum);
                source.insert(++i, buf);
                lineNum++;
            }
        }
    }
    // Print source
    stream << source << std::endl;
}
