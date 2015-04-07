///
/// SpaceSystemLoadStructs.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 20 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Structs for SpaceSystem initialization
///

#pragma once

#ifndef SpaceSystemLoadStructs_h__
#define SpaceSystemLoadStructs_h__

struct PlanetGenData;

#include <Vorb/io/Keg.h>
#include <Vorb/ecs/Entity.h>

enum class BodyType {
    NONE,
    PLANET,
    STAR,
    GAS_GIANT
};
KEG_TYPE_DECL(BodyType);

struct SystemBody {
    nString name = "";
    nString parentName = "";
    SystemBody* parent = nullptr;
    vecs::EntityID entity = 0;
    BodyType type = BodyType::NONE;
};

struct AtmosphereKegProperties {
    f32 kr = 0.0025f;
    f32 km = 0.0020f;
    f32 g = -0.99f;
    f32 scaleDepth = 0.25f;
    f32v3 waveLength = f32v3(0.65, 0.57, 0.475);
};
KEG_TYPE_DECL(AtmosphereKegProperties);

struct SystemBodyKegProperties {
    nString parent = "";
    nString path = "";
    f64 e = 0.0; ///< Shape of orbit, 0-1
    f64 t = 0.0; ///< Period of a full orbit in sec
    f64 trueAnomaly = 0.0; ///< Start true anomaly in deg
    f64 o = 0.0; ///< Longitude of the ascending node in deg
    f64 p = 0.0; ///< Longitude of the periapsis in deg
    f64 i = 0.0; ///< Inclination in deg
    ui8v4 pathColor = ui8v4(255); ///< Color of rendered path
};
KEG_TYPE_DECL(SystemBodyKegProperties);

struct PlanetKegProperties {
    f64 diameter = 0.0;
    f64 density = 0.0;
    f64 mass = 0.0;
    f64v3 axis;
    f64 angularSpeed = 0.0; // TODO(Ben): Remove
    f64 rotationalPeriod = 0.0;
    nString displayName = "";
    nString generation = "";
    PlanetGenData* planetGenData = nullptr;
    AtmosphereKegProperties atmosphere;
};
KEG_TYPE_DECL(PlanetKegProperties);

struct StarKegProperties {
    f64 surfaceTemperature = 0.0; ///< temperature in kelvin
    f64 diameter = 0.0;
    f64 density = 0.0;
    f64 mass = 0.0;
    f64v3 axis;
    f64 angularSpeed = 0.0;
    f64 rotationalPeriod = 0.0;
    nString displayName = "";
};
KEG_TYPE_DECL(StarKegProperties);

struct Binary {

    bool containsBody(const SystemBody* body); //TODO: no

    nString name;
    Array<const char*> bodies; ///< Temporary due to bug
    f64 mass = 0.0;
};
KEG_TYPE_DECL(Binary);

struct GasGiantKegProperties {
    f64 diameter = 0.0;
    f64 density = 0.0;
    f64 mass = 0.0;
    f64v3 axis;
    f64 angularSpeed = 0.0;
    f64 rotationalPeriod = 0.0;
    f32 oblateness = 0.0;
    nString colorMap = "";
    nString displayName = "";
    AtmosphereKegProperties atmosphere;
};
KEG_TYPE_DECL(GasGiantKegProperties);

#endif // SpaceSystemLoadStructs_h__