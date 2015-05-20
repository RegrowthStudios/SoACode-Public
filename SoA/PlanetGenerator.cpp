#include "stdafx.h"
#include "PlanetGenerator.h"
#include "PlanetData.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/SamplerState.h>
#include <random>

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

    TerrainFuncKegProperties tf1;
    tf1.low = 1.0;
    tf1.high = 100.0;
    data->baseTerrainFuncs.funcs.setData(&tf1, 1);
    data->tempTerrainFuncs.funcs.setData(&tf1, 1);
    data->humTerrainFuncs.funcs.setData(&tf1, 1);
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
    static std::mt19937 generator(1053);
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
