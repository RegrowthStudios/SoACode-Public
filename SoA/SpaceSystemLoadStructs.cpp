#include "stdafx.h"
#include "SpaceSystemLoadStructs.h"

KEG_ENUM_DEF(BodyType, BodyType, kt) {
    kt.addValue("planet", BodyType::PLANET);
    kt.addValue("star", BodyType::STAR);
    kt.addValue("gasGiant", BodyType::GAS_GIANT);
}

KEG_TYPE_DEF_SAME_NAME(SystemBodyKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyKegProperties, parent, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyKegProperties, path, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyKegProperties, e, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyKegProperties, t, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyKegProperties, meanAnomaly, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyKegProperties, o, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyKegProperties, p, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyKegProperties, i, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, SystemBodyKegProperties, pathColor, UI8_V4);
}

KEG_TYPE_DEF_SAME_NAME(PlanetKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetKegProperties, diameter, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetKegProperties, density, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetKegProperties, mass, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetKegProperties, axis, F64_V3);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetKegProperties, angularSpeed, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetKegProperties, rotationalPeriod, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetKegProperties, displayName, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PlanetKegProperties, generation, STRING);
}

KEG_TYPE_DEF_SAME_NAME(StarKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, surfaceTemperature, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, diameter, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, density, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, mass, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, axis, F32_V3);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, angularSpeed, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, StarKegProperties, displayName, STRING);
}

KEG_TYPE_DEF_SAME_NAME(Binary, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, Binary, name, STRING);
    kt.addValue("bodies", keg::Value::array(offsetof(Binary, bodies), keg::BasicType::C_STRING));
}

KEG_TYPE_DEF_SAME_NAME(GasGiantKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, diameter, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, density, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, mass, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, axis, F32_V3);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, angularSpeed, F64);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, oblateness, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, colorMap, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, GasGiantKegProperties, displayName, STRING);
}

bool Binary::containsBody(const SystemBody* body) {
    for (int i = 0; i < bodies.size(); i++) {
        if (bodies[i] == body->name) return true;
    }
    return false;
}