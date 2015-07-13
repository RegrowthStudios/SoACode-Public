#include "stdafx.h"
#include "Biome.h"

KEG_TYPE_DEF_SAME_NAME(BlockLayer, kt) {
    using namespace keg;
    kt.addValue("width", Value::basic(offsetof(BlockLayer, width), BasicType::UI32));
}
