#include "stdafx.h"
#include "WorldStructs.h"

#include "BlockData.h"
#include "Chunk.h"
#include "Options.h"
#include "GameManager.h"
#include "GLProgramManager.h"
#include "RenderTask.h"
#include "Texture2d.h"

MultiplePreciseTimer globalMultiplePreciseTimer; ///< for easy global benchmarking

class Item *ObjectList[OBJECT_LIST_SIZE];

Marker::Marker(const glm::dvec3 &Pos, nString Name, glm::vec3 Color) : pos(Pos), name(Name), dist(0.0)
{
    // TODO(Ben): implement
}

void Marker::Draw(glm::mat4 &VP, const glm::dvec3 &playerPos)
{
    // TODO(Ben): implement
}
