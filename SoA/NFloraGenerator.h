///
/// NFloraGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 15 Jul 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Generates flora and trees
///

#pragma once

#ifndef NFloraGenerator_h__
#define NFloraGenerator_h__

#include "Flora.h"
#include "Chunk.h"

// Will not work for chunks > 32^3
struct FloraNode {
    ui16 blockIndex;
    ui16 chunkOffset; ///< Packed 0 XXXXX YYYYY ZZZZZ for positional offset. 00111 == 0
    ui16 blockType;
};

class NFloraGenerator {
public:
    /// @brief Generates flora for a chunk using its QueuedFlora.
    /// @param chunk: Chunk who's flora should be generated.
    /// @param fNodes: Returned low priority nodes, for flora and leaves.
    /// @param wNodes: Returned high priority nodes, for tree "wood".
    void generateChunkFlora(const Chunk* chunk, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes);
    /// Generates standalone tree.
    void generateTree(NTreeType* type, ui32 seed, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui16 chunkOffset = 0, ui16 blockIndex = 0);
    /// Generates standalone flora.
    void generateFlora(FloraData type, ui32 seed, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui16 chunkOffset = 0, ui16 blockIndex = 0);
private:
    std::vector<FloraNode>* m_fNodes;
    std::vector<FloraNode>* m_wNodes;
    FloraData m_floraData;
    TreeData m_treeData;
};

#endif // NFloraGenerator_h__