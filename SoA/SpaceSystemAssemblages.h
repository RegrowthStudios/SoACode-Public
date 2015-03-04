///
/// SpaceSystemAssemblages.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 13 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component and entity assemblages for SpaceSystem
///

#pragma once

#ifndef SpaceSystemAssemblages_h__
#define SpaceSystemAssemblages_h__

class SpaceSystem;

#include "VoxelCoordinateSpaces.h"
#include <Vorb/ecs/Entity.h>
#include <Vorb/VorbPreDecl.inl>

struct PlanetGenData;
class SoaState;
struct GasGiantKegProperties;
struct PlanetKegProperties;
struct StarKegProperties;
struct SystemBody;
struct SystemBodyKegProperties;
struct SphericalTerrainComponent;

DECL_VG(
    class GLProgram;
    class TextureRecycler;
)
DECL_VVOX(class VoxelMapData);

namespace SpaceSystemAssemblages {
    /************************************************************************/
    /* Entity Factories                                                     */
    /************************************************************************/
   
    /// Planet entity
    extern vcore::EntityID createPlanet(OUT SpaceSystem* spaceSystem,
                                        const SystemBodyKegProperties* sysProps,
                                        const PlanetKegProperties* properties,
                                        SystemBody* body);
    extern void destroyPlanet(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity);

    /// Star entity
    extern vcore::EntityID createStar(OUT SpaceSystem* spaceSystem,
                                        const SystemBodyKegProperties* sysProps,
                                        const StarKegProperties* properties,
                                        SystemBody* body);
    extern void destroyStar(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity);

    /// GasGiant entity
    extern vcore::EntityID createGasGiant(OUT SpaceSystem* spaceSystem,
                                        const SystemBodyKegProperties* sysProps,
                                        const GasGiantKegProperties* properties,
                                        SystemBody* body);
    extern void destroyGasGiant(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity);

    /************************************************************************/
    /* Component Factories                                                  */
    /************************************************************************/
    /// Spherical voxel component
    extern vcore::ComponentID addSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                         vcore::ComponentID sphericalTerrainComponent,
                                                         vcore::ComponentID farTerrainComponent,
                                                         vcore::ComponentID axisRotationComponent,
                                                         vcore::ComponentID namePositionComponent,
                                                         WorldCubeFace worldFace,
                                                         const SoaState* soaState);
    extern void removeSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity);

    /// Axis rotation component
    extern vcore::ComponentID addAxisRotationComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                         const f64q& axisOrientation,
                                                         f64 startAngle,
                                                         f64 rotationalPeriod);
    extern void removeAxisRotationComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity);

    /// Spherical terrain component
    extern vcore::ComponentID addSphericalTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                           vcore::ComponentID npComp,
                                                           vcore::ComponentID arComp,
                                                           PlanetGenData* planetGenData,
                                                           vg::GLProgram* normalProgram,
                                                           vg::TextureRecycler* normalMapRecycler);
    extern void removeSphericalTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity);

    /// Spherical terrain component
    extern vcore::ComponentID addFarTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                     SphericalTerrainComponent& parentCmp,
                                                     WorldCubeFace face);
    extern void removeFarTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity);

    /// Spherical Gravity component
    extern vcore::ComponentID addSphericalGravityComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                           vcore::ComponentID npComp, f64 radius, f64 mass);
    extern void removeSphericalGravityComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity);

    /// Name Position component
    extern vcore::ComponentID addNamePositionComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                           const nString& name, const f64v3& position);
    extern void removeNamePositionComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity);

    /// Orbit component
    extern vcore::ComponentID addOrbitComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                f64 eccentricity, f64 orbitalPeriod,
                                                const ui8v4& pathColor, const f64q& orientation);
    extern void removeOrbitComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity);

    /// Space Light Component
    extern vcore::ComponentID addSpaceLightComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                     vcore::ComponentID npCmp, color3 color, f32 intensity);
    extern void removeSpaceLightComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity);
}

#endif // SpaceSystemAssemblages_h__