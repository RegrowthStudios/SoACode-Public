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

// 00111 00111 00111 = 0x1CE7
#define NO_CHUNK_OFFSET 0x1CE7

// Will not work for chunks > 32^3
struct FloraNode {
    FloraNode(ui16 blockIndex, ui16 blockID, ui16 chunkOffset = NO_CHUNK_OFFSET) :
        blockIndex(blockIndex), blockID(blockID), chunkOffset(chunkOffset) {};
    ui16 blockIndex;
    ui16 blockID;
    ui16 chunkOffset; ///< Packed 0 XXXXX YYYYY ZZZZZ for positional offset. 00111 == 0
};

class NFloraGenerator {
public:
    /// @brief Generates flora for a chunk using its QueuedFlora.
    /// @param chunk: Chunk who's flora should be generated.
    /// @param gridData: The heightmap to use
    /// @param fNodes: Returned low priority nodes, for flora and leaves.
    /// @param wNodes: Returned high priority nodes, for tree "wood".
    void generateChunkFlora(const Chunk* chunk, const PlanetHeightData* heightData, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes);
    /// Generates standalone tree.
    void generateTree(const NTreeType* type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui16 chunkOffset = NO_CHUNK_OFFSET, ui16 blockIndex = 0);
    /// Generates standalone flora.
    void generateFlora(FloraData type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui16 chunkOffset = NO_CHUNK_OFFSET, ui16 blockIndex = 0);
    /// Generates a specific tree's properties
    static void generateTreeProperties(const NTreeType* type, f32 age, OUT TreeData& tree);
private:
    std::vector<FloraNode>* m_fNodes;
    std::vector<FloraNode>* m_wNodes;
    TreeData m_treeData;
    FloraData m_floraData;
};

#endif // NFloraGenerator_h__
