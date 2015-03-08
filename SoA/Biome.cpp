#include "stdafx.h"
#include "Biome.h"

KEG_TYPE_DEF(BlockLayer, BlockLayer, kt) {
    using namespace keg;
    kt.addValue("width", Value::basic(offsetof(BlockLayer, width), BasicType::UI32));
}

KEG_TYPE_DEF(Biome, Biome, kt) {
    using namespace keg;
    kt.addValue("displayName", Value::basic(offsetof(Biome, displayName), BasicType::UI32));
    kt.addValue("mapColor", Value::basic(offsetof(Biome, mapColor), BasicType::UI32));
    kt.addValue("blockLayers", Value::array(offsetof(Biome, blockLayers), Value::custom(0, "BlockLayer")));
}
