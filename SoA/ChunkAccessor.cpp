#include "stdafx.h"
#include "ChunkAccessor.h"

#include "ChunkAllocator.h"

ChunkHandle ChunkHandle::acquire() {
    return m_chunk->accessor->acquire(*this);
}
void ChunkHandle::release() {
    m_chunk->accessor->release(*this);
}

void ChunkAccessor::init(PagedChunkAllocator* allocator) {
    m_allocator = allocator;
}
void ChunkAccessor::destroy() {
    std::unordered_map<ChunkID, ChunkHandle>().swap(m_chunkLookup);
}

ChunkHandle ChunkAccessor::acquire(ChunkID id) {
    auto& it = m_chunkLookup.find(id);
    if (it == m_chunkLookup.end()) {
        ChunkHandle& rv = m_chunkLookup[id];
        rv.m_chunk = m_allocator->alloc();
        rv->m_hRefCount = 1;
        return rv;
    } else {
        return acquire(it->second);
    }
}
ChunkHandle ChunkAccessor::acquire(ChunkHandle& chunk) {
    chunk->m_hRefCount++;
    return chunk;
}

void ChunkAccessor::release(ChunkHandle& chunk) {
    // TODO(Cristian): Heavy thread-safety
    chunk->m_hRefCount--;
    if (chunk->m_hRefCount == 0) {
        // Make sure it can't be accessed until acquired again
        chunk->accessor = nullptr;
        m_chunkLookup.erase(chunk->m_id);
        // TODO(Ben): Time based free?
        m_allocator->free(chunk.m_chunk);
    }
    chunk.m_chunk = nullptr;
}
