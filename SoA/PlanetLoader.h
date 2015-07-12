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

#include "PlanetGenerator.h"
#include "SpaceSystemLoadStructs.h"

DECL_VIO(class IOManager);
DECL_VCORE(class RPCManager);

struct NoiseBase;
struct PlanetGenData;
struct BiomeKegProperties;

typedef ui32 BiomeColorCode;

class PlanetLoader {
public:
    void init(vio::IOManager* ioManager);

    /// Loads a planet from file
    /// @param filePath: Path of the planet
    /// @param glrpc: Optional RPC if you want to load on a non-render thread
    /// @return planet gen data
    PlanetGenData* loadPlanet(const nString& filePath, vcore::RPCManager* glrpc = nullptr);
    /// Returns a default planetGenData
    /// @param glrpc: Optional RPC if you want to load on a non-render thread
    /// @return planet gen data
    PlanetGenData* getDefaultGenData(vcore::RPCManager* glrpc = nullptr);
    /// Returns a default planetGenData
    /// @param glrpc: Optional RPC if you want to load on a non-render thread
    /// @return planet gen data
    PlanetGenData* getRandomGenData(f32 radius, vcore::RPCManager* glrpc = nullptr);
    AtmosphereProperties getRandomAtmosphere();

private:
    /// Loads the biomes from file
    /// @param filePath: Path to the biome file
    /// @param genData: generation data to be modified
    void loadBiomes(const nString& filePath, OUT PlanetGenData* genData);
    /// Parses terrain noise functions
    /// @param terrainFuncs: The functions to parse
    /// @param reader: The YAML reader
    /// @param node: The YAML node
    void parseTerrainFuncs(NoiseBase* terrainFuncs, keg::ReadContext& context, keg::Node node);
    /// Parses liquid color data
    /// @param reader: The YAML reader
    /// @param node: The YAML node
    /// @param genData: The generation data to modify
    void parseLiquidColor(keg::ReadContext& context, keg::Node node, OUT PlanetGenData* genData);
    /// Parses terrain color data
    /// @param reader: The YAML reader
    /// @param node: The YAML node
    /// @param genData: The generation data to modify
    void parseTerrainColor(keg::ReadContext& context, keg::Node node, OUT PlanetGenData* genData);
    /// Parses block layer data
    /// @param reader: The YAML reader
    /// @param node: The YAML node
    /// @param genData: The generation data to modify
    void parseBlockLayers(keg::ReadContext& context, keg::Node node, OUT PlanetGenData* genData);

    PlanetGenData* m_defaultGenData = nullptr; ///< Default generation data handle

    vio::IOManager* m_iom = nullptr; ///< IOManager handle

    vg::TextureCache m_textureCache; ///< Texture cache for re-using textures

    vcore::RPCManager* m_glRpc = nullptr;

    PlanetGenerator m_planetGenerator;
};

#endif // PlanetLoader_h__

