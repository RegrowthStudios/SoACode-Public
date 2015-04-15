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
#include "SpaceSystemComponentTables.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/ecs/ECS.h>
#include <Vorb/VorbPreDecl.inl>

#define SPACE_SYSTEM_CT_NAMEPOSITIION_NAME "NamePosition"
#define SPACE_SYSTEM_CT_AXISROTATION_NAME "AxisRotation"
#define SPACE_SYSTEM_CT_ORBIT_NAME "Orbit"
#define SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME "SphericalTerrain"
#define SPACE_SYSTEM_CT_FARTERRAIN_NAME "FarTerrain"
#define SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME "SphericalGravity"
#define SPACE_SYSTEM_CT_GASGIANT_NAME "GasGiant"
#define SPACE_SYSTEM_CT_STAR_NAME "Star"
#define SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME "SphericalVoxel"
#define SPACE_SYSTEM_CT_SPACELIGHT_NAME "SpaceLight"
#define SPACE_SYSTEM_CT_ATMOSPHERE_NAME "Atmosphere"

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

DECL_VG(class TextureRecycler)
DECL_VG(class GLProgram)

//TODO(Ben): This should be POD, split it up
class SpaceSystem : public vecs::ECS {
    friend class SpaceSystemRenderStage;
    friend class MainMenuSystemViewer;
public:
    SpaceSystem();
    ~SpaceSystem();

    vecs::ComponentTable<NamePositionComponent> m_namePositionCT;
    vecs::ComponentTable<AxisRotationComponent> m_axisRotationCT;
    OrbitComponentTable m_orbitCT;
    vecs::ComponentTable<SphericalGravityComponent> m_sphericalGravityCT;
    SphericalTerrainComponentTable m_sphericalTerrainCT;
    vecs::ComponentTable<GasGiantComponent> m_gasGiantCT;
    vecs::ComponentTable<StarComponent> m_starCT;
    vecs::ComponentTable<FarTerrainComponent> m_farTerrainCT;
    vecs::ComponentTable<SpaceLightComponent> m_spaceLightCT;
    vecs::ComponentTable<AtmosphereComponent> m_atmosphereCT;
    SphericalVoxelComponentTable m_sphericalVoxelCT;

    nString systemDescription; ///< textual description of the system
    std::unique_ptr<vg::TextureRecycler> normalMapRecycler = nullptr; ///< For recycling normal maps
    std::unique_ptr<vg::GLProgram> normalMapGenProgram = nullptr; ///< For generating normal maps

protected:
    void addPlanet(const SystemBodyKegProperties* sysProps, const PlanetKegProperties* properties, SystemBody* body);

    void addStar(const SystemBodyKegProperties* sysProps, const StarKegProperties* properties, SystemBody* body);

    void addGasGiant(const SystemBodyKegProperties* sysProps, const GasGiantKegProperties* properties, SystemBody* body);

    vio::IOManager m_ioManager;
};

#endif // SpaceSystem_h__
