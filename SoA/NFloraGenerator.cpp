#include "stdafx.h"
#include "NFloraGenerator.h"

void NFloraGenerator::generateChunkFlora(const Chunk* chunk, const PlanetHeightData* heightData, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes) {
    f32 age = 0.5; //TODO(Ben): Temporary
    // Iterate all block indices where flora must be generated
    for (auto& blockIndex : chunk->floraToGenerate) {
        const PlanetHeightData& hd = heightData[blockIndex & 0x3FF]; // & 0x3FF = % CHUNK_LAYER
        const Biome* b = hd.biome;
        if (hd.flora < b->flora.size()) {
            // It's a flora
            generateFlora(b->flora[hd.flora].data, age, fNodes, wNodes, NO_CHUNK_OFFSET, blockIndex);
        } else {
            // It's a tree
            generateTree(b->trees[hd.flora - b->flora.size()].data, age, fNodes, wNodes, NO_CHUNK_OFFSET, blockIndex);
        }
    }
}

void NFloraGenerator::generateTree(const NTreeType* type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui16 chunkOffset /*= NO_CHUNK_OFFSET*/, ui16 blockIndex /*= 0*/) {
    generateTreeProperties(type, age, m_treeData);
}

void NFloraGenerator::generateFlora(FloraData type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui16 chunkOffset /*= NO_CHUNK_OFFSET*/, ui16 blockIndex /*= 0*/) {
    // TODO(Ben): Multiblock flora
    fNodes.emplace_back(blockIndex, type.block);
}

#define AGE_LERP(var) lerp(var.min, var.max, age)

inline void setFruitProps(TreeFruitProperties& fruitProps, const TreeTypeFruitProperties& typeProps, f32 age) {
    fruitProps.chance = AGE_LERP(typeProps.chance);
    fruitProps.flora = typeProps.flora;
}

inline void setLeafProps(TreeLeafProperties& leafProps, const TreeTypeLeafProperties& typeProps, f32 age) {
    setFruitProps(leafProps.fruitProps, typeProps.fruitProps, age);
    leafProps.type = typeProps.type;
    switch (leafProps.type) {
        case TreeLeafType::CLUSTER:
            leafProps.cluster.blockID = typeProps.cluster.blockID;
            leafProps.cluster.width = AGE_LERP(typeProps.cluster.width);
            leafProps.cluster.height = AGE_LERP(typeProps.cluster.height);
            break;
        case TreeLeafType::ROUND:
            leafProps.round.blockID = typeProps.round.blockID;
            leafProps.round.radius = AGE_LERP(typeProps.round.radius);
            break;
        case TreeLeafType::PINE:
            leafProps.pine.blockID = typeProps.pine.blockID;
            leafProps.pine.thickness = AGE_LERP(typeProps.pine.thickness);
            break;
        case TreeLeafType::MUSHROOM:
            leafProps.mushroom.capBlockID = typeProps.mushroom.capBlockID;
            leafProps.mushroom.gillBlockID = typeProps.mushroom.gillBlockID;
            leafProps.mushroom.capThickness = AGE_LERP(typeProps.mushroom.capThickness);
            leafProps.mushroom.curlLength = AGE_LERP(typeProps.mushroom.curlLength);
            leafProps.mushroom.lengthMod = AGE_LERP(typeProps.mushroom.lengthMod);
            break;
        default:
            break;
    }
}

inline void setBranchProps(TreeBranchProperties& branchProps, const TreeTypeBranchProperties& typeProps, f32 age) {
    branchProps.barkBlockID = typeProps.barkBlockID;
    branchProps.barkWidth = AGE_LERP(typeProps.barkWidth);
    branchProps.coreWidth = AGE_LERP(typeProps.coreWidth);
    branchProps.branchChance = AGE_LERP(typeProps.branchChance);
    setLeafProps(branchProps.leafProps, typeProps.leafProps, age);
    setFruitProps(branchProps.fruitProps, typeProps.fruitProps, age);
}

void NFloraGenerator::generateTreeProperties(const NTreeType* type, f32 age, OUT TreeData& tree) {
    tree.age = age;
    tree.height = AGE_LERP(type->heightRange);
    tree.trunkProps.resize(type->trunkProps.size());
    for (size_t i = 0; i < tree.trunkProps.size(); ++i) {
        TreeTrunkProperties& tp = tree.trunkProps[i];
        const TreeTypeTrunkProperties& ttp = type->trunkProps[i];
        tp.coreWidth = AGE_LERP(ttp.coreWidth);
        tp.barkWidth = AGE_LERP(ttp.barkWidth);
        tp.branchChance = AGE_LERP(ttp.branchChance);
        tp.coreBlockID = ttp.coreBlockID;
        tp.barkBlockID = ttp.barkBlockID;
        setFruitProps(tp.fruitProps, ttp.fruitProps, age);
        setLeafProps(tp.leafProps, ttp.leafProps, age);
        setBranchProps(tp.branchProps, ttp.branchProps, age);
    }
}
#undef AGE_LERP
