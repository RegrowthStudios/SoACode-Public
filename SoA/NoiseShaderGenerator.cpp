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
    fSource += N_HEIGHT + "=" + std::to_string(genData->baseTerrainFuncs.base) + ";\n";
    fSource += N_TEMP + "=" + std::to_string(genData->tempTerrainFuncs.base) + ";\n";
    fSource += N_HUM + "=" + std::to_string(genData->humTerrainFuncs.base) + ";\n";

    // Add all the noise functions
    m_funcCounter = 0;
    addNoiseFunctions(fSource, N_HEIGHT, genData->baseTerrainFuncs.funcs, "", TerrainOp::ADD);
    addNoiseFunctions(fSource, N_TEMP, genData->tempTerrainFuncs.funcs, "", TerrainOp::ADD);
    addNoiseFunctions(fSource, N_HUM, genData->humTerrainFuncs.funcs, "", TerrainOp::ADD);

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

char getOpChar(TerrainOp op) {
    switch (op) {
        case TerrainOp::ADD: return '+';
        case TerrainOp::SUB: return '-';
        case TerrainOp::MUL: return '*';
        case TerrainOp::DIV: return '/';
    }
    return '?';
}

void NoiseShaderGenerator::addNoiseFunctions(OUT nString& fSource, const nString& variable,
                                             const Array<TerrainFuncKegProperties>& funcs, const nString& modifier,
                                             const TerrainOp& op) {
#define TS(x) (std::to_string(x))

    // Each call to addNoiseFunctions gets its own h variable
    nString h = "h" + std::to_string(++m_funcCounter);
    fSource += "float " + h + " = 0.0; ";

    // NOTE: Make sure this implementation matches SphericalTerrainCpuGenerator::getNoiseValue()
    for (int f = 0; f < funcs.size(); f++) {
        auto& fn = funcs[f];

        fSource = fSource +
            "total = 0.0;\n" +
            "amplitude = 1.0;\n" +
            "maxAmplitude = 0.0;\n" +
            "frequency = " + TS(fn.frequency) + ";\n" +

            "for (int i = 0; i < " + TS(fn.octaves) + "; i++) {\n";
        switch (fn.func) {
            case TerrainFunction::NOISE:
                fSource += "total += snoise(pos * frequency) * amplitude;\n";
                break;
            case TerrainFunction::RIDGED_NOISE:
                fSource += "total += ((1.0 - abs(snoise(pos * frequency))) * 2.0 - 1.0) * amplitude;\n";
                break;
            case TerrainFunction::ABS_NOISE:
                fSource += "total += abs(snoise(pos * frequency)) * amplitude;\n";
                break;
            case TerrainFunction::SQUARED_NOISE:
                fSource += "tmp = snoise(pos * frequency);\n";
                fSource += "total += tmp * tmp * amplitude;\n";
                break;
            case TerrainFunction::CUBED_NOISE:
                fSource += "tmp = snoise(pos * frequency);\n";
                fSource += "total += tmp * tmp * tmp * amplitude;\n";
                break;
        }
        fSource = fSource + 
            "  frequency *= 2.0;\n" +
            "  maxAmplitude += amplitude;\n" +
            "  amplitude *= " + TS(fn.persistence) + ";\n" +
            "}\n";
        // Conditional scaling. 
        if (fn.low != -1.0f || fn.high != 1.0f) {
            // (total / maxAmplitude) * (high - low) * 0.5 + (high + low) * 0.5;
            fSource += h + "+= (total / maxAmplitude) * (" +
                TS(fn.high) + " - " + TS(fn.low) + ") * 0.5 + (" + TS(fn.high) + " + " + TS(fn.low) + ") * 0.5;\n";
        } else {
            fSource += h + "+= total / maxAmplitude;\n";
        }

        // Optional clamp if both fields are not 0.0f
        if (fn.clamp[0] != 0.0f || fn.clamp[1] != 0.0f) {
            fSource += h + " = clamp(" + h + "," + TS(fn.clamp[0]) + "," + TS(fn.clamp[1]) + ");";
        }
        // Apply modifier from parent if needed
        if (modifier.length()) {
            fSource += h + getOpChar(op) + "=" + modifier + ";";
        }
        // If we have children, we need to clamp from 0 to 1
        if (fn.children.size()) {
            // Recursively call children with operation
            addNoiseFunctions(fSource, variable, fn.children, h, fn.op);
        } else {
            // Modify the final terrain height
            fSource += variable + getOpChar(fn.op) + "=" + h + ";";
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
