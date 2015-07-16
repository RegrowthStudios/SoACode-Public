#include "stdafx.h"
#include "NFloraGenerator.h"

#define X_1 0x400
#define Y_1 0x20
#define Z_1 0x1

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

inline void lerpTrunkProperties(TreeTrunkProperties& rvProps, const TreeTrunkProperties& a, const TreeTrunkProperties& b, f32 heightRatio) {
    // TODO(Ben): Other interpolation types
    f32 l = (heightRatio - a.loc) / (b.loc - a.loc);
    // Hermite interpolation for smooth curve
    l = hermite(l);
    // Round IDs
    if (l < 0.5f) {
        rvProps.barkBlockID = a.barkBlockID;
        rvProps.coreBlockID = a.coreBlockID;
    } else {
        rvProps.barkBlockID = b.barkBlockID;
        rvProps.coreBlockID = b.coreBlockID;
    }
    // Lerp the rest
    rvProps.coreWidth = lerp(a.coreWidth, b.coreWidth, l);
    rvProps.barkWidth = lerp(a.barkWidth, b.barkWidth, l);
    rvProps.branchChance = lerp(a.branchChance, b.branchChance, l);
    rvProps.slope.min = lerp(a.slope.min, b.slope.min, l);
    rvProps.slope.max = lerp(a.slope.max, b.slope.max, l);
    // TODO(Ben): Lerp other properties
}

void NFloraGenerator::generateTree(const NTreeType* type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui16 chunkOffset /*= NO_CHUNK_OFFSET*/, ui16 blockIndex /*= 0*/) {
    // Get the properties for this tree
    generateTreeProperties(type, age, m_treeData);

    // Get handles
    m_wNodes = &wNodes;
    m_fNodes = &fNodes;

    // Interpolated trunk properties
    TreeTrunkProperties lerpedTrunkProps;
    const TreeTrunkProperties* trunkProps;
    
    ui32 pointIndex = 0;
    for (ui32 h = 0; h < m_treeData.height; ++h) {
        // TODO(Ben): Don't have to calculate every time
        m_centerX = blockIndex & 0x5; // & 0x5 = % 32
        m_centerY = blockIndex / CHUNK_LAYER;
        m_centerZ = (blockIndex & 0x3FF) / CHUNK_WIDTH; // & 0x3FF = % 1024
        // Get height ratio for interpolation purposes
        f32 heightRatio = (f32)h / m_treeData.height;
        // Do interpolation along data points
        if (pointIndex < m_treeData.trunkProps.size() - 1) {
            if (heightRatio > m_treeData.trunkProps[pointIndex + 1].loc) {
                ++pointIndex;
                if (pointIndex < m_treeData.trunkProps.size() - 1) {
                    lerpTrunkProperties(lerpedTrunkProps, m_treeData.trunkProps[pointIndex], m_treeData.trunkProps[pointIndex + 1], heightRatio);
                    trunkProps = &lerpedTrunkProps;
                } else {
                    // Don't need to interpolate if we are at the last data point
                    trunkProps = &m_treeData.trunkProps.back();
                }
            } else {
                lerpTrunkProperties(lerpedTrunkProps, m_treeData.trunkProps[pointIndex], m_treeData.trunkProps[pointIndex + 1], heightRatio);
                trunkProps = &lerpedTrunkProps;
            }
        } else {
            // Don't need to interpolate if we are at the last data point
            trunkProps = &m_treeData.trunkProps.back();
        }
        // Build the trunk slice
        makeTrunkSlice(chunkOffset, *trunkProps);

        // TODO(Ben): Can optimize out the switch
        directionalMove(blockIndex, chunkOffset, TreeDir::TREE_UP);
    }
}

void NFloraGenerator::generateFlora(FloraData type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui16 chunkOffset /*= NO_CHUNK_OFFSET*/, ui16 blockIndex /*= 0*/) {
    // TODO(Ben): Multiblock flora
    fNodes.emplace_back(type.block, blockIndex, chunkOffset);
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
        tp.loc = ttp.loc;
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

// TODO(Ben): Need to handle different shapes than just round
void NFloraGenerator::makeTrunkSlice(ui16 chunkOffset, const TreeTrunkProperties& props) {
    // This function is so clever
    int width = (int)(props.coreWidth + props.barkWidth);
    int innerWidth2 = (int)(props.coreWidth * props.coreWidth);
    int width2 = width * width;
    // Get position at back left corner
    // We are inverting and treating negative as positive here so modulus works
    int x = (CHUNK_WIDTH - (int)m_centerX) + width;
    int z = (CHUNK_WIDTH - (int)m_centerZ) + width;
    chunkOffset -= X_1 * (x / CHUNK_WIDTH); // >> 5 = / 32
    chunkOffset -= Z_1 * (z / CHUNK_WIDTH);
    // Modulo 32 and invert back to positive
    x = CHUNK_WIDTH - (x & 0x1f);
    z = CHUNK_WIDTH - (z & 0x1f);
    // Y voxel offset
    int yOff = m_centerY * CHUNK_LAYER;

    for (int dz = -width; dz <= width; ++dz) {
        for (int dx = -width; dx <= width; ++dx) {
            int dist2 = dx * dx + dz * dz;
            if (dist2 <= width2) {
                int x2 = x + dx + width;
                int z2 = z + dz + width;
                // Get chunk offset of front right corner
                int chunkOff = chunkOffset;
                chunkOff += X_1 * (x2 / CHUNK_WIDTH); // >> 5 = / 32
                chunkOff += Z_1 * (z2 / CHUNK_WIDTH);
                // Get x,z position in that chunk
                x2 &= 0x1f;
                z2 &= 0x1f;
                ui16 blockIndex = (ui16)(x2 + yOff + z2 * CHUNK_WIDTH);
                if (dist2 < innerWidth2) {
                    m_wNodes->emplace_back(props.coreBlockID, blockIndex, chunkOff);
                } else {
                    m_wNodes->emplace_back(props.barkBlockID, blockIndex, chunkOff);
                }
            }
        }
    }
}

void NFloraGenerator::directionalMove(ui16& blockIndex, ui16 &chunkOffset, TreeDir dir) {
    // Constants for chunkOffset
    // TODO(Ben): Optimize out the modulus

    switch (dir) {
        case TREE_UP:
            blockIndex += CHUNK_LAYER;
            if (blockIndex >= CHUNK_SIZE) {
                blockIndex -= CHUNK_SIZE;
                chunkOffset += Y_1;
            }
            break;
        case TREE_DOWN:
            blockIndex -= CHUNK_LAYER;
            if (blockIndex < 0) {
                blockIndex += CHUNK_SIZE;
                chunkOffset -= Y_1;
            }
            break;
        case TREE_LEFT:
            if (blockIndex % CHUNK_WIDTH) {
                blockIndex--;
            } else {
                blockIndex = blockIndex + CHUNK_WIDTH - 1;
                chunkOffset -= X_1;
            }
            break;
        case TREE_BACK:
            if ((blockIndex % CHUNK_LAYER) / CHUNK_WIDTH) {
                blockIndex -= CHUNK_WIDTH;
            } else {
                blockIndex = blockIndex + CHUNK_LAYER - CHUNK_WIDTH;
                chunkOffset -= Z_1;
            }
            break;
        case TREE_RIGHT:
            if (blockIndex % CHUNK_WIDTH < CHUNK_WIDTH - 1) {
                blockIndex++;
            } else {
                blockIndex = blockIndex - CHUNK_WIDTH + 1;
                chunkOffset += X_1;
            }
            break;
        case TREE_FRONT:
            if ((blockIndex % CHUNK_LAYER) / CHUNK_WIDTH < CHUNK_WIDTH - 1) {
                blockIndex += CHUNK_WIDTH;
            } else {
                blockIndex = blockIndex - CHUNK_LAYER + CHUNK_WIDTH;
                chunkOffset += Z_1;
            }
            break;
        default:
            break;
    }
}