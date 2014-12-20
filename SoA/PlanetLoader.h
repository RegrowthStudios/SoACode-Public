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

class IOManager;

class PlanetGenerationData {
public:
    VGTexture surfaceColorMap;
    VGTexture watercolorMap;
    std::vector<VGTexture> biomeMaps;
    vg::GLProgram* program;
};

class PlanetLoader {
public:
    PlanetLoader(IOManager* ioManager);
    ~PlanetLoader();

    PlanetGenerationData* loadPlanet(const nString& filePath);
private:
    void parseTerrainFuncs(TerrainFuncs& terrainFuncs, YAML::Node& node);

    IOManager* m_iom;
};

#endif // PlanetLoader_h__

