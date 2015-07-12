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
#include <Vorb/graphics/GLProgram.h>

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
#define SPACE_SYSTEM_CT_PLANETRINGS_NAME "PlanetRings"
#define SPACE_SYSTEM_CT_CLOUDS_NAME "Clouds"

class App;
class Binary;
class Camera;
class GameSystem;
class PlanetLoader;
struct SoaState;
class SpriteBatch;
class SpriteFont;
struct GasGiantKegProperties;
struct PlanetProperties;
struct StarKegProperties;
struct SystemBody;
struct SystemBodyProperties;

DECL_VG(class TextureRecycler)

class SpaceSystem : public vecs::ECS {
    friend class SpaceSystemRenderStage;
    friend class MainMenuSystemViewer;
public:
    SpaceSystem();
    ~SpaceSystem();

    vecs::ComponentTable<NamePositionComponent> namePosition;
    vecs::ComponentTable<AxisRotationComponent> axisRotation;
    OrbitComponentTable orbit;
    vecs::ComponentTable<SphericalGravityComponent> sphericalGravity;
    SphericalTerrainComponentTable sphericalTerrain;
    vecs::ComponentTable<GasGiantComponent> gasGiant;
    vecs::ComponentTable<StarComponent> star;
    FarTerrainComponentTable farTerrain;
    vecs::ComponentTable<SpaceLightComponent> spaceLight;
    vecs::ComponentTable<AtmosphereComponent> atmosphere;
    vecs::ComponentTable<PlanetRingsComponent> planetRings;
    vecs::ComponentTable<CloudsComponent> clouds;
    SphericalVoxelComponentTable sphericalVoxel;
    
    f32 age = 0.0f; ///< age of the system
    nString systemDescription = "No description"; ///< textual description of the system
    
    std::map<nString, std::pair<f32v4, f32v4> > pathColorMap; ///< Map of body type to path colors
};

#endif // SpaceSystem_h__
