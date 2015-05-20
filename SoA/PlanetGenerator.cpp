#include "stdafx.h"
#include "PlanetGenerator.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/SamplerState.h>

std::mt19937 PlanetGenerator::generator(36526);

CALLEE_DELETE PlanetGenData* PlanetGenerator::generateRandomPlanet(SpaceObjectType type) {
    switch (type) {
        case SpaceObjectType::PLANET:
        case SpaceObjectType::DWARF_PLANET:
        case SpaceObjectType::MOON:
        case SpaceObjectType::DWARF_MOON:
            return generatePlanet();
        case SpaceObjectType::ASTEROID:
            return generateAsteroid();
        case SpaceObjectType::COMET:
            return generateComet();
        default:
            return nullptr;
    }
}

CALLEE_DELETE PlanetGenData* PlanetGenerator::generatePlanet() {
    PlanetGenData* data = new PlanetGenData;
    data->terrainColorMap = getRandomColorMap();
    data->terrainTexture = getRandomColorMap();
    data->liquidColorMap = getRandomColorMap();

    std::vector<TerrainFuncKegProperties> funcs;
    // Terrain
    getRandomTerrainFuncs(funcs,
                          std::uniform_int_distribution<int>(1, 5),
                          std::uniform_int_distribution<int>(1, 4),
                          std::uniform_real_distribution<f32>(0.0008, 0.2f),
                          std::uniform_real_distribution<f32>(-100.0f, 300.0f),
                          std::uniform_real_distribution<f32>(100.0f, 10000.0f));
    data->baseTerrainFuncs.funcs.setData(funcs.data(), funcs.size());
    // Temperature
    getRandomTerrainFuncs(funcs,
                          std::uniform_int_distribution<int>(1, 5),
                          std::uniform_int_distribution<int>(1, 4),
                          std::uniform_real_distribution<f32>(0.0008, 0.2f),
                          std::uniform_real_distribution<f32>(0.0f, 50.0f),
                          std::uniform_real_distribution<f32>(200.0f, 255.0f));
    data->tempTerrainFuncs.funcs.setData(funcs.data(), funcs.size());
    // Humidity
    getRandomTerrainFuncs(funcs,
                          std::uniform_int_distribution<int>(1, 5),
                          std::uniform_int_distribution<int>(1, 4),
                          std::uniform_real_distribution<f32>(0.0008, 0.2f),
                          std::uniform_real_distribution<f32>(0.0f, 50.0f),
                          std::uniform_real_distribution<f32>(200.0f, 255.0f));
    data->humTerrainFuncs.funcs.setData(funcs.data(), funcs.size());

    return data;
}

CALLEE_DELETE PlanetGenData* PlanetGenerator::generateAsteroid() {
    PlanetGenData* data = new PlanetGenData;
    return data;
}

CALLEE_DELETE PlanetGenData* PlanetGenerator::generateComet() {
    PlanetGenData* data = new PlanetGenData;
    return data;
}

VGTexture PlanetGenerator::getRandomColorMap() {
    static const int WIDTH = 256;
    color4 pixels[WIDTH][WIDTH];
    static std::uniform_int_distribution<int> numColors(1, 12);
    static std::uniform_int_distribution<int> rPos(0, WIDTH - 1);
    static std::uniform_int_distribution<int> rColor(0, 255);

    int numPoints = numColors(generator);
    std::vector<color4> randColors(numPoints);
    std::vector<i32v2> randPoints(numPoints);
    for (int i = 0; i < numPoints; i++) {
        randColors[i].r = (ui8)rColor(generator);
        randColors[i].g = (ui8)rColor(generator);
        randColors[i].b = (ui8)rColor(generator);
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

    return vg::GpuMemory::uploadTexture(pixels, WIDTH, WIDTH, vg::TexturePixelType::UNSIGNED_BYTE,
                                        vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_CLAMP);
}

void PlanetGenerator::getRandomTerrainFuncs(OUT std::vector<TerrainFuncKegProperties>& funcs,
                                            const std::uniform_int_distribution<int>& funcsRange,
                                            const std::uniform_int_distribution<int>& octavesRange,
                                            const std::uniform_real_distribution<f32>& freqRange,
                                            const std::uniform_real_distribution<f32>& heightMinRange,
                                            const std::uniform_real_distribution<f32>& heightWidthRange) {
    int numFuncs = funcsRange(generator);
    funcs.resize(numFuncs);

    for (int i = 0; i < numFuncs; i++) {
        auto& f = funcs[i];
        f.low = heightMinRange(generator);
        f.high = f.low + heightWidthRange(generator);
        f.octaves = octavesRange(generator);
        f.frequency = freqRange(generator);
        f.persistence = 0.8;
    }
}