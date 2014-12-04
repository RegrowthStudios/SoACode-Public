///
/// SpaceSystem.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 9 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Let there be light!
///

#pragma once

#ifndef SpaceSystem_h__
#define SpaceSystem_h__

#include "ComponentTable.hpp"
#include "AxisRotationComponent.h"
#include "ECS.h"
#include "NamePositionComponent.h"
#include "OrbitComponent.h"
#include "SphericalTerrainComponent.h"

#define SPACE_SYSTEM_CT_NAMEPOSITIION_NAME "NamePosition"
#define SPACE_SYSTEM_CT_AXISROTATION_NAME "AxisRotation"
#define SPACE_SYSTEM_CT_ORBIT_NAME "Orbit"
#define SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME "SphericalTerrain"

class PlanetKegProperties;
class Camera;

class SpaceSystem : public vcore::ECS {
public:
    void addPlanet(const nString& filePath);

    /// Updates the space system
    /// @param time: The time in seconds
    void update(double time);
    
    /// Renders the space system
    /// @param camera: Camera for rendering
    void draw(const Camera* camera);

protected:

    bool loadPlanetProperties(const cString filePath, PlanetKegProperties& result);

    vcore::ComponentTable<NamePositionComponent> m_namePositionCT;
    vcore::ComponentTable<AxisRotationComponent> m_axisRotationCT;
    vcore::ComponentTable<OrbitComponent> m_orbitCT;
    vcore::ComponentTable<SphericalTerrainComponent> m_sphericalTerrainCT;
};

#endif // SpaceSystem_h__