#include "stdafx.h"
#include "VoxelNodeSetter.h"

#include "ChunkGrid.h"

void VoxelNodeSetter::setNodes(ChunkHandle& h, ChunkGenLevel requiredGenLevel, std::vector<VoxelToPlace>& forcedNodes, std::vector<VoxelToPlace>& condNodes) {
    {
        std::lock_guard<std::mutex> l(m_lckVoxelsToAdd);
        auto& it = m_handleLookup.find(h);
        if (it != m_handleLookup.end()) {
            VoxelNodeSetterLookupData& ld = it->second;
            // Copy new voxels to add
            size_t oldSize = ld.forcedNodes.size();
            ld.forcedNodes.resize(oldSize + forcedNodes.size());
            memcpy(&ld.forcedNodes[oldSize], forcedNodes.data(), forcedNodes.size() * sizeof(VoxelToPlace));
            oldSize = ld.condNodes.size();
            ld.condNodes.resize(oldSize + condNodes.size());
            memcpy(&ld.condNodes[oldSize], condNodes.data(), condNodes.size() * sizeof(VoxelToPlace));
            // Update required gen level if needed
            if (m_waitingChunks[ld.waitingChunksIndex].requiredGenLevel < requiredGenLevel) {
                m_waitingChunks[ld.waitingChunksIndex].requiredGenLevel = requiredGenLevel;
            }
        } else {
            VoxelNodeSetterLookupData& ld = m_handleLookup[h];
            ld.forcedNodes.swap(forcedNodes);
            ld.condNodes.swap(condNodes);
            ld.h = h.acquire();
            ld.waitingChunksIndex = m_waitingChunks.size();

            VoxelNodeSetterWaitingChunk wc;
            wc.ch = h;
            wc.requiredGenLevel = requiredGenLevel;
            m_waitingChunks.push_back(std::move(wc));
        }
    }
    // TODO(Ben): Faster overload?
    grid->submitQuery(h->getChunkPosition(), requiredGenLevel, true);
}

void VoxelNodeSetter::update() {
    if (m_waitingChunks.size()) {
        std::cout << m_waitingChunks.size() << std::endl;
    }
    std::lock_guard<std::mutex> l(m_lckVoxelsToAdd);
    for (int i = (int)m_waitingChunks.size() - 1; i >= 0; i--) {
        VoxelNodeSetterWaitingChunk& v = m_waitingChunks[i];

        if (v.ch->genLevel >= v.requiredGenLevel) {
            auto& it = m_handleLookup.find(v.ch);

            {
                // Send task
                // TODO(Ben): MEMORY MANAGEMENT PROBLEM
                VoxelNodeSetterTask* newTask = new VoxelNodeSetterTask;
                newTask->h = it->second.h.acquire();
                newTask->forcedNodes.swap(it->second.forcedNodes);
                newTask->condNodes.swap(it->second.condNodes);
                threadPool->addTask(newTask);
            }
           
            it->second.h.release();
            m_handleLookup.erase(it);
            
            // Copy back node for fast removal from vector
            VoxelNodeSetterWaitingChunk& bn = m_waitingChunks.back();
            v.ch = bn.ch;
            v.requiredGenLevel = bn.requiredGenLevel;

            // Update lookup index
            m_handleLookup[v.ch].waitingChunksIndex = i;

            m_waitingChunks.pop_back();
        }
    }
}
