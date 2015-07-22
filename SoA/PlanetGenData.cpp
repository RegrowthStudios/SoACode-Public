#include "stdafx.h"
#include "PlanetGenData.h"

KEG_TYPE_DEF_SAME_NAME(BlockLayerKegProperties, kt) {
    using namespace keg;
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockLayerKegProperties, block, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockLayerKegProperties, surface, STRING);
    kt.addValue("width", Value::basic(offsetof(BlockLayerKegProperties, width), BasicType::UI32));
}

KEG_TYPE_DEF_SAME_NAME(LiquidColorKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, colorPath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, texturePath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, tint, UI8_V3);
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, depthScale, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, LiquidColorKegProperties, freezeTemp, F32);
}

KEG_TYPE_DEF_SAME_NAME(TerrainColorKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainColorKegProperties, colorPath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainColorKegProperties, grassTexturePath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainColorKegProperties, rockTexturePath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainColorKegProperties, tint, UI8_V3);
}
