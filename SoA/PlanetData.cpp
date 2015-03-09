#include "stdafx.h"
#include "PlanetData.h"

KEG_TYPE_DEF_SAME_NAME(LiquidColorKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, colorPath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, texturePath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, tint, UI8_V3);
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, depthScale, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, freezeTemp, F32);
}

KEG_TYPE_DEF_SAME_NAME(TerrainColorKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainColorKegProperties, colorPath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainColorKegProperties, texturePath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainColorKegProperties, tint, UI8_V3);
}

KEG_ENUM_DEF(TerrainFunction, TerrainFunction, kt) {
    kt.addValue("noise", TerrainFunction::NOISE);
    kt.addValue("ridged", TerrainFunction::RIDGED_NOISE);
}

KEG_TYPE_DEF_SAME_NAME(TerrainFuncKegProperties, kt) {
    kt.addValue("type", keg::Value::custom(offsetof(TerrainFuncKegProperties, func), "TerrainFunction", true));
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, octaves, I32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, persistence, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, frequency, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, low, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, high, F32);
    kt.addValue("children", keg::Value::array(offsetof(TerrainFuncKegProperties, children), keg::Value::custom(0, "TerrainFuncKegProperties", false)));
}
KEG_TYPE_DEF_SAME_NAME(NoiseBase, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, NoiseBase, base, F32);
    kt.addValue("funcs", keg::Value::array(offsetof(NoiseBase, funcs), keg::Value::custom(0, "TerrainFuncKegProperties", false)));
}
