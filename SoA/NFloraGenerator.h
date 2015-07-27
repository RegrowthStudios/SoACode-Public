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
#include "soaUtils.h"

// 0111111111 0111111111 0111111111 = 0x1FF7FDFF
#define NO_CHUNK_OFFSET 0x1FF7FDFF

// Will not work for chunks > 32^3
struct FloraNode {
    FloraNode(ui16 blockID, ui16 blockIndex, ui32 chunkOffset) :
        blockID(blockID), blockIndex(blockIndex), chunkOffset(chunkOffset) {
    };
    ui16 blockID;
    ui16 blockIndex;
    // TODO(Ben): ui32 instead for massive trees? Use leftover bits for Y?
    ui32 chunkOffset; ///< Packed 00 XXXXXXXXXX YYYYYYYYYY ZZZZZZZZZZ for positional offset. 00111 == 0
};

#define SC_NO_PARENT 0x7FFFu

struct SCRayNode {
    SCRayNode(const f32v3& pos, ui16 parent, f32 width, ui16 trunkPropsIndex) :
        pos(pos), trunkPropsIndex(trunkPropsIndex), wasVisited(false), parent(parent), width(width) {};
    f32v3 pos;
    struct {
        ui16 trunkPropsIndex : 15;
        bool wasVisited : 1;
    };
    ui16 parent;
    f32 width;
};
static_assert(sizeof(SCRayNode) == 24, "Size of SCRayNode is not 24");

struct SCTreeNode {
    SCTreeNode(ui16 rayNode) :
        rayNode(rayNode), dir(0.0f) {};
    ui16 rayNode;
    f32v3 dir;
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
    void generateTree(const NTreeType* type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui32 chunkOffset = NO_CHUNK_OFFSET, ui16 blockIndex = 0);
    /// Generates standalone flora.
    void generateFlora(const FloraType* type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui32 chunkOffset = NO_CHUNK_OFFSET, ui16 blockIndex = 0);
    /// Generates a specific tree's properties
    static void generateTreeProperties(const NTreeType* type, f32 age, OUT TreeData& tree);
    static void generateFloraProperties(const FloraType* type, f32 age, OUT FloraData& flora);

    void spaceColonization(const f32v3& startPos);

    static inline int getChunkXOffset(ui32 chunkOffset) {
        return (int)((chunkOffset >> 20) & 0x3FF) - 0x1FF;
    }
    static inline int getChunkYOffset(ui32 chunkOffset) {
        return (int)((chunkOffset >> 10) & 0x3FF) - 0x1FF;
    }
    static inline int getChunkZOffset(ui32 chunkOffset) {
        return (int)(chunkOffset & 0x3FF) - 0x1FF;
    }
private:
    enum TreeDir {
        TREE_LEFT = 0, TREE_BACK, TREE_RIGHT, TREE_FRONT, TREE_UP, TREE_DOWN, TREE_NO_DIR
    };

    void makeTrunkSlice(ui32 chunkOffset, const TreeTrunkProperties& props);
    void generateBranch(ui32 chunkOffset, int x, int y, int z, f32 length, f32 width, f32 endWidth, f32v3 dir, bool makeLeaves, const TreeBranchProperties& props);
    void generateSCBranches();
    void generateLeaves(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props);
    void generateRoundLeaves(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props);
    void generateEllipseLeaves(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props);
    void generateMushroomCap(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props);
   
    std::set<ui32> m_scLeafSet;
    std::vector<SCRayNode> m_scRayNodes;
    std::vector<SCTreeNode> m_scNodes;
    std::vector<TreeTrunkProperties> m_scTrunkProps; ///< Stores branch properties for nodes
    std::vector<FloraNode>* m_fNodes;
    std::vector<FloraNode>* m_wNodes;
    TreeData m_treeData;
    FloraData m_floraData;
    i32v3 m_center;
    ui32 m_h; ///< Current height along the tree
    FastRandGenerator m_rGen;
};

#endif // NFloraGenerator_h__
