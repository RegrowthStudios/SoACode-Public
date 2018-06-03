#include "stdafx.h"
#include "ChunkQuery.h"

#include "ChunkGrid.h"

void ChunkQuery::release() {
    grid->releaseQuery(this);
}