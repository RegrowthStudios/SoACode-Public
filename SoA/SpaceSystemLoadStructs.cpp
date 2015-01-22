#include "stdafx.h"
#include "SpaceSystemLoadStructs.h"

KEG_ENUM_INIT_BEGIN(BodyType, BodyType, e);
e->addValue("planet", BodyType::PLANET);
e->addValue("star", BodyType::STAR);
e->addValue("gasGiant", BodyType::GAS_GIANT);
KEG_ENUM_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(SystemBodyKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, STRING, parent);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, STRING, path);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, eccentricity);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, period);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, startOrbit);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64_V3, orbitNormal);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, UI8_V4, pathColor);
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(PlanetKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, diameter);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, mass);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, STRING, displayName);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, STRING, generation);
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(StarKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, surfaceTemperature);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, diameter);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, mass);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F32_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, STRING, displayName);
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(Binary)
KEG_TYPE_INIT_ADD_MEMBER(Binary, STRING, name);
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("bodies", Keg::Value::array(offsetof(Binary, bodies), Keg::BasicType::C_STRING));
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(GasGiantKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F64, diameter);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F64, mass);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F32_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, STRING, displayName);
KEG_TYPE_INIT_END

bool Binary::containsBody(const SystemBody* body) {
    for (int i = 0; i < bodies.getLength(); i++) {
        if (bodies[i] == body->name) return true;
    }
    return false;
}