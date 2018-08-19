///
/// FrustumComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 25 Jan 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Updates frustum components
///

#pragma once

#ifndef FrustumComponentUpdater_h__
#define FrustumComponentUpdater_h__

#include "Vorb/decorators.h"

class GameSystem;

class FrustumComponentUpdater {
public:
    /// Updates frustum components
    /// @param gameSystem: Game ECS
    void update(OUT GameSystem* gameSystem);
};

#endif // FrustumComponentUpdater_h__
