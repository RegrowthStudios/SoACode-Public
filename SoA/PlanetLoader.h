///
/// PlanetLoader.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 19 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles the loading of planet files and the
/// construction of GPU generation shaders.
///

#pragma once

#ifndef PlanetLoader_h__
#define PlanetLoader_h__

#pragma once

#include <GLProgram.h>
#include <Keg.h>
#include <vector>
#include "TextureCache.h"
#include "Biome.h"

class IOManager;
class TerrainFuncs;

class PlanetGenData {
public:
    vg::Texture terrainColorMap = 0;
    vg::Texture liquidColorMap = 0;
    vg::Texture terrainTexture = 0;
    vg::Texture liquidTexture = 0;
    ColorRGB8 liquidTint = ColorRGB8(255, 255, 255);
    ColorRGB8 terrainTint = ColorRGB8(255, 255, 255);
    float liquidDepthScale = 1000.0f;
    VGTexture biomeArrayTexture = 0;
    std::vector<Biome> biomes;
    vg::GLProgram* program = nullptr;
};

class PlanetLoader {
public:
    PlanetLoader(IOManager* ioManager);
    ~PlanetLoader();

    PlanetGenData* loadPlanet(const nString& filePath);
    PlanetGenData* getDefaultGenData();
private:
    void loadBiomes(const nString& filePath, PlanetGenData* genData);

    void addBiomePixel(ui32 colorCode, int index);

    void parseTerrainFuncs(TerrainFuncs* terrainFuncs, YAML::Node& node);
    
    void parseLiquidColor(YAML::Node& node, PlanetGenData* genData);
    
    void parseTerrainColor(YAML::Node& node, PlanetGenData* genData);

    vg::GLProgram* generateProgram(PlanetGenData* genData,
                                   TerrainFuncs& baseTerrainFuncs,
                                   TerrainFuncs& tempTerrainFuncs,
                                   TerrainFuncs& humTerrainFuncs);
    void addNoiseFunctions(nString& fSource, const nString& variable, const TerrainFuncs& funcs);

    ui32 m_biomeCount = 0;
    std::map<ui32, ui32> m_baseTextureMap;
    std::map<ui32, std::vector<ui8> > m_interpLookupMap;

    PlanetGenData* m_defaultGenData = nullptr;

    IOManager* m_iom = nullptr;

    vg::TextureCache m_textureCache;
};

#endif // PlanetLoader_h__

