#include "stdafx.h"
#include "HardCodedIDs.h"

#include "BlockPack.h"

ui16 NONE = 0;
ui16 DIRT = 0;
ui16 SAND = 0;
ui16 FIRE = 0;
ui16 ICE = 0;
ui16 SNOW = 0;
ui16 DIRTGRASS = 0;
ui16 GRAVEL = 0;
ui16 SLATE = 0;
ui16 LIMESTONE = 0;
ui16 GNEISS = 0;
ui16 BASALT = 0;
ui16 GRANITE = 0;
ui16 STONE = 0;
ui16 LEAVES1 = 0;

void findIDs(BlockPack* pack) {
    BlockPack& p = *pack;
    NONE = p["None"].ID;
    DIRT = p["Dirt"].ID;
    SAND = p["Sand"].ID;
    FIRE = p["Fire"].ID;
    ICE = p["Ice"].ID;
    SNOW = p["Snow"].ID;
    DIRTGRASS = p["Grass Block"].ID;
    GRAVEL = p["Gravel"].ID;
    SLATE = p["Slate"].ID;
    LIMESTONE = p["Limestone"].ID;
    GNEISS = p["Gneiss"].ID;
    BASALT = p["Basalt"].ID;
    GRANITE = p["Granite"].ID;
    STONE = p["Stone"].ID;
    LEAVES1 = p["Leaves (Deciduous Tree)"].ID;
}
