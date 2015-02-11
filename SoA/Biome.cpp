#include "stdafx.h"
#include "Biome.h"

KEG_TYPE_INIT_BEGIN_DEF_VAR(BlockLayer)
KEG_TYPE_INIT_ADD_MEMBER(BlockLayer, UI32, width);
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(Biome)
KEG_TYPE_INIT_ADD_MEMBER(Biome, STRING, displayName);
KEG_TYPE_INIT_ADD_MEMBER(Biome, UI8_V3, mapColor);
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("blockLayers", Keg::Value::array(offsetof(Biome, blockLayers), Keg::Value::custom("BlockLayer", 0)));
KEG_TYPE_INIT_END