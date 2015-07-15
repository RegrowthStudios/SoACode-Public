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
class BlockPack;

typedef ui32 BiomeColorCode;

class PlanetGenLoader {
public:
    void init(vio::IOManager* ioManager);

    /// Loads a planet from file
    PlanetGenData* loadPlanetGenData(const nString& terrainPath);
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

    void loadFlora(const nString& filePath, PlanetGenData* genData);
    void loadTrees(const nString& filePath, PlanetGenData* genData);
    void loadBiomes(const nString& filePath, PlanetGenData* genData);

    void parseTerrainFuncs(NoiseBase* terrainFuncs, keg::ReadContext& context, keg::Node node);
    void parseLiquidColor(keg::ReadContext& context, keg::Node node, PlanetGenData* genData);
    void parseTerrainColor(keg::ReadContext& context, keg::Node node, PlanetGenData* genData);
    void parseBlockLayers(keg::ReadContext& context, keg::Node node, PlanetGenData* genData);

    PlanetGenData* m_defaultGenData = nullptr; ///< Default generation data handle

    vio::IOManager* m_iom = nullptr; ///< IOManager handle

    vg::TextureCache m_textureCache; ///< Texture cache for re-using textures

    vcore::RPCManager* m_glRpc = nullptr;

    PlanetGenerator m_planetGenerator;
};

#endif // PlanetLoader_h__

