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
#include "GLProgram.h"
#include "IOManager.h"
#include "NamePositionComponent.h"
#include "OrbitComponent.h"
#include "SphericalGravityComponent.h"
#include "SphericalTerrainComponent.h"

#define SPACE_SYSTEM_CT_NAMEPOSITIION_NAME "NamePosition"
#define SPACE_SYSTEM_CT_AXISROTATION_NAME "AxisRotation"
#define SPACE_SYSTEM_CT_ORBIT_NAME "Orbit"
#define SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME "SphericalTerrain"

class Binary;
class Camera;
class PlanetKegProperties;
class StarKegProperties;
class SystemBodyKegProperties;

enum class BodyType {
    NONE,
    PLANET,
    STAR,
    GAS_GIANT
};

class SystemBody {
public:
    ~SystemBody() { delete entity; }

    nString name = "";
    nString parentName = "";
    SystemBody* parent = nullptr;
    vcore::Entity* entity = nullptr;
    BodyType type = BodyType::NONE;
};

class SpaceSystem : public vcore::ECS {
public:
    SpaceSystem();

    /// Updates the space system
    /// @param time: The time in seconds
    void update(double time);
    
    /// Renders the space bodies
    /// @param camera: Camera for rendering
    void drawBodies(const Camera* camera) const;

    /// Renders the space paths
    /// @param camera: Camera for rendering
    /// @param colorProgram: glProgram for basic color
    void drawPaths(const Camera* camera, vg::GLProgram* colorProgram);

    /// Adds a solar system and all its bodies to the system
    /// @param filePath: Path to the solar system directory
    void addSolarSystem(const nString& filePath);

protected:
    bool loadBodyProperties(const nString& filePath, const SystemBodyKegProperties* sysProps, SystemBody* body);

    void addPlanet(const SystemBodyKegProperties* sysProps, const PlanetKegProperties* properties, SystemBody* body);

    void addStar(const SystemBodyKegProperties* sysProps, const StarKegProperties* properties, SystemBody* body);

    bool loadSystemProperties(const nString& dirPath);

    void calculateOrbit(vcore::EntityID entity, f64 parentMass);

    vcore::ComponentTable<NamePositionComponent> m_namePositionCT;
    vcore::ComponentTable<AxisRotationComponent> m_axisRotationCT;
    vcore::ComponentTable<OrbitComponent> m_orbitCT;
    vcore::ComponentTable<SphericalGravityComponent> m_sphericalGravityCT;
    vcore::ComponentTable<SphericalTerrainComponent> m_sphericalTerrainCT;

    IOManager m_ioManager;

    std::map<nString, Binary*> m_binaries; ///< Contains all binary systems
    std::map<nString, SystemBody*> m_systemBodies; ///< Contains all system bodies

    nString m_systemDescription; ///< textual description of the system
};

#endif // SpaceSystem_h__