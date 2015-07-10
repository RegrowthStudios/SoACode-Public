#include "stdafx.h"
#include "Biome.h"

KEG_TYPE_DEF_SAME_NAME(BlockLayer, kt) {
    using namespace keg;
    kt.addValue("width", Value::basic(offsetof(BlockLayer, width), BasicType::UI32));
}

KEG_ENUM_DEF(BiomeAxisType, BiomeAxisType, e) {
    e.addValue("noise", BiomeAxisType::NOISE);
    e.addValue("height", BiomeAxisType::HEIGHT);
}