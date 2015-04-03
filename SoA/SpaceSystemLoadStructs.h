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

struct SystemBodyKegProperties {
    nString parent = "";
    nString path = "";
    f64 eccentricity = 0.0;
    f64 period = 0.0;
    f64 startOrbit = 0.0;
    f64v3 orbitNormal = f64v3(1.0, 0.0, 0.0);
    ui8v4 pathColor = ui8v4(255);
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
};
KEG_TYPE_DECL(GasGiantKegProperties);

#endif // SpaceSystemLoadStructs_h__