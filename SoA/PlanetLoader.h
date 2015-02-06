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

#include "NoiseShaderGenerator.h"

DECL_VIO(class, IOManager);

struct TerrainFuncs;
struct PlanetGenData;

#define LOOKUP_TEXTURE_WIDTH 256
#define LOOKUP_TEXTURE_SIZE 65536

class PlanetLoader {
public:
    PlanetLoader(vio::IOManager* ioManager);
    ~PlanetLoader();

    PlanetGenData* loadPlanet(const nString& filePath);
    PlanetGenData* getDefaultGenData();
private:
    void loadBiomes(const nString& filePath, PlanetGenData* genData);

    void addBiomePixel(ui32 colorCode, int index);

    void parseTerrainFuncs(TerrainFuncs* terrainFuncs, keg::YAMLReader& reader, keg::Node node);
    
    void parseLiquidColor(keg::YAMLReader& reader, keg::Node node, PlanetGenData* genData);
    
    void parseTerrainColor(keg::YAMLReader& reader, keg::Node node, PlanetGenData* genData);

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

    NoiseShaderGenerator m_shaderGenerator;
};

#endif // PlanetLoader_h__

