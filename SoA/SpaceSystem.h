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

#include "AxisRotationComponent.h"
#include "ComponentTable.hpp"
#include "ECS.h"
#include "IOManager.h"
#include "NamePositionComponent.h"
#include "OrbitComponent.h"
#include "SphericalTerrainComponent.h"

#define SPACE_SYSTEM_CT_NAMEPOSITIION_NAME "NamePosition"
#define SPACE_SYSTEM_CT_AXISROTATION_NAME "AxisRotation"
#define SPACE_SYSTEM_CT_ORBIT_NAME "Orbit"
#define SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME "SphericalTerrain"

class PlanetKegProperties;
class SystemKegProperties;
class Camera;

class SystemBody {
public:
    nString name = "";
    SystemBody* parent = nullptr;
    vcore::Entity entity;
};

class SpaceSystem : public vcore::ECS {
public:
    SpaceSystem();

    void addPlanet(const nString& filePath);

    /// Updates the space system
    /// @param time: The time in seconds
    void update(double time);
    
    /// Renders the space system
    /// @param camera: Camera for rendering
    void draw(const Camera* camera);

    /// Adds a solar system and all its bodies to the system
    /// @param filePath: Path to the solar system directory
    void addSolarSystem(const nString& filePath);

protected:

    bool loadSystemProperties(const cString filePath, SystemKegProperties& result);

    bool loadPlanetProperties(const cString filePath, PlanetKegProperties& result);

    vcore::ComponentTable<NamePositionComponent> m_namePositionCT;
    vcore::ComponentTable<AxisRotationComponent> m_axisRotationCT;
    vcore::ComponentTable<OrbitComponent> m_orbitCT;
    vcore::ComponentTable<SphericalTerrainComponent> m_sphericalTerrainCT;

    IOManager m_ioManager;

    std::map<nString, SystemBody*> m_systemBodies;
};

#endif // SpaceSystem_h__