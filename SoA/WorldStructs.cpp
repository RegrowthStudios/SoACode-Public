#include "stdafx.h"
#include "WorldStructs.h"

#include "BlockData.h"
#include "SoaOptions.h"
#include "GameManager.h"

MultiplePreciseTimer globalMultiplePreciseTimer; ///< for easy global benchmarking
AccumulationTimer globalAccumulationTimer; ///< for easy global benchmarking
AccumulationTimer globalRenderAccumulationTimer; ///< for easy global benchmarking

class Item *ObjectList[OBJECT_LIST_SIZE];

Marker::Marker(const f64v3 &Pos, nString Name, f32v3 Color) : pos(Pos), name(Name), dist(0.0)
{
    // TODO(Ben): implement
}

void Marker::Draw(f32m4 &VP, const f64v3 &playerPos)
{
    // TODO(Ben): implement
}
