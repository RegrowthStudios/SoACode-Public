#include "stdafx.h"
#include "BlockTexture.h"

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

KEG_TYPE_DEF_SAME_NAME(BlockTextureLayer, kt) {
    kt.addValue("method", keg::Value::custom(offsetof(BlockTextureLayer, method), "ConnectedTextureMethods", true));
    kt.addValue("reducedMethod", keg::Value::custom(offsetof(BlockTextureLayer, reducedMethod), "ConnectedTextureReducedMethod", true));
    kt.addValue("symmetry", keg::Value::custom(offsetof(BlockTextureLayer, symmetry), "ConnectedTextureSymmetry", true));
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, innerSeams, BOOL);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, transparency, BOOL);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, useMapColor, STRING);
    kt.addValue("weights", keg::Value::array(offsetof(BlockTextureLayer, weights), keg::BasicType::I32));
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, path, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, normalPath, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, dispPath, STRING);
}

KEG_TYPE_DEF_SAME_NAME(BlockTexture, kt) {
    kt.addValue("base", keg::Value::custom(offsetof(BlockTexture, base), "BlockTextureLayer"));
    kt.addValue("overlay", keg::Value::custom(offsetof(BlockTexture, overlay), "BlockTextureLayer"));
    kt.addValue("blendMode", keg::Value::custom(offsetof(BlockTexture, blendMode), "BlendType", true));
}

