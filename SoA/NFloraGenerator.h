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
    FloraNode(ui16 blockID, ui16 blockIndex, ui16 chunkOffset) :
        blockID(blockID), blockIndex(blockIndex), chunkOffset(chunkOffset) {
    };
    ui16 blockID;
    ui16 blockIndex;
    // TODO(Ben): ui32 instead for massive trees? Use leftover bits for Y?
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

    static inline int getChunkXOffset(ui16 chunkOffset) {
        return (int)((chunkOffset >> 10) & 0x1F) - 0x7;
    }
    static inline int getChunkYOffset(ui16 chunkOffset) {
        return (int)((chunkOffset >> 5) & 0x1F) - 0x7;
    }
    static inline int getChunkZOffset(ui16 chunkOffset) {
        return (int)(chunkOffset & 0x1F) - 0x7;
    }
private:
    enum TreeDir {
        TREE_LEFT = 0, TREE_BACK, TREE_RIGHT, TREE_FRONT, TREE_UP, TREE_DOWN, TREE_NO_DIR
    };

    void makeTrunkSlice(ui16 chunkOffset, const TreeTrunkProperties& props);
    void generateBranch(ui16 chunkOffset, int x, int y, int z, ui32 segments, f32 length, f32 width, const f32v3& dir, const TreeBranchProperties& props);
    void generateLeaves(ui16 chunkOffset, int x, int y, int z, const TreeLeafProperties& props);
    void generateRoundLeaves(ui16 chunkOffset, int x, int y, int z, const TreeLeafProperties& props);
    void directionalMove(ui16& blockIndex, ui16 &chunkOffset, TreeDir dir);
   
    std::vector<FloraNode>* m_fNodes;
    std::vector<FloraNode>* m_wNodes;
    TreeData m_treeData;
    FloraData m_floraData;
    ui16 m_centerX, m_centerY, m_centerZ;
};

#endif // NFloraGenerator_h__
