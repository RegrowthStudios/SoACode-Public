#include "stdafx.h"
#include "Biome.h"

KEG_TYPE_DEF_SAME_NAME(BiomeFlora, kt) {
    using namespace keg;
    KEG_TYPE_INIT_ADD_MEMBER(kt, BiomeFlora, chance, F32);
}

KEG_TYPE_DEF_SAME_NAME(BlockLayer, kt) {
    using namespace keg;
    kt.addValue("width", Value::basic(offsetof(BlockLayer, width), BasicType::UI32));
}
