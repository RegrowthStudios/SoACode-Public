#include "stdafx.h"
#include "BlockTexture.h"


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
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, size, I32_V2);
    kt.addValue("symmetry", keg::Value::custom(offsetof(BlockTextureLayer, symmetry), "ConnectedTextureSymmetry", true));
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, innerSeams, BOOL);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, transparency, BOOL);
    kt.addValue("height", keg::Value::basic(offsetof(BlockTextureLayer, floraHeight), keg::BasicType::UI32));
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, useMapColor, STRING);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, totalWeight, I32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, numTiles, I32);
    kt.addValue("weights", keg::Value::array(offsetof(BlockTextureLayer, weights), keg::BasicType::I32));
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, textureIndex, I32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, path, STRING);
}

KEG_TYPE_DEF_SAME_NAME(BlockTexture, kt) {
    kt.addValue("base", keg::Value::custom(offsetof(BlockTexture, base), "BlockTextureLayer"));
    kt.addValue("overlay", keg::Value::custom(offsetof(BlockTexture, overlay), "BlockTextureLayer"));
    kt.addValue("blendMode", keg::Value::custom(offsetof(BlockTexture, blendMode), "BlendType", true));
}
