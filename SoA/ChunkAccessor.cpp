#include "stdafx.h"
#include "ChunkAccessor.h"

#include "ChunkAllocator.h"

const ui32 HANDLE_STATE_DEAD = 0;
const ui32 HANDLE_STATE_ACQUIRING = 1;
const ui32 HANDLE_STATE_ALIVE = 2;
const ui32 HANDLE_STATE_FREEING = 3;

ChunkHandle::ChunkHandle(const ChunkHandle& other) :
    m_acquired(false),
    m_accessor(other.m_acquired ? other.m_chunk->accessor : other.m_accessor),
    m_id(other.m_id) {
    // Empty
}
ChunkHandle& ChunkHandle::operator= (const ChunkHandle& other) {
    m_acquired = false;
    m_accessor = other.m_acquired ? other.m_chunk->accessor : other.m_accessor;
    m_id = other.m_id;

    return *this;
}

void ChunkHandle::acquireSelf() {
    if (!m_acquired) m_chunk->accessor->acquire(*this);
}
ChunkHandle ChunkHandle::acquire() {
    if (m_acquired) {
        ChunkHandle h(m_chunk->accessor->acquire(*this));
        h.m_acquired = true;
        h.m_chunk = m_chunk;
        return std::move(h);
    } else {
        return std::move(m_accessor->acquire(m_id));
    }
}
void ChunkHandle::release() {
    if (m_acquired) m_chunk->accessor->release(*this);
}

void ChunkAccessor::init(PagedChunkAllocator* allocator) {
    m_allocator = allocator;
}
void ChunkAccessor::destroy() {
    std::unordered_map<ChunkID, ChunkHandle>().swap(m_chunkLookup);
}

#ifdef FAST_CHUNK_ACCESS

ChunkHandle ChunkAccessor::acquire(ChunkID id) {
    std::unique_lock<std::mutex> lMap(m_lckLookup);
    auto& it = m_chunkLookup.find(id);
    if (it == m_chunkLookup.end()) {
        ChunkHandle& h = m_chunkLookup[id];
        h.m_chunk = m_allocator->alloc();
        h->m_handleRefCount = 1;
        h->m_id = id;
        h->accessor = this;
        h->m_handleState = HANDLE_STATE_ALIVE;
        lMap.unlock();
        onAdd(ChunkHandle(h));
        return h;
    } else {
        InterlockedIncrement(&it->second->m_handleRefCount);
        it->second->m_handleState = HANDLE_STATE_ALIVE;
        return it->second;
    }
}
ChunkHandle ChunkAccessor::acquire(ChunkHandle& chunk) {
    switch (InterlockedCompareExchange(&chunk->m_handleState, HANDLE_STATE_ACQUIRING, HANDLE_STATE_ALIVE)) {
    case HANDLE_STATE_FREEING:
        return acquire(chunk->m_id);
    case HANDLE_STATE_ACQUIRING:
        InterlockedIncrement(&chunk->m_handleRefCount);
        return chunk;
    case HANDLE_STATE_ALIVE:
        InterlockedIncrement(&chunk->m_handleRefCount);
        InterlockedCompareExchange(&chunk->m_handleState, HANDLE_STATE_ALIVE, HANDLE_STATE_ACQUIRING);
        return chunk;
    default:
        throw std::invalid_argument("INVALID CHUNK HANDLE STATE");
    }
}
void ChunkAccessor::release(ChunkHandle& chunk) {
    ui32 retries = 0;

    // TODO(Cristian): Heavy thread-safety
    ui32 currentCount = InterlockedDecrement(&chunk->m_handleRefCount);
RETEST_CHUNK_LIVELINESS:
    if (currentCount == 0) {
        // If the chunk is alive, set it to zombie mode. Otherwise, it's being birthed or already dead.
        switch (InterlockedCompareExchange(&chunk->m_handleState, HANDLE_STATE_FREEING, HANDLE_STATE_ALIVE)) {
        case HANDLE_STATE_ALIVE:
        { // Highlander... there can only be one... killer of chunks
            std::lock_guard<std::mutex> chunkLock(chunk->m_handleMutex);
            currentCount = InterlockedDecrement(&chunk->m_handleRefCount);
            if (chunk->m_handleState == HANDLE_STATE_FREEING) {
                // We are able to kill this chunk
                safeRemove(chunk);
            }
        }
        case HANDLE_STATE_FREEING:
            // Let the other thread do the work
            break;
        case HANDLE_STATE_ACQUIRING:
            // Need to retry
            std::this_thread::yield();
            currentCount = chunk->m_handleRefCount;
            retries++;
            goto RETEST_CHUNK_LIVELINESS;
        }
    }

    // Invalidate the handle
    chunk.m_chunk = nullptr;
    if (retries > 2) {
        printf("A lot of release retries occurred: %d\n", retries);
        fflush(stdout);
    }
}

#else

ChunkHandle ChunkAccessor::acquire(ChunkID id) {
    bool wasOld;
    ChunkHandle chunk = std::move(safeAdd(id, wasOld));

    if (wasOld) return std::move(acquire(chunk));

    chunk.m_acquired = true;
    return std::move(chunk);
}
ChunkHandle ChunkAccessor::acquire(ChunkHandle& chunk) {
    std::lock_guard<std::mutex> lChunk(chunk->m_handleMutex);
    if (chunk->m_handleRefCount == 0) {
        // We need to re-add the chunk
        bool wasOld = false;
        chunk.m_acquired = true;
        return std::move(safeAdd(chunk.m_id, wasOld));
    } else {
        chunk->m_handleRefCount++;
        chunk.m_acquired = true;
        return std::move(chunk);
    }
}
void ChunkAccessor::release(ChunkHandle& chunk) {
    std::lock_guard<std::mutex> lChunk(chunk->m_handleMutex);
    chunk->m_handleRefCount--;
    if (chunk->m_handleRefCount == 0) {
        // We need to remove the chunk
        safeRemove(chunk);
    }
    chunk.m_acquired = false;
    chunk.m_accessor = this;
}

#endif

ChunkHandle ChunkAccessor::safeAdd(ChunkID id, bool& wasOld) {
    std::unique_lock<std::mutex> l(m_lckLookup);
    auto& it = m_chunkLookup.find(id);
    if (it == m_chunkLookup.end()) {
        wasOld = false;
        ChunkHandle& h = m_chunkLookup[id];
        h.m_chunk = m_allocator->alloc();
        h.m_id = id;
        h->m_id = id;
        h->accessor = this;
        h->m_handleState = HANDLE_STATE_ALIVE;
        h->m_handleRefCount = 1;
        ChunkHandle tmp(h);
        l.unlock();
        onAdd(tmp);
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
        m_chunkLookup.erase(chunk.m_id);
    }
    // Fire event before deallocating
    onRemove(chunk);
    m_allocator->free(chunk.m_chunk);
}
