#include "stdafx.h"
#include "ChunkQuery.h"

#include "ChunkGrid.h"

void ChunkQuery::release() {
    m_grid->releaseQuery(this);
}