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

KEG_TYPE_DEF_SAME_NAME(StarKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, surfaceTemperature, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, diameter, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, density, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, mass, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, aTilt, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, lNorth, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, rotationalPeriod, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, displayName, STRING);
}

KEG_TYPE_DEF_SAME_NAME(GasGiantKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, diameter, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, density, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, mass, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, aTilt, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, lNorth, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, rotationalPeriod, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, oblateness, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, colorMap, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, displayName, STRING);
    kt.addValue("atmosphere", keg::Value::custom(offsetof(GasGiantKegProperties, atmosphere), "AtmosphereKegProperties", false));
    kt.addValue("rings", keg::Value::array(offsetof(GasGiantKegProperties, rings), keg::Value::custom(0, "PlanetRingKegProperties", false)));
}
