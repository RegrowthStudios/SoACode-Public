#include "stdafx.h"
#include "RenderTask.h"

#include "Chunk.h"

void RenderTask::setChunk(Chunk* ch, MeshJobType cType) {
    type = cType;
    chunk = ch;
    num = ch->numBlocks;
    position = ch->position;
    wSize = 0;
}