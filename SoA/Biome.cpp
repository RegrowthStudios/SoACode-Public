#include "stdafx.h"
#include "Biome.h"

KEG_TYPE_DEF_SAME_NAME(BiomeFloraKegProperties, kt) {
    using namespace keg;
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeFloraKegProperties, id, STRING);
    kt.addValue("chance", Value::custom(offsetof(BiomeFloraKegProperties, chance), "NoiseBase", false));
}

KEG_TYPE_DEF_SAME_NAME(BlockLayer, kt) {
    using namespace keg;
    kt.addValue("width", Value::basic(offsetof(BlockLayer, width), BasicType::UI32));
}
