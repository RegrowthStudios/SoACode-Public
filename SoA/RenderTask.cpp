#include "stdafx.h"
#include "RenderTask.h"

#include "Chunk.h"

void RenderTask::execute() {

}

void RenderTask::setChunk(Chunk* ch, MeshJobType cType) {
    type = cType;
    chunk = ch;
    num = ch->numBlocks;
    position = ch->gridPosition;
    wSize = 0;
}