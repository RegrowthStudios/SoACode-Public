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

class PlanetGenerationData {
public:
    VGTexture surfaceColorMap;
    VGTexture watercolorMap;
    std::vector<VGTexture> biomeMaps;
    vg::GLProgram* program;
};

class PlanetLoader {
public:
    PlanetLoader();
    ~PlanetLoader();

    PlanetGenerationData* loadPlanet(const nString& filePath);
};

#endif // PlanetLoader_h__

