#include "stdafx.h"
#include "ChunkAccessor.h"

#include "ChunkAllocator.h"

ChunkHandle ChunkHandle::aquire() {
    return chunk->accessor->aquire(*this);
}

void ChunkHandle::release() {
    chunk->accessor->release(*this);
}

void ChunkAccessor::init(PagedChunkAllocator* allocator) {
    m_allocator = allocator;
}

void ChunkAccessor::destroy() {
    std::unordered_map<ChunkID, ChunkHandle>().swap(m_chunkLookup);
}

ChunkHandle ChunkAccessor::aquire(ChunkID id) {
    auto& it = m_chunkLookup.find(id);
    if (it == m_chunkLookup.end()) {
        ChunkHandle& rv = m_chunkLookup[id];
        rv.chunk = m_allocator->alloc();
        rv.chunk->m_hRefCount = 1;
        return rv;
    }
}

ChunkHandle ChunkAccessor::aquire(ChunkHandle& chunk) {
    ++chunk.chunk->m_hRefCount;
    ChunkHandle rv;
    rv.chunk = chunk.chunk;
    return rv;
}

void ChunkAccessor::release(ChunkHandle& chunk) {
    --chunk.chunk->m_hRefCount;
    if (chunk.chunk->m_hRefCount == 0) {
        m_chunkLookup.erase(chunk.chunk->m_id);
        // TODO(Ben): Time based free?
        m_allocator->free(chunk.chunk);
    }
    chunk.chunk = nullptr;
}
