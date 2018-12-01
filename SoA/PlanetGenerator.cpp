#include "stdafx.h"
#include "PlanetGenerator.h"

#include "ShaderLoader.h"
#include "SoaOptions.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/SamplerState.h>

#define BLUR_PASSES 4

PlanetGenerator::PlanetGenerator() {
    // Empty
}

void PlanetGenerator::dispose(vcore::RPCManager* glrpc VORB_UNUSED) {
    // TODO(Ben): Implement
}

CALLER_DELETE PlanetGenData* PlanetGenerator::generateRandomPlanet(SpaceObjectType type, vcore::RPCManager* glrpc /* = nullptr */) {
    switch (type) {
        case SpaceObjectType::PLANET:
        case SpaceObjectType::DWARF_PLANET:
        case SpaceObjectType::MOON:
        case SpaceObjectType::DWARF_MOON:
            return generatePlanet(glrpc);
        case SpaceObjectType::ASTEROID:
            return generateAsteroid(glrpc);
        case SpaceObjectType::COMET:
            return generateComet(glrpc);
        default:
            return nullptr;
    }
}

CALLER_DELETE PlanetGenData* PlanetGenerator::generatePlanet(vcore::RPCManager* glrpc) {
    PlanetGenData* data = new PlanetGenData;
    data->terrainColorMap = getRandomColorMap(glrpc, true);
    data->liquidColorMap = getRandomColorMap(glrpc, true);

    // Falloffs
    static std::uniform_real_distribution<f32> falloff(0.0f, 100.0f);
    f32 f = falloff(m_generator);
    data->tempLatitudeFalloff = f * 1.9f;
    data->tempHeightFalloff = 5.0f;
    data->humLatitudeFalloff = f * 0.3f;
    data->humHeightFalloff = 1.0f;

    std::vector<TerrainFuncProperties> funcs;
    // Mountains
    getRandomTerrainFuncs(funcs,
                          TerrainStage::RIDGED_NOISE,
                          0, 2,
                          3, 7,
                          0.00001f, 0.001f,
                          -15000.0f, 15000.0f,
                          100.0f, 30000.0f);
                          
    // Terrain

    getRandomTerrainFuncs(funcs,
                          TerrainStage::NOISE,
                          2, 5,
                          1, 4,
                          0.0002f, 0.2f,
                          -500.0f, 500.0f,
                          10.0f, 1000.0f);
    data->baseTerrainFuncs.funcs.setData(funcs.data(), funcs.size());
    funcs.clear();
    // Temperature

    data->tempTerrainFuncs.base = 128.0f;
    getRandomTerrainFuncs(funcs,
                          TerrainStage::NOISE,
                          1, 2,
                          3, 8,
                          0.00008f, 0.008f,
                          -128.0f, -128.0f,
                          255.0f, 255.0f);
    data->tempTerrainFuncs.funcs.setData(funcs.data(), funcs.size());
    funcs.clear();
    // Humidity
    data->humTerrainFuncs.base = 128.0f;
    getRandomTerrainFuncs(funcs,
                          TerrainStage::NOISE,
                          1, 2,
                          3, 8,
                          0.00008f, 0.008f,
                          -128.0f, -128.0f,
                          255.0f, 255.0f);
    data->humTerrainFuncs.funcs.setData(funcs.data(), funcs.size());
    funcs.clear();

    return data;
}

CALLER_DELETE PlanetGenData* PlanetGenerator::generateAsteroid(vcore::RPCManager* glrpc VORB_MAYBE_UNUSED) {
    PlanetGenData* data = new PlanetGenData;
    return data;
}

CALLER_DELETE PlanetGenData* PlanetGenerator::generateComet(vcore::RPCManager* glrpc VORB_MAYBE_UNUSED) {
    PlanetGenData* data = new PlanetGenData;
    return data;
}

VGTexture PlanetGenerator::getRandomColorMap(vcore::RPCManager* glrpc, bool shouldBlur) {
    static const int WIDTH = 256;
    color4 pixels[WIDTH][WIDTH];
    static std::uniform_int_distribution<int> numColors(4, 12);
    static std::uniform_int_distribution<int> rPos(0, WIDTH - 1);
    static std::uniform_int_distribution<int> rColor(0, 16777215); // 0-2^24

    int numPoints = numColors(m_generator);
    std::vector<color4> randColors(numPoints);
    std::vector<i32v2> randPoints(numPoints);
    for (int i = 0; i < numPoints; i++) {
        int newColor = rColor(m_generator);
        randColors[i].r = (ui8)(newColor >> 16);
        randColors[i].g = (ui8)((newColor >> 8) & 0xFF);
        randColors[i].b = (ui8)(newColor & 0xFF);
        randColors[i].a = (ui8)255;
        randPoints[i].x = rPos(m_generator);
        randPoints[i].y = rPos(m_generator);
    }
    
    // Voronoi diagram generation
    // TODO(Ben): n^3 is slow
    for (int y = 0; y < WIDTH; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int closestDist = INT_MAX;
            int closestIndex = 0;
            for (int i = 0; i < numPoints; i++) {
                int dx = (x - randPoints[i].x);
                int dy = (y - randPoints[i].y);
                int dist = dx * dx + dy * dy;
                if (dist < closestDist) {
                    closestDist = dist;
                    closestIndex = i;
                }
            }
            pixels[y][x] = randColors[closestIndex];
        }
    }

    // TODO: Implement these as suitable.
    // Upload texture
    VGTexture tex = 0;
    if (glrpc) {
        vcore::RPC rpc;
        rpc.data.f = new vcore::RPCFunction(makeFunctor([&](Sender, void*) {
            tex = vg::GpuMemory::uploadTexture(pixels, WIDTH, WIDTH, vg::TexturePixelType::UNSIGNED_BYTE,
                                              vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_CLAMP);
        }));
        glrpc->invoke(&rpc, true);
        delete rpc.data.f;
    } else {
        tex = vg::GpuMemory::uploadTexture(pixels, WIDTH, WIDTH, vg::TexturePixelType::UNSIGNED_BYTE,
                                          vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_CLAMP);
    }

    // Handle Gaussian blur
    auto f = makeFunctor([&](Sender, void*) {
        if (!m_blurPrograms[0].isCreated()) {
            m_blurPrograms[0] = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                                    "Shaders/PostProcessing/Blur.frag", nullptr,
                                                                    "#define HORIZONTAL_BLUR_9\n");
            m_blurPrograms[1] = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                                    "Shaders/PostProcessing/Blur.frag", nullptr,
                                                                    "#define VERTICAL_BLUR_9\n");
            m_quad.init();
            m_targets[0].setSize(WIDTH, WIDTH);
            m_targets[1].setSize(WIDTH, WIDTH);
            m_targets[0].init();
            m_targets[1].init();
        }
        // Bind the voronoi color map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        // Compute BLUR_PASSES passes of 2-pass gaussian blur
        for (int p = 0; p < BLUR_PASSES; p++) {
            // Two pass Gaussian blur
            for (int i = 0; i < 2; i++) {
                m_blurPrograms[i].use();
                m_blurPrograms[i].enableVertexAttribArrays();

                glUniform1i(m_blurPrograms[i].getUniform("unTex"), 0);
                glUniform1f(m_blurPrograms[i].getUniform("unSigma"), 5.0f);
                glUniform1f(m_blurPrograms[i].getUniform("unBlurSize"), 1.0f / (f32)WIDTH);
                m_targets[i].use();

                m_quad.draw();

                m_targets[i].unuse((ui32)soaOptions.get(OPT_SCREEN_WIDTH).value.i, (ui32)soaOptions.get(OPT_SCREEN_HEIGHT).value.i);
                m_blurPrograms[i].disableVertexAttribArrays();
                m_blurPrograms[i].unuse();
                m_targets[i].bindTexture();
            }
        }

        // Get the pixels and use them to re-upload the texture
        // TODO(Ben): A direct copy would be more efficient.
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, WIDTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    });

    // See if we should use RPC
    if (shouldBlur) {
        if (glrpc) {
            vcore::RPC rpc;
            rpc.data.f = new vcore::RPCFunction(std::move(f));
            glrpc->invoke(&rpc, true);
            delete rpc.data.f;
        } else {
            f.invoke(nullptr, nullptr);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void PlanetGenerator::getRandomTerrainFuncs(OUT std::vector<TerrainFuncProperties>& funcs,
                                            TerrainStage func,
                                            int funcsRange1, int funcsRange2,
                                            int octavesRange1, int octavesRange2,
                                            float freqRange1, float freqRange2,
                                            float heightMinRange1, float heightMinRange2,
                                            float heightWidthRange1, float heightWidthRange2) {

    std::uniform_int_distribution<int> funcsRange(funcsRange1, funcsRange2);
    std::uniform_int_distribution<int> octavesRange(octavesRange1, octavesRange2);
    std::uniform_real_distribution<f32> freqRange(freqRange1, freqRange2);
    std::uniform_real_distribution<f32> heightMinRange(heightMinRange1, heightMinRange2);
    std::uniform_real_distribution<f32> heightWidthRange(heightWidthRange1, heightWidthRange2);

    int numFuncs = funcsRange(m_generator);
    if (numFuncs <= 0) return;
    funcs.resize(funcs.size() + numFuncs);

    for (int i = 0; i < numFuncs; i++) {
        auto& f = funcs[i];
        f.func = func;
        f.low = heightMinRange(m_generator);
        f.high = f.low + heightWidthRange(m_generator);
        f.octaves = octavesRange(m_generator);
        f.frequency = freqRange(m_generator);
        f.persistence = 0.8f;
    }
}