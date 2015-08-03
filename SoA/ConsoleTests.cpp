#include "stdafx.h"
#include "ConsoleTests.h"

#include "ChunkAllocator.h"
#include "ChunkAccessor.h"

struct ChunkAccessSpeedData {
    size_t numThreads;
    std::thread* threads;

    std::mutex lock;
    std::condition_variable cv;

    PagedChunkAllocator allocator;
    ChunkAccessor accessor;

    ChunkID* ids;
    ChunkHandle* handles;
};

ChunkAccessSpeedData* createCASData(size_t numThreads, size_t requestCount, ui64 maxID) {
    ChunkAccessSpeedData* data = new ChunkAccessSpeedData;

    data->accessor.init(&data->allocator);

    // Create the thread pools
    data->numThreads = numThreads;
    data->threads = new std::thread[numThreads] {};
    for (size_t threadID = 0; threadID < numThreads; threadID++) {
        std::thread([data, threadID, requestCount] () {
            { // Wait until everyone is notified
                std::unique_lock<std::mutex> lock(data->lock);
                printf("Thread %d awaiting notification\n", threadID);
                data->cv.wait(lock);
            }

            // Begin requesting chunks
            printf("Thread %d starting\n", threadID);
            ChunkID* id = data->ids + (requestCount * threadID);
            ChunkHandle* hndAcquire = data->handles + (requestCount * threadID);
            ChunkHandle* hndRelease = hndAcquire;
            ChunkHandle* hndEnd = hndRelease + requestCount;
            while (hndRelease != hndEnd) {
                if ((hndAcquire > hndRelease) && (rand() & 0xff) > 127) {
                    // Release a handle
                    hndRelease->release();
                    hndRelease++;
                } else if(hndAcquire != hndEnd) {
                    // Acquire a handle
                    *hndAcquire = data->accessor.acquire(*id);
                    hndAcquire++;
                    id++;
                }
            }
            printf("Thread %d finished\n", threadID);
        }).swap(data->threads[threadID]);
        data->threads[threadID].detach();
    }

    // Create the random requests
    data->ids = new ChunkID[requestCount * numThreads];
    data->handles = new ChunkHandle[requestCount * numThreads]{};
    for (size_t i = 0; i < requestCount * numThreads; i++) {
        data->ids[i] = rand() % maxID;
    }

    return data;
}

void runCAS(ChunkAccessSpeedData* data) {
    // Start the races
    data->cv.notify_all();
}

void freeCAS(ChunkAccessSpeedData* data) {
    data->accessor.destroy();
    delete[] data->ids;
    delete[] data->handles;
    delete[] data->threads;
    delete data;
}


