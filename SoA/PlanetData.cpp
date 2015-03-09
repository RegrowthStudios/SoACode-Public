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

KEG_ENUM_DEF(TerrainWaveform, TerrainWaveform, kt) {
    kt.addValue("noise", TerrainWaveform::NOISE);
    kt.addValue("ridged", TerrainWaveform::RIDGED_NOISE);
    kt.addValue("abs", TerrainWaveform::ABS_NOISE);
    kt.addValue("squared", TerrainWaveform::SQUARED_NOISE);
    kt.addValue("cubed", TerrainWaveform::CUBED_NOISE);
}

KEG_ENUM_DEF(TerrainOp, TerrainOp, kt) {
    kt.addValue("add", TerrainOp::ADD);
    kt.addValue("sub", TerrainOp::SUB);
    kt.addValue("mul", TerrainOp::MUL);
    kt.addValue("div", TerrainOp::DIV);
}

KEG_TYPE_DEF_SAME_NAME(TerrainFuncKegProperties, kt) {
    kt.addValue("type", keg::Value::custom(offsetof(TerrainFuncKegProperties, func), "TerrainFunction", true));
    kt.addValue("op", keg::Value::custom(offsetof(TerrainFuncKegProperties, op), "TerrainOp", true));
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, octaves, I32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, persistence, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, frequency, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, low, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, high, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncKegProperties, clamp, F32_V2);
    kt.addValue("children", keg::Value::array(offsetof(TerrainFuncKegProperties, children), keg::Value::custom(0, "TerrainFuncKegProperties", false)));
}
KEG_TYPE_DEF_SAME_NAME(NoiseBase, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, NoiseBase, base, F32);
    kt.addValue("funcs", keg::Value::array(offsetof(NoiseBase, funcs), keg::Value::custom(0, "TerrainFuncKegProperties", false)));
}
