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
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/ecs/Entity.h>
#include <Vorb/graphics/gtypes.h>

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

enum class SpaceObjectType;

namespace SpaceSystemAssemblages {
    /************************************************************************/
    /* Entity Factories                                                     */
    /************************************************************************/
   
    // Plain orbit entity
    extern vecs::EntityID createOrbit(SpaceSystem* spaceSystem,
                                       const SystemBodyKegProperties* sysProps,
                                       SystemBody* body, f64 bodyRadius);

    /// Planet entity
    extern vecs::EntityID createPlanet(SpaceSystem* spaceSystem,
                                        const SystemBodyKegProperties* sysProps,
                                        const PlanetKegProperties* properties,
                                        SystemBody* body);
    extern void destroyPlanet(SpaceSystem* gameSystem, vecs::EntityID planetEntity);

    /// Star entity
    extern vecs::EntityID createStar(SpaceSystem* spaceSystem,
                                        const SystemBodyKegProperties* sysProps,
                                        const StarKegProperties* properties,
                                        SystemBody* body);
    extern void destroyStar(SpaceSystem* gameSystem, vecs::EntityID planetEntity);

    /// GasGiant entity
    extern vecs::EntityID createGasGiant(SpaceSystem* spaceSystem,
                                        const SystemBodyKegProperties* sysProps,
                                        const GasGiantKegProperties* properties,
                                        SystemBody* body,
                                        VGTexture colorMap);
    extern void destroyGasGiant(SpaceSystem* gameSystem, vecs::EntityID planetEntity);

    /************************************************************************/
    /* Component Assemblages                                                */
    /************************************************************************/
    /// Atmosphere component
    extern vecs::ComponentID addAtmosphereComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                    vecs::ComponentID namePositionComponent, f32 planetRadius,
                                                    f32 radius, f32 kr, f32 km, f32 g, f32 scaleDepth,
                                                    f32v3 wavelength, f32 oblateness = 0.0f);
    extern void removeAtmosphereComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Spherical voxel component
    extern vecs::ComponentID addSphericalVoxelComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                         vecs::ComponentID sphericalTerrainComponent,
                                                         vecs::ComponentID farTerrainComponent,
                                                         vecs::ComponentID axisRotationComponent,
                                                         vecs::ComponentID namePositionComponent,
                                                         WorldCubeFace worldFace,
                                                         const SoaState* soaState);
    extern void removeSphericalVoxelComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Axis rotation component
    extern vecs::ComponentID addAxisRotationComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                         const f64q& axisOrientation,
                                                         f64 startAngle,
                                                         f64 rotationalPeriod);
    extern void removeAxisRotationComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Spherical terrain component
    extern vecs::ComponentID addSphericalTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                           vecs::ComponentID npComp,
                                                           vecs::ComponentID arComp,
                                                           f64 radius,
                                                           PlanetGenData* planetGenData,
                                                           vg::GLProgram* normalProgram,
                                                           vg::TextureRecycler* normalMapRecycler);
    extern void removeSphericalTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Star Component
    extern vecs::ComponentID addStarComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                              vecs::ComponentID npComp,
                                              vecs::ComponentID arComp,
                                              f64 mass,
                                              f64 radius,
                                              f64 temperature);
    extern void removeStarComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Gas giant component
    extern vecs::ComponentID addGasGiantComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                  vecs::ComponentID npComp,
                                                  vecs::ComponentID arComp,
                                                  f32 oblateness,
                                                  f64 radius,
                                                  VGTexture colorMap);
    extern void removeGasGiantComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Far terrain component
    extern vecs::ComponentID addFarTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                     SphericalTerrainComponent& parentCmp,
                                                     WorldCubeFace face);
    extern void removeFarTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Spherical Gravity component
    extern vecs::ComponentID addSphericalGravityComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                           vecs::ComponentID npComp, f64 radius, f64 mass);
    extern void removeSphericalGravityComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Name Position component
    extern vecs::ComponentID addNamePositionComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                           const nString& name, const f64v3& position);
    extern void removeNamePositionComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Orbit component
    extern vecs::ComponentID addOrbitComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                               vecs::ComponentID npComp, SpaceObjectType oType,
                                               f64 eccentricity, f64 orbitalPeriod,
                                               f64 ascendingLong, f64 periapsisLong,
                                               f64 inclination, f64 trueAnomaly);
    extern void removeOrbitComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);

    /// Space Light Component
    extern vecs::ComponentID addSpaceLightComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                     vecs::ComponentID npCmp, color3 color, f32 intensity);
    extern void removeSpaceLightComponent(SpaceSystem* spaceSystem, vecs::EntityID entity);
}

#endif // SpaceSystemAssemblages_h__