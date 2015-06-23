#include "stdafx.h"
#include "BlockMaterial.h"

KEG_ENUM_DEF(ConnectedTextureReducedMethod, ConnectedTextureReducedMethod, e) {
    e.addValue("none", ConnectedTextureReducedMethod::NONE);
    e.addValue("top", ConnectedTextureReducedMethod::TOP);
    e.addValue("bottom", ConnectedTextureReducedMethod::BOTTOM);
}
KEG_ENUM_DEF(BlendType, BlendType, e) {
    e.addValue("add", BlendType::ADD);
    e.addValue("multiply", BlendType::MULTIPLY);
    e.addValue("replace", BlendType::ALPHA);
    e.addValue("subtract", BlendType::SUBTRACT);
}

KEG_ENUM_DEF(ConnectedTextureMethods, ConnectedTextureMethods, e) {
    e.addValue("none", ConnectedTextureMethods::NONE);
    e.addValue("connect", ConnectedTextureMethods::CONNECTED);
    e.addValue("random", ConnectedTextureMethods::RANDOM);
    e.addValue("repeat", ConnectedTextureMethods::REPEAT);
    e.addValue("grass", ConnectedTextureMethods::GRASS);
    e.addValue("horizontal", ConnectedTextureMethods::HORIZONTAL);
    e.addValue("vertical", ConnectedTextureMethods::VERTICAL);
    e.addValue("flora", ConnectedTextureMethods::FLORA);
}
KEG_ENUM_DEF(ConnectedTextureSymmetry, ConnectedTextureSymmetry, e) {
    e.addValue("none", ConnectedTextureSymmetry::NONE);
    e.addValue("opposite", ConnectedTextureSymmetry::OPPOSITE);
    e.addValue("all", ConnectedTextureSymmetry::ALL);
}

KEG_TYPE_DEF_SAME_NAME(BlockMaterialLayer, kt) {
    kt.addValue("method", keg::Value::custom(offsetof(BlockMaterialLayer, method), "ConnectedTextureMethods", true));
    kt.addValue("reducedMethod", keg::Value::custom(offsetof(BlockMaterialLayer, reducedMethod), "ConnectedTextureReducedMethod", true));
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockMaterialLayer, size, UI32_V2);
    kt.addValue("symmetry", keg::Value::custom(offsetof(BlockMaterialLayer, symmetry), "ConnectedTextureSymmetry", true));
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockMaterialLayer, innerSeams, BOOL);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockMaterialLayer, transparency, BOOL);
    kt.addValue("height", keg::Value::basic(offsetof(BlockMaterialLayer, floraHeight), keg::BasicType::UI32));
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockMaterialLayer, useMapColor, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockMaterialLayer, totalWeight, UI32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockMaterialLayer, numTiles, UI32);
    kt.addValue("weights", keg::Value::array(offsetof(BlockMaterialLayer, weights), keg::BasicType::I32));
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockMaterialLayer, index, UI32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockMaterialLayer, path, STRING);
}

KEG_TYPE_DEF_SAME_NAME(BlockMaterial, kt) {
    kt.addValue("base", keg::Value::custom(offsetof(BlockMaterial, base), "BlockTextureLayer"));
    kt.addValue("overlay", keg::Value::custom(offsetof(BlockMaterial, overlay), "BlockTextureLayer"));
    kt.addValue("blendMode", keg::Value::custom(offsetof(BlockMaterial, blendMode), "BlendType", true));
}

