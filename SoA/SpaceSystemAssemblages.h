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
    extern vecs::EntityID createPlanet(OUT SpaceSystem* spaceSystem,
                                        const SystemBodyKegProperties* sysProps,
                                        const PlanetKegProperties* properties,
                                        SystemBody* body);
    extern void destroyPlanet(OUT SpaceSystem* gameSystem, vecs::EntityID planetEntity);

    /// Star entity
    extern vecs::EntityID createStar(OUT SpaceSystem* spaceSystem,
                                        const SystemBodyKegProperties* sysProps,
                                        const StarKegProperties* properties,
                                        SystemBody* body);
    extern void destroyStar(OUT SpaceSystem* gameSystem, vecs::EntityID planetEntity);

    /// GasGiant entity
    extern vecs::EntityID createGasGiant(OUT SpaceSystem* spaceSystem,
                                        const SystemBodyKegProperties* sysProps,
                                        const GasGiantKegProperties* properties,
                                        SystemBody* body);
    extern void destroyGasGiant(OUT SpaceSystem* gameSystem, vecs::EntityID planetEntity);

    /************************************************************************/
    /* Component Assemblages                                                */
    /************************************************************************/
    /// Atmosphere component
    extern vecs::ComponentID addAtmosphereComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                    vecs::ComponentID namePositionComponent, f32 planetRadius,
                                                    f32 radius);
    extern void removeAtmosphereComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Spherical voxel component
    extern vecs::ComponentID addSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                         vecs::ComponentID sphericalTerrainComponent,
                                                         vecs::ComponentID farTerrainComponent,
                                                         vecs::ComponentID axisRotationComponent,
                                                         vecs::ComponentID namePositionComponent,
                                                         WorldCubeFace worldFace,
                                                         const SoaState* soaState);
    extern void removeSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Axis rotation component
    extern vecs::ComponentID addAxisRotationComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                         const f64q& axisOrientation,
                                                         f64 startAngle,
                                                         f64 rotationalPeriod);
    extern void removeAxisRotationComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Spherical terrain component
    extern vecs::ComponentID addSphericalTerrainComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                           vecs::ComponentID npComp,
                                                           vecs::ComponentID arComp,
                                                           PlanetGenData* planetGenData,
                                                           vg::GLProgram* normalProgram,
                                                           vg::TextureRecycler* normalMapRecycler);
    extern void removeSphericalTerrainComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Spherical terrain component
    extern vecs::ComponentID addFarTerrainComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                     SphericalTerrainComponent& parentCmp,
                                                     WorldCubeFace face);
    extern void removeFarTerrainComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Spherical Gravity component
    extern vecs::ComponentID addSphericalGravityComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                           vecs::ComponentID npComp, f64 radius, f64 mass);
    extern void removeSphericalGravityComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Name Position component
    extern vecs::ComponentID addNamePositionComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                           const nString& name, const f64v3& position);
    extern void removeNamePositionComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Orbit component
    extern vecs::ComponentID addOrbitComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                f64 eccentricity, f64 orbitalPeriod,
                                                const ui8v4& pathColor, const f64q& orientation);
    extern void removeOrbitComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Space Light Component
    extern vecs::ComponentID addSpaceLightComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                     vecs::ComponentID npCmp, color3 color, f32 intensity);
    extern void removeSpaceLightComponent(OUT SpaceSystem* spaceSystem, vecs::EntityID entity);
}

#endif // SpaceSystemAssemblages_h__