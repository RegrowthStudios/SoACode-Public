#include "stdafx.h"
#include "NoiseShaderGenerator.h"
#include "NoiseShaderCode.hpp"
#include "PlanetData.h"
#include "Errors.h"
#include "ProgramGenDelegate.h"

#include <Vorb/RPC.h>
#include <Vorb/graphics/GLProgram.h>

vg::GLProgram* NoiseShaderGenerator::generateProgram(PlanetGenData* genData,
                                                     vcore::RPCManager* glrpc /* = nullptr */) {
    // Build initial string
    nString fSource = NOISE_SRC_FRAG;
    fSource.reserve(fSource.size() + 8192);

    // Set initial values
    fSource += N_HEIGHT + "=" + std::to_string(genData->baseTerrainFuncs.baseHeight) + ";\n";
    fSource += N_TEMP + "=" + std::to_string(genData->tempTerrainFuncs.baseHeight) + ";\n";
    fSource += N_HUM + "=" + std::to_string(genData->humTerrainFuncs.baseHeight) + ";\n";

    // Add all the noise functions
    modCounter = 0;
    addNoiseFunctions(fSource, N_HEIGHT, genData->baseTerrainFuncs.funcs, "");
    addNoiseFunctions(fSource, N_TEMP, genData->tempTerrainFuncs.funcs, "");
    addNoiseFunctions(fSource, N_HUM, genData->humTerrainFuncs.funcs, "");

    // Add biome code
    addBiomes(fSource, genData);

    // Add final brace
    fSource += "}";

    // Generate the shader
    ProgramGenDelegate gen;
    vg::ShaderSource vertSource, fragSource;
    vertSource.sources.push_back(NOISE_SRC_VERT.c_str());
    vertSource.stage = vg::ShaderType::VERTEX_SHADER;
    fragSource.sources.push_back(fSource.c_str());
    fragSource.stage = vg::ShaderType::FRAGMENT_SHADER;
    std::vector<nString> attr;
    attr.push_back(N_HEIGHT.c_str());
    attr.push_back(N_TEMP.c_str());
    attr.push_back(N_HUM.c_str());

    gen.init("TerrainGen", vertSource, fragSource, &attr);

    if (glrpc) {
        glrpc->invoke(&gen.rpc, true);
    } else {
        gen.invoke(nullptr, nullptr);
    }

    // Check if we should use an RPC
    if (gen.program == nullptr) {
        dumpShaderCode(std::cout, fSource, true);
        std::cerr << "Failed to load shader NormalMapGen with error: " << gen.errorMessage;
    }
    dumpShaderCode(std::cout, fSource, true);

    return gen.program;
}

vg::GLProgram* NoiseShaderGenerator::getDefaultProgram(vcore::RPCManager* glrpc /* = nullptr */) {
    // Build string
    nString fSource = NOISE_SRC_FRAG;
    fSource.reserve(fSource.size() + 128);
    // Use pos so uniforms don't get optimized out
    fSource += N_HEIGHT + "= pos.x*0.000001;";
    fSource += N_TEMP + "= 0;";
    fSource += N_HUM + "= 0; }";

    // Generate the shader
    ProgramGenDelegate gen;
    vg::ShaderSource vertSource, fragSource;
    vertSource.sources.push_back(NOISE_SRC_VERT.c_str());
    vertSource.stage = vg::ShaderType::VERTEX_SHADER;
    fragSource.sources.push_back(fSource.c_str());
    fragSource.stage = vg::ShaderType::FRAGMENT_SHADER;
    std::vector<nString> attr;
    attr.push_back(N_HEIGHT.c_str());
    attr.push_back(N_TEMP.c_str());
    attr.push_back(N_HUM.c_str());

    gen.init("TerrainGen", vertSource, fragSource, &attr);

    // Check if we should use an RPC
    if (glrpc) {
        glrpc->invoke(&gen.rpc, true);
    } else {
        gen.invoke(nullptr, nullptr);
    }

    if (gen.program == nullptr) {
        dumpShaderCode(std::cout, fSource, true);
        std::cerr << "Failed to load shader NormalMapGen with error: " << gen.errorMessage;
    }
    return gen.program;
}

void NoiseShaderGenerator::addNoiseFunctions(OUT nString& fSource, const nString& variable,
                                             const Array<TerrainFuncKegProperties>& funcs, const nString& modifier) {

   
    nString modVar = "m" + std::to_string(++modCounter);
    fSource += "float " + modVar + " = 0.0; ";

#define TS(x) (std::to_string(x))
    // Conditional scaling code. Generates (total / maxAmplitude) * (high - low) * 0.5 + (high + low) * 0.5;
#define SCALE_CODE ((fn.low != -1.0f || fn.high != 1.0f) ? \
    fSource += modVar + "+= (total / maxAmplitude) * (" + \
        TS(fn.high) + " - " + TS(fn.low) + ") * 0.5 + (" + TS(fn.high) + " + " + TS(fn.low) + ") * 0.5;\n" :\
    fSource += modVar + "+= total / maxAmplitude;\n")

    // NOTE: Make sure this implementation matches SphericalTerrainCpuGenerator::getNoiseValue()
    for (int f = 0; f < funcs.size(); f++) {
        auto& fn = funcs[f];
        std::cout << "LOL: " << fn.children.size() << std::endl;
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
        // If we have children, we need to clamp from 0 to 1
        if (fn.children.size()) {
            fSource += modVar + " = clamp(" + modVar + ", 0.0, 1.0);";
            if (modifier.length()) {
                fSource += modVar + " *= " + modifier + ";";
            }

            if (fn.children.size()) {
                std::cout << "Here: " << fn.children.size() << std::endl;
                addNoiseFunctions(fSource, variable, fn.children, modVar);
            }
        } else {
            if (modifier.size()) {
                fSource += modVar + " *= " + modifier + ";";
            }
            fSource += variable + " += " + modVar + ";";
        }
    }
}

void NoiseShaderGenerator::addBiomes(OUT nString& fSource, PlanetGenData* genData) {

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

void NoiseShaderGenerator::dumpShaderCode(std::ostream& stream, nString source, bool addLineNumbers) {

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
