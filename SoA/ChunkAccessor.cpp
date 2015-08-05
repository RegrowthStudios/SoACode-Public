#include "stdafx.h"
#include "ChunkAccessor.h"

#include "ChunkAllocator.h"

const ui32 HANDLE_STATE_DEAD = 0;
const ui32 HANDLE_STATE_BIRTHING = 1;
const ui32 HANDLE_STATE_ALIVE = 2;
const ui32 HANDLE_STATE_ZOMBIE = 3;

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

#ifdef FAST_CHUNK_ACCESS

ChunkHandle ChunkAccessor::acquire(ChunkID id) {
    // TODO(Cristian): Try to not lock down everything
    bool wasOld;
    ChunkHandle chunk = safeAdd(id, wasOld);
    return wasOld ? acquire(chunk) : chunk;
}
ChunkHandle ChunkAccessor::acquire(ChunkHandle& chunk) {
    // If it's dead, we want to make it come to life
REVIVAL:
    switch (InterlockedCompareExchange(&chunk->m_handleState, HANDLE_STATE_BIRTHING, HANDLE_STATE_ZOMBIE)) {
    case HANDLE_STATE_BIRTHING:
    case HANDLE_STATE_ALIVE:
ACQUIRE :
        // We can safely increment here
        InterlockedIncrement(&chunk->m_handleRefCount);
        // Make this alive
        InterlockedCompareExchange(&chunk->m_handleState, HANDLE_STATE_ALIVE, HANDLE_STATE_BIRTHING);
        return chunk;
    case HANDLE_STATE_ZOMBIE:
    { // Someone is trying to kill this chunk, don't let them
        std::lock_guard<std::mutex>(chunk->mutex);
        if (InterlockedCompareExchange(&chunk->m_handleState, HANDLE_STATE_BIRTHING, HANDLE_STATE_ZOMBIE) != HANDLE_STATE_DEAD) {
            goto ACQUIRE;
        } else {
            std::this_thread::yield();
            goto REVIVAL;
        }
    }
    case HANDLE_STATE_DEAD:
        // It's dead, it must be revived
        return acquire(chunk->m_id);
    default:
        throw new nString("Invalid HANDLE_STATE");
        return chunk;
    }
}
void ChunkAccessor::release(ChunkHandle& chunk) {
    // TODO(Cristian): Heavy thread-safety
    ui32 currentCount = InterlockedDecrement(&chunk->m_handleRefCount);
    if (currentCount == 0) {
        // If the chunk is alive, set it to zombie mode. Otherwise, it's being birthed or already dead.
        if (InterlockedCompareExchange(&chunk->m_handleState, HANDLE_STATE_ZOMBIE, HANDLE_STATE_ALIVE) == HANDLE_STATE_ALIVE) {
            // Now that it's a zombie, we kill it.
            if (InterlockedCompareExchange(&chunk->m_handleState, HANDLE_STATE_DEAD, HANDLE_STATE_ZOMBIE) == HANDLE_STATE_ZOMBIE) {
                // Highlander... there can only be one... killer of chunks
                std::lock_guard<std::mutex> chunkLock(chunk->mutex);
                currentCount = InterlockedDecrement(&chunk->m_handleRefCount);
                if (currentCount == -1) {
                    // We are able to kill this chunk
                    safeRemove(chunk);
                }
            }
        }
    }
    chunk.m_chunk = nullptr;
}

#else

ChunkHandle ChunkAccessor::acquire(ChunkID id) {
    bool wasOld;
    ChunkHandle chunk = safeAdd(id, wasOld);
    return wasOld ? acquire(chunk) : chunk;
}
ChunkHandle ChunkAccessor::acquire(ChunkHandle& chunk) {
    std::lock_guard<std::mutex> lChunk(chunk->mutex);
    if (chunk->m_handleRefCount == 0) {
        // We need to re-add the chunk
        bool wasOld = false;
        return safeAdd(chunk->m_id, wasOld);
    } else {
        chunk->m_handleRefCount++;
        return chunk;
    }
}
void ChunkAccessor::release(ChunkHandle& chunk) {
    std::lock_guard<std::mutex> lChunk(chunk->mutex);
    chunk->m_handleRefCount--;
    if (chunk->m_handleRefCount == 0) {
        // We need to remove the chunk
        safeRemove(chunk);
    }
    chunk.m_chunk = nullptr;
}

#endif

ChunkHandle ChunkAccessor::safeAdd(ChunkID id, bool& wasOld) {
    std::unique_lock<std::mutex> l(m_lckLookup);
    auto& it = m_chunkLookup.find(id);
    if (it == m_chunkLookup.end()) {
        wasOld = false;
        ChunkHandle& h = m_chunkLookup[id];
        h.m_chunk = m_allocator->alloc();
        h->m_id = id;
        h->accessor = this;
        h->m_handleState = HANDLE_STATE_ALIVE;
        h->m_handleRefCount = 1;
        l.unlock();
        onAdd(ChunkHandle(h));
        return h;
    } else {
        wasOld = true;
        return it->second;
    }
}
void ChunkAccessor::safeRemove(ChunkHandle& chunk) {
    { // TODO(Cristian): This needs to be added to a free-list?
        std::lock_guard<std::mutex> l(m_lckLookup);

        // Make sure it can't be accessed until acquired again
        chunk->accessor = nullptr;

        // TODO(Ben): Time based free?
        m_chunkLookup.erase(chunk->m_id);
    }
    // Fire event before deallocating
    onRemove(chunk);
    m_allocator->free(chunk.m_chunk);
}
