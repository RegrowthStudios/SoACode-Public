#include "stdafx.h"
#include "GenerateTask.h"

#include "Chunk.h"
#include "ChunkGenerator.h"
#include "ChunkGrid.h"
#include "FloraGenerator.h"

void GenerateTask::execute(WorkerData* workerData) {
    Chunk& chunk = query->chunk;

    // Check if this is a heightmap gen
    if (chunk.gridData->isLoading) {
        chunkGenerator->m_proceduralGenerator.generateHeightmap(&chunk, heightData);
    } else { // Its a chunk gen

        switch (query->genLevel) {
            case ChunkGenLevel::GEN_DONE:
            case ChunkGenLevel::GEN_TERRAIN:
                chunkGenerator->m_proceduralGenerator.generateChunk(&chunk, heightData);
                chunk.genLevel = GEN_TERRAIN;
                // TODO(Ben): Not lazy load.
                if (!workerData->floraGenerator) {
                    workerData->floraGenerator = new FloraGenerator;
                }
                generateFlora(workerData, chunk);
                chunk.genLevel = ChunkGenLevel::GEN_DONE;
                break;
            case ChunkGenLevel::GEN_FLORA:
                chunk.genLevel = ChunkGenLevel::GEN_DONE;
                break;
            case ChunkGenLevel::GEN_SCRIPT:
                chunk.genLevel = ChunkGenLevel::GEN_DONE;
                break;
        }
        query->m_isFinished = true;
        query->m_cond.notify_one();
        // TODO(Ben): Not true for all gen?
        chunk.isAccessible = true;
    }
    chunkGenerator->finishQuery(query);
}

struct ChunkFloraArrays {
    std::vector<VoxelToPlace> fNodes;
    std::vector<VoxelToPlace> wNodes;
};

void GenerateTask::generateFlora(WorkerData* workerData, Chunk& chunk) {
    std::vector<FloraNode> fNodes, wNodes;
    workerData->floraGenerator->generateChunkFlora(&chunk, heightData, fNodes, wNodes);


    // Sort based on chunk to minimize locking
    std::map<ChunkID, ChunkFloraArrays> chunkMap;

    // Add all nodes
    // TODO(Ben): There should be a way to approximate size needs or use map on generator side.
    for (auto& it : fNodes) {
        ChunkID id(chunk.getID());
        id.x += FloraGenerator::getChunkXOffset(it.chunkOffset);
        id.y += FloraGenerator::getChunkYOffset(it.chunkOffset);
        id.z += FloraGenerator::getChunkZOffset(it.chunkOffset);
        chunkMap[id].fNodes.emplace_back(it.blockID, it.blockIndex);
    }
    for (auto& it : wNodes) {
        ChunkID id(chunk.getID());
        id.x += FloraGenerator::getChunkXOffset(it.chunkOffset);
        id.y += FloraGenerator::getChunkYOffset(it.chunkOffset);
        id.z += FloraGenerator::getChunkZOffset(it.chunkOffset);

        chunkMap[id].wNodes.emplace_back(it.blockID, it.blockIndex);
    }

    // Traverse chunks
    for (auto& it : chunkMap) {
        ChunkHandle h = query->grid->accessor.acquire(it.first);
        // TODO(Ben): Handle other case
        if (h->genLevel >= GEN_TERRAIN) {
            {
                std::lock_guard<std::mutex> l(h->dataMutex);
                for (auto& node : it.second.wNodes) {
                    h->blocks.set(node.blockIndex, node.blockID);
                }
                for (auto& node : it.second.fNodes) {
                    if (h->blocks.get(node.blockIndex) == 0) {
                        h->blocks.set(node.blockIndex, node.blockID);
                    }
                }
            }

            if (h->genLevel == GEN_DONE) h->DataChange(h);
        } else {
            query->grid->nodeSetter.setNodes(h, GEN_TERRAIN, it.second.wNodes, it.second.fNodes);
        }
        h.release();
    }

    std::vector<ui16>().swap(chunk.floraToGenerate);
}