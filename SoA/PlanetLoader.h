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

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/io/Keg.h>
#include <Vorb/graphics/TextureCache.h>
#include <Vorb/VorbPreDecl.inl>

#include "Biome.h"

DECL_VIO(class, IOManager);

class TerrainFuncs;

#define LOOKUP_TEXTURE_WIDTH 256
#define LOOKUP_TEXTURE_SIZE 65536

class PlanetGenData {
public:
    vg::Texture terrainColorMap = 0;
    vg::Texture liquidColorMap = 0;
    vg::Texture terrainTexture = 0;
    vg::Texture liquidTexture = 0;
    ColorRGB8 liquidTint = ColorRGB8(255, 255, 255);
    ColorRGB8 terrainTint = ColorRGB8(255, 255, 255);
    float liquidDepthScale = 1000.0f;
    float liquidFreezeTemp = -1.0f;
    float tempLatitudeFalloff = 0.0f;
    float humLatitudeFalloff = 0.0f;
    VGTexture biomeArrayTexture = 0;
    VGTexture baseBiomeLookupTexture = 0;
    std::vector<Biome> biomes;
    vg::GLProgram* program = nullptr;
};

class PlanetLoader {
public:
    PlanetLoader(vio::IOManager* ioManager);
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

    void addBiomes(nString& fSource, PlanetGenData* genData);

    class BiomeLookupTexture {
    public:
        int index;
        std::vector<ui8> data = std::vector<ui8>(LOOKUP_TEXTURE_SIZE, 0);
    };

    ui32 m_biomeCount = 0;
    std::map<ui32, BiomeLookupTexture> m_biomeLookupMap;
    std::vector<ui8> baseBiomeLookupTexture;

    PlanetGenData* m_defaultGenData = nullptr;

    vio::IOManager* m_iom = nullptr;

    vg::TextureCache m_textureCache;
};

#endif // PlanetLoader_h__

