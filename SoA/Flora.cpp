#include "stdafx.h"
#include "Flora.h"

KEG_ENUM_DEF(TreeLeafType, TreeLeafType, e) {
    e.addValue("none", TreeLeafType::NONE);
    e.addValue("round", TreeLeafType::ROUND);
    e.addValue("pine", TreeLeafType::PINE);
    e.addValue("mushroom", TreeLeafType::MUSHROOM);
}
