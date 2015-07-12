#include "stdafx.h"
#include "SpaceSystemLoadStructs.h"

KEG_ENUM_DEF(SpaceBodyType, SpaceBodyType, kt) {
    kt.addValue("planet", SpaceBodyType::PLANET);
    kt.addValue("star", SpaceBodyType::STAR);
    kt.addValue("gasGiant", SpaceBodyType::GAS_GIANT);
}

KEG_ENUM_DEF(SpaceObjectType, SpaceObjectType, kt) {
    kt.addValue("Barycenter", SpaceObjectType::BARYCENTER);
    kt.addValue("Star", SpaceObjectType::STAR);
    kt.addValue("Planet", SpaceObjectType::PLANET);
    kt.addValue("DwarfPlanet", SpaceObjectType::DWARF_PLANET);
    kt.addValue("Moon", SpaceObjectType::MOON);
    kt.addValue("DwarfMoon", SpaceObjectType::DWARF_MOON);
    kt.addValue("Asteroid", SpaceObjectType::ASTEROID);
    kt.addValue("Comet", SpaceObjectType::COMET);
}

KEG_ENUM_DEF(TrojanType, TrojanType, kt) {
    kt.addValue("L4", TrojanType::L4);
    kt.addValue("L5", TrojanType::L5);
}

KEG_TYPE_DEF_SAME_NAME(SystemBodyProperties, kt) {
    kt.addValue("type", keg::Value::custom(offsetof(SystemBodyProperties, type), "SpaceObjectType", true));
    kt.addValue("trojan", keg::Value::custom(offsetof(SystemBodyProperties, trojan), "TrojanType", true));
    kt.addValue("comps", keg::Value::array(offsetof(SystemBodyProperties, comps), keg::BasicType::C_STRING));
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, par, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, path, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, ref, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, e, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, t, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, a, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, n, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, p, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, i, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, RA, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, dec, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, dist, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, td, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyProperties, tf, F64);
}

KEG_TYPE_DEF_SAME_NAME(AtmosphereProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, AtmosphereProperties, kr, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, AtmosphereProperties, km, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, AtmosphereProperties, g, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, AtmosphereProperties, scaleDepth, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, AtmosphereProperties, waveLength, F32_V3);
}

KEG_TYPE_DEF_SAME_NAME(PlanetRingProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetRingProperties, innerRadius, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetRingProperties, outerRadius, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetRingProperties, aTilt, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetRingProperties, lNorth, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetRingProperties, colorLookup, STRING);
}

KEG_TYPE_DEF_SAME_NAME(CloudsProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, CloudsProperties, color, F32_V3);
    KEG_TYPE_INIT_ADD_MEMBER(kt, CloudsProperties, scale, F32_V3);
    KEG_TYPE_INIT_ADD_MEMBER(kt, CloudsProperties, density, F32);
}

KEG_TYPE_DEF_SAME_NAME(PlanetProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetProperties, diameter, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetProperties, density, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetProperties, mass, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetProperties, aTilt, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetProperties, lNorth, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetProperties, rotationalPeriod, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetProperties, displayName, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetProperties, generation, STRING);
    kt.addValue("atmosphere", keg::Value::custom(offsetof(PlanetProperties, atmosphere), "AtmosphereKegProperties", false));
    kt.addValue("clouds", keg::Value::custom(offsetof(PlanetProperties, clouds), "CloudsKegProperties", false));
}

KEG_TYPE_DEF_SAME_NAME(StarProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarProperties, surfaceTemperature, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarProperties, diameter, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarProperties, density, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarProperties, mass, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarProperties, aTilt, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarProperties, lNorth, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarProperties, rotationalPeriod, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarProperties, displayName, STRING);
}

KEG_TYPE_DEF_SAME_NAME(GasGiantProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantProperties, diameter, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantProperties, density, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantProperties, mass, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantProperties, aTilt, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantProperties, lNorth, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantProperties, rotationalPeriod, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantProperties, oblateness, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantProperties, colorMap, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantProperties, displayName, STRING);
    kt.addValue("atmosphere", keg::Value::custom(offsetof(GasGiantProperties, atmosphere), "AtmosphereKegProperties", false));
    kt.addValue("rings", keg::Value::array(offsetof(GasGiantProperties, rings), keg::Value::custom(0, "PlanetRingKegProperties", false)));
}
