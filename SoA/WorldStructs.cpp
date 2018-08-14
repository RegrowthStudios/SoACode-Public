#include "stdafx.h"
#include "WorldStructs.h"

#include "BlockData.h"
#include "SoaOptions.h"
#include "GameManager.h"

MultiplePreciseTimer globalMultiplePreciseTimer; ///< for easy global benchmarking
AccumulationTimer globalAccumulationTimer; ///< for easy global benchmarking
AccumulationTimer globalRenderAccumulationTimer; ///< for easy global benchmarking

class Item *ObjectList[OBJECT_LIST_SIZE];

Marker::Marker(const f64v3 &Pos VORB_UNUSED, nString Name VORB_UNUSED, f32v3 Color VORB_UNUSED) : pos(Pos), name(Name), dist(0.0)
{
    // TODO(Ben): implement and remove unused tags
}

void Marker::Draw(f32m4 &VP VORB_UNUSED, const f64v3 &playerPos VORB_UNUSED)
{
    // TODO(Ben): implement and remove unused tags
}
