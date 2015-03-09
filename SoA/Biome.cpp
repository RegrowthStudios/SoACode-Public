#include "stdafx.h"
#include "Biome.h"

KEG_TYPE_DEF_SAME_NAME(BlockLayer, kt) {
    using namespace keg;
    kt.addValue("width", Value::basic(offsetof(BlockLayer, width), BasicType::UI32));
}

KEG_TYPE_DEF_SAME_NAME(Biome, kt) {
    using namespace keg;
    kt.addValue("displayName", Value::basic(offsetof(Biome, displayName), BasicType::STRING));
    kt.addValue("mapColor", Value::basic(offsetof(Biome, mapColor), BasicType::UI8_V3));
    kt.addValue("blockLayers", Value::array(offsetof(Biome, blockLayers), Value::custom(0, "BlockLayer")));
}
