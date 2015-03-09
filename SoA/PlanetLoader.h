///
/// PlanetLoader.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 19 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles the loading of planet files
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

DECL_VIO(class IOManager);
DECL_VCORE(class RPCManager);

struct NoiseBase;
struct PlanetGenData;

#define LOOKUP_TEXTURE_WIDTH 256
#define LOOKUP_TEXTURE_SIZE 65536

class PlanetLoader {
public:
    /// Constructor
    /// @param ioManager: Iomanager for IO
    PlanetLoader(vio::IOManager* ioManager);
    ~PlanetLoader();

    /// Loads a planet from file
    /// @param filePath: Path of the planet
    /// @param glrpc: Optional RPC if you want to load on a non-render thread
    /// @return planet gen data
    PlanetGenData* loadPlanet(const nString& filePath, vcore::RPCManager* glrpc = nullptr);
    /// Returns a default planetGenData
    /// @param glrpc: Optional RPC if you want to load on a non-render thread
    /// @return planet gen data
    PlanetGenData* getDefaultGenData(vcore::RPCManager* glrpc = nullptr);
private:
    /// Loads the biomes from file
    /// @param filePath: Path to the biome file
    /// @param genData: generation data to be modified
    void loadBiomes(const nString& filePath, OUT PlanetGenData* genData);
    /// Adds a pixel to the biome color lookup map
    /// @param colorCode: Color code of the pixel
    /// @param index: Index into the biome map
    void addBiomePixel(ui32 colorCode, int index);
    /// Parses terrain noise functions
    /// @param terrainFuncs: The functions to parse
    /// @param reader: The YAML reader
    /// @param node: The YAML node
    void parseTerrainFuncs(NoiseBase* terrainFuncs, keg::YAMLReader& reader, keg::Node node);
    /// Parses liquid color data
    /// @param reader: The YAML reader
    /// @param node: The YAML node
    /// @param genData: The generation data to modify
    void parseLiquidColor(keg::YAMLReader& reader, keg::Node node, OUT PlanetGenData* genData);
    /// Parses terrain color data
    /// @param reader: The YAML reader
    /// @param node: The YAML node
    /// @param genData: The generation data to modify
    void parseTerrainColor(keg::YAMLReader& reader, keg::Node node, OUT PlanetGenData* genData);
    /// Parses block layer data
    /// @param reader: The YAML reader
    /// @param node: The YAML node
    /// @param genData: The generation data to modify
    void parseBlockLayers(keg::YAMLReader& reader, keg::Node node, OUT PlanetGenData* genData);

    /// A lookup texture for biomes, to be used on the GPU
    class BiomeLookupTexture {
    public:
        int index;
        std::vector<ui8> data = std::vector<ui8>(LOOKUP_TEXTURE_SIZE, 0);
    };

    ui32 m_biomeCount = 0; ///< Number of biomes
    std::map<ui32, BiomeLookupTexture> m_biomeLookupMap; ///< To lookup biomes via color code
    std::vector<ui8> m_baseBiomeLookupTextureData; ///< Pixel data for baseBiomeLookupTexture

    PlanetGenData* m_defaultGenData = nullptr; ///< Default generation data handle

    vio::IOManager* m_iom = nullptr; ///< IOManager handle

    vg::TextureCache m_textureCache; ///< Texture cache for re-using textures

    NoiseShaderGenerator m_shaderGenerator; ///< Generates the generation shaders
};

#endif // PlanetLoader_h__

