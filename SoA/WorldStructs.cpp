#include "stdafx.h"
#include "WorldStructs.h"

#include "BlockData.h"
#include "Chunk.h"
#include "SoaOptions.h"
#include "GameManager.h"
#include "RenderTask.h"

MultiplePreciseTimer globalMultiplePreciseTimer; ///< for easy global benchmarking
AccumulationTimer globalAccumulationTimer; ///< for easy global benchmarking
AccumulationTimer globalRenderAccumulationTimer; ///< for easy global benchmarking

class Item *ObjectList[OBJECT_LIST_SIZE];

Marker::Marker(const glm::dvec3 &Pos, nString Name, glm::vec3 Color) : pos(Pos), name(Name), dist(0.0)
{
    // TODO(Ben): implement
}

void Marker::Draw(glm::mat4 &VP, const glm::dvec3 &playerPos)
{
    // TODO(Ben): implement
}
