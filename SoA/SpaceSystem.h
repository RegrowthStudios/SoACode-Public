///
/// SpaceSystem.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Implementation of a Star System with ECS
///

#pragma once

#ifndef SpaceSystem_h__
#define SpaceSystem_h__

#include "SpaceSystemComponents.h"
#include "SphericalVoxelComponentTable.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/ecs/ECS.h>

#define SPACE_SYSTEM_CT_NAMEPOSITIION_NAME "NamePosition"
#define SPACE_SYSTEM_CT_AXISROTATION_NAME "AxisRotation"
#define SPACE_SYSTEM_CT_ORBIT_NAME "Orbit"
#define SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME "SphericalTerrain"
#define SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME "SphericalGravity"
#define SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME "SphericalVoxel"

class App;
class Binary;
class Camera;
class GameSystem;
class GasGiantKegProperties;
class PlanetKegProperties;
class PlanetLoader;
class SoaState;
class SpriteBatch;
class SpriteFont;
class StarKegProperties;
class SystemBodyKegProperties;
struct SystemBody;

namespace vorb {
    namespace core {
        namespace graphics {
            class TextureRecycler;
            class GLProgramManager;
        }
    }
}

//TODO(Ben): This should be POD, split it up
class SpaceSystem : public vcore::ECS {
    friend class SpaceSystemRenderStage;
    friend class MainMenuSystemViewer;
public:
    SpaceSystem();
    ~SpaceSystem();

    /// TEMPORARY
    void init(vg::GLProgramManager* glProgramManager) { this->glProgramManager = glProgramManager; }

    /// Adds a solar system and all its bodies to the system
    /// @param filePath: Path to the solar system directory
    void addSolarSystem(const nString& filePath);

    vcore::ComponentTable<NamePositionComponent> m_namePositionCT;
    vcore::ComponentTable<AxisRotationComponent> m_axisRotationCT;
    vcore::ComponentTable<OrbitComponent> m_orbitCT;
    vcore::ComponentTable<SphericalGravityComponent> m_sphericalGravityCT;
    vcore::ComponentTable<SphericalTerrainComponent> m_sphericalTerrainCT;
    SphericalVoxelComponentTable m_sphericalVoxelCT;

    nString systemDescription; ///< textual description of the system
    vg::GLProgramManager* glProgramManager; ///< TEMPORARY
    vg::TextureRecycler* normalMapRecycler = nullptr; ///< For recycling normal maps

protected:
    void addPlanet(const SystemBodyKegProperties* sysProps, const PlanetKegProperties* properties, SystemBody* body);

    void addStar(const SystemBodyKegProperties* sysProps, const StarKegProperties* properties, SystemBody* body);

    void addGasGiant(const SystemBodyKegProperties* sysProps, const GasGiantKegProperties* properties, SystemBody* body);

    vio::IOManager m_ioManager;
};

#endif // SpaceSystem_h__
