#include "stdafx.h"
#include "PlanetGenerator.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/SamplerState.h>

CALLEE_DELETE PlanetGenData* PlanetGenerator::generateRandomPlanet(SpaceObjectType type, vcore::RPCManager* glrpc /* = nullptr */) {
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

CALLEE_DELETE PlanetGenData* PlanetGenerator::generatePlanet(vcore::RPCManager* glrpc) {
    PlanetGenData* data = new PlanetGenData;
    data->terrainColorMap = getRandomColorMap(glrpc, true);
    data->liquidColorMap = getRandomColorMap(glrpc, true);

    std::vector<TerrainFuncKegProperties> funcs;
    // Terrain
    getRandomTerrainFuncs(funcs,
                          std::uniform_int_distribution<int>(1, 5),
                          std::uniform_int_distribution<int>(1, 4),
                          std::uniform_real_distribution<f32>(0.0001, 0.2f),
                          std::uniform_real_distribution<f32>(-4000.0f, 5000.0f),
                          std::uniform_real_distribution<f32>(10.0f, 8000.0f));
    data->baseTerrainFuncs.funcs.setData(funcs.data(), funcs.size());
    funcs.clear();
    // Temperature
    getRandomTerrainFuncs(funcs,
                          std::uniform_int_distribution<int>(1, 3),
                          std::uniform_int_distribution<int>(1, 7),
                          std::uniform_real_distribution<f32>(0.00008, 0.008f),
                          std::uniform_real_distribution<f32>(0.0f, 50.0f),
                          std::uniform_real_distribution<f32>(200.0f, 255.0f));
    data->tempTerrainFuncs.funcs.setData(funcs.data(), funcs.size());
    funcs.clear();
    // Humidity
    getRandomTerrainFuncs(funcs,
                          std::uniform_int_distribution<int>(1, 3),
                          std::uniform_int_distribution<int>(1, 7),
                          std::uniform_real_distribution<f32>(0.00008, 0.008f),
                          std::uniform_real_distribution<f32>(0.0f, 50.0f),
                          std::uniform_real_distribution<f32>(200.0f, 255.0f));
    data->humTerrainFuncs.funcs.setData(funcs.data(), funcs.size());
    funcs.clear();

    return data;
}

CALLEE_DELETE PlanetGenData* PlanetGenerator::generateAsteroid(vcore::RPCManager* glrpc) {
    PlanetGenData* data = new PlanetGenData;
    return data;
}

CALLEE_DELETE PlanetGenData* PlanetGenerator::generateComet(vcore::RPCManager* glrpc) {
    PlanetGenData* data = new PlanetGenData;
    return data;
}

VGTexture PlanetGenerator::getRandomColorMap(vcore::RPCManager* glrpc, bool shouldBlur) {
    static const int WIDTH = 256;
    color4 pixels[WIDTH][WIDTH];
    static std::uniform_int_distribution<int> numColors(11, 12);
    static std::uniform_int_distribution<int> rPos(0, WIDTH - 1);
    static std::uniform_int_distribution<int> rColor(0, 16777215); // 0-2^24

    int numPoints = numColors(generator);
    std::vector<color4> randColors(numPoints);
    std::vector<i32v2> randPoints(numPoints);
    for (int i = 0; i < numPoints; i++) {
        int newColor = rColor(generator);
        randColors[i].r = (ui8)(newColor >> 16);
        randColors[i].g = (ui8)((newColor >> 8) & 0xFF);
        randColors[i].b = (ui8)(newColor & 0xFF);
        randColors[i].a = (ui8)255;
        randPoints[i].x = rPos(generator);
        randPoints[i].y = rPos(generator);
    }
    
    // Voronoi diagram generation
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

    VGTexture tex;
    if (glrpc) {
        vcore::RPC rpc;
        rpc.data.f = makeFunctor<Sender, void*>([&](Sender s, void* userData) {
            tex = vg::GpuMemory::uploadTexture(pixels, WIDTH, WIDTH, vg::TexturePixelType::UNSIGNED_BYTE,
                                               vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_CLAMP);
        });
        glrpc->invoke(&rpc, true);
    } else {
        tex = vg::GpuMemory::uploadTexture(pixels, WIDTH, WIDTH, vg::TexturePixelType::UNSIGNED_BYTE,
                                           vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_CLAMP);
    }

    if (shouldBlur) {

    }

    return tex;
}

void PlanetGenerator::getRandomTerrainFuncs(OUT std::vector<TerrainFuncKegProperties>& funcs,
                                            const std::uniform_int_distribution<int>& funcsRange,
                                            const std::uniform_int_distribution<int>& octavesRange,
                                            const std::uniform_real_distribution<f32>& freqRange,
                                            const std::uniform_real_distribution<f32>& heightMinRange,
                                            const std::uniform_real_distribution<f32>& heightWidthRange) {
    int numFuncs = funcsRange(generator);
    funcs.resize(funcs.size() + numFuncs);

    for (int i = 0; i < numFuncs; i++) {
        auto& f = funcs[i];
        f.low = heightMinRange(generator);
        f.high = f.low + heightWidthRange(generator);
        f.octaves = octavesRange(generator);
        f.frequency = freqRange(generator);
        f.persistence = 0.8;
    }
}