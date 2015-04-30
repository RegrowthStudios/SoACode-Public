///
/// PlanetGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 16 Apr 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Generates random planet data
///

#pragma once

#ifndef PlanetGenerator_h__
#define PlanetGenerator_h__

#include "SpaceSystemLoadStructs.h"
#include <Vorb/graphics/gtypes.h>

struct PlanetGenData;

class PlanetGenerator {
public:
    static CALLEE_DELETE PlanetGenData* generateRandomPlanet(SpaceObjectType type);
private:
    static CALLEE_DELETE PlanetGenData* generatePlanet();
    static CALLEE_DELETE PlanetGenData* generateAsteroid();
    static CALLEE_DELETE PlanetGenData* generateComet();
    static VGTexture getRandomColorMap();
};

#endif // PlanetGenerator_h__
