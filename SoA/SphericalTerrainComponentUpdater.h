///
/// SphericalTerrainComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates SphericalTerrainComponents.
///

#pragma once

#ifndef SphericalTerrainComponentUpdater_h__
#define SphericalTerrainComponentUpdater_h__

class SpaceSystem;
class SphericalTerrainComponent;

class SphericalTerrainComponentUpdater {
public:
    void update(SpaceSystem* spaceSystem, const f64v3& cameraPos);

    /// Updates openGL specific stuff. Call on render thread
    void glUpdate(SpaceSystem* spaceSystem);

private:
    void initPatches(SphericalTerrainComponent& cmp);
};

#endif // SphericalTerrainComponentUpdater_h__