#include "stdafx.h"
#include "BlockTexture.h"

#include "BlockTexturePack.h"

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

KEG_TYPE_DEF_SAME_NAME(BlockTexture, kt) {
    kt.addValue("base", keg::Value::custom(offsetOf(BlockTexture, layers.base), "BlockTextureLayer"));
    kt.addValue("overlay", keg::Value::custom(offsetOf(BlockTexture, layers.overlay), "BlockTextureLayer"));
    kt.addValue("blendMode", keg::Value::custom(offsetOf(BlockTexture, blendMode), "BlendType", true));
}

/// "less than" operator for inserting into sets in TexturePackLoader
bool BlockTextureLayer::operator<(const BlockTextureLayer& b) const {

    // Helper macro for checking if !=
#define LCHECK(a) if (a < b.a) { return true; } else if (a > b.a) { return false; }
    LCHECK(path);
    LCHECK(method);
    LCHECK(size.x);
    LCHECK(size.y);
    LCHECK(symmetry);
    LCHECK(color.r);
    LCHECK(color.g);
    LCHECK(color.b);
    LCHECK(reducedMethod);
    LCHECK(weights.size());
    LCHECK(totalWeight);
    LCHECK(numTiles);
    LCHECK(innerSeams);
    LCHECK(transparency);
    return false;
}

void BlockTextureLayer::getFinalColor(OUT color3& resColor, ui8 temperature, ui8 rainfall, ui32 altColor) const {
    // TODO(Ben): Alternate colors
    if (colorMap) {
        // TODO(Ben): Store as floats to prevent cast overhead?
        const color3& mapColor = colorMap->pixels[rainfall][temperature];
        //Average the map color with the base color
        resColor.r = (ui8)(((f32)color.r * (f32)mapColor.r) / 255.0f);
        resColor.g = (ui8)(((f32)color.g * (f32)mapColor.g) / 255.0f);
        resColor.b = (ui8)(((f32)color.b * (f32)mapColor.b) / 255.0f);
    } /*else if (altColor > altColors.size()) { //alt colors, for leaves and such
        baseColor = altColors[flags - 1];
    } */else {
        resColor = color;
    }
}