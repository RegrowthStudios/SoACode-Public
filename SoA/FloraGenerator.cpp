#include "stdafx.h"
#include "FloraGenerator.h"

#define X_1 0x100000
#define Y_1 0x400
#define Z_1 0x1

#define OUTER_SKIP_MOD 5

#ifdef _WINDOWS
#pragma region helpers
#endif//_WINDOWS
/************************************************************************/
/* Helper Functions                                                     */
/************************************************************************/
/*  These functions allow us to move in chunk-space without branching.  */
/************************************************************************/
// Adds a positive offset
inline void offsetPositive(int& x, int x1, ui32& chunkOffset, int offset) {
    x += offset;
    chunkOffset += x1 * (x / CHUNK_WIDTH);
    x &= 0x1f; // Modulo 32
}
inline void offsetNegative(int& x, int x1, ui32& chunkOffset, int offset) {
    // We are inverting and treating negative as positive here so modulus works
    // without branching
    x = (CHUNK_WIDTH_M1 - x) + offset;
    chunkOffset -= x1 * (x / CHUNK_WIDTH);
    // Modulo 32 and invert back to positive
    x = CHUNK_WIDTH_M1 - (x & 0x1f);
}
inline void offsetNegative(int& x, int& y, int x1, int y1, ui32& chunkOffset, int offset) {
    // We are inverting and treating negative as positive here so modulus works
    // without branching
    x = (CHUNK_WIDTH_M1 - x) + offset;
    y = (CHUNK_WIDTH_M1 - y) + offset;
    chunkOffset -= x1 * (x / CHUNK_WIDTH);
    chunkOffset -= y1 * (y / CHUNK_WIDTH);
    // Modulo 32 and invert back to positive
    x = CHUNK_WIDTH_M1 - (x & 0x1f);
    y = CHUNK_WIDTH_M1 - (y & 0x1f);
}
inline void offsetNegative(int& x, int& y, int& z, ui32& chunkOffset, int offset) {
    // We are inverting and treating negative as positive here so modulus works
    // without branching
    x = (CHUNK_WIDTH_M1 - x) + offset;
    y = (CHUNK_WIDTH_M1 - y) + offset;
    z = (CHUNK_WIDTH_M1 - z) + offset;
    chunkOffset -= X_1 * (x / CHUNK_WIDTH);
    chunkOffset -= Y_1 * (y / CHUNK_WIDTH);
    chunkOffset -= Z_1 * (z / CHUNK_WIDTH);
    // Modulo 32 and invert back to positive
    x = CHUNK_WIDTH_M1 - (x & 0x1f);
    y = CHUNK_WIDTH_M1 - (y & 0x1f);
    z = CHUNK_WIDTH_M1 - (z & 0x1f);
}
inline void addChunkOffset(i32v2& pos, ui32& chunkOffset) {
    // Modify chunk offset
    chunkOffset += X_1 * (pos.x / CHUNK_WIDTH);
    chunkOffset += Z_1 * (pos.y / CHUNK_WIDTH);
    pos &= 0x1f; // Modulo 32
}
inline void addChunkOffset(i32v3& pos, ui32& chunkOffset) {
    // Modify chunk offset
    chunkOffset += X_1 * (pos.x / CHUNK_WIDTH);
    chunkOffset += Y_1 * (pos.y / CHUNK_WIDTH);
    chunkOffset += Z_1 * (pos.z / CHUNK_WIDTH);
    // Modulo 32
    pos &= 0x1f; // Modulo 32
}

inline void offsetAxis(int& x, int x1, ui32& chunkOffset, int offset) {
    if (offset < 0) {
        x = (CHUNK_WIDTH_M1 - x) - offset;
        chunkOffset -= x1 * (x / CHUNK_WIDTH);
        x = CHUNK_WIDTH_M1 - (x & 0x1f);
    } else {
        x = x + offset;
        chunkOffset += x1 * (x / CHUNK_WIDTH);
        x &= 0x1f;
    }
}

// Offsets by a floating point position
inline void offsetByPos(int& x, int& y, int& z, ui32& chunkOffset, const f32v3& pos) {
    if (pos.x < 0.0f) {
        x = (CHUNK_WIDTH_M1 - x) - fastFloor(pos.x);
        chunkOffset -= X_1 * (x / CHUNK_WIDTH);
        x = CHUNK_WIDTH_M1 - (x & 0x1f);
    } else {
        x = x + (int)pos.x;
        chunkOffset += X_1 * (x / CHUNK_WIDTH);
        x &= 0x1f;
    }
    if (pos.y < 0.0f) {
        y = (CHUNK_WIDTH_M1 - y) - fastFloor(pos.y);
        chunkOffset -= Y_1 * (y / CHUNK_WIDTH);
        y = CHUNK_WIDTH_M1 - (y & 0x1f);
    } else {
        y = y + (int)pos.y;
        chunkOffset += Y_1 * (y / CHUNK_WIDTH);
        y &= 0x1f;
    }
    if (pos.z < 0.0f) {
        z = (CHUNK_WIDTH_M1 - z) - fastFloor(pos.z);
        chunkOffset -= Z_1 * (z / CHUNK_WIDTH);
        z = CHUNK_WIDTH_M1 - (z & 0x1f);
    } else {
        z = z + (int)pos.z;
        chunkOffset += Z_1 * (z / CHUNK_WIDTH);
        z &= 0x1f;
    }
}
inline void offsetByPos(int& x, int& y, int& z, ui32& chunkOffset, const i32v3& pos) {
    if (pos.x < 0) {
        x = (CHUNK_WIDTH_M1 - x) - pos.x;
        chunkOffset -= X_1 * (x / CHUNK_WIDTH);
        x = CHUNK_WIDTH_M1 - (x & 0x1f);
    } else {
        x = x + pos.x;
        chunkOffset += X_1 * (x / CHUNK_WIDTH);
        x &= 0x1f;
    }
    if (pos.y < 0) {
        y = (CHUNK_WIDTH_M1 - y) - pos.y;
        chunkOffset -= Y_1 * (y / CHUNK_WIDTH);
        y = CHUNK_WIDTH_M1 - (y & 0x1f);
    } else {
        y = y + pos.y;
        chunkOffset += Y_1 * (y / CHUNK_WIDTH);
        y &= 0x1f;
    }
    if (pos.z < 0) {
        z = (CHUNK_WIDTH_M1 - z) - pos.z;
        chunkOffset -= Z_1 * (z / CHUNK_WIDTH);
        z = CHUNK_WIDTH_M1 - (z & 0x1f);
    } else {
        z = z + pos.z;
        chunkOffset += Z_1 * (z / CHUNK_WIDTH);
        z &= 0x1f;
    }
}
/************************************************************************/
/* End Helper Functions                                                 */
/************************************************************************/
#ifdef _WINDOWS
#pragma endregion
#endif//_WINDOWS

// Smooths an input factor on the range 0-1
inline void smoothInterpFactor(f32& l, const FloraInterpType& type) {
    switch (type) {
        case FloraInterpType::HERMITE:
            l = hermite(l);
            break;
        case FloraInterpType::COSINE:
            // TODO(Ben): cos lookup table
            l = (f32)(1.0 - cos((f64)l * M_PI_2));
            break;
        case FloraInterpType::SINE:
            // TODO(Ben): sin lookup table
            l = (f32)(sin((f64)l * M_PI_2));
            break;
        default:
            break;
    }
}

void FloraGenerator::generateChunkFlora(const Chunk* chunk, const PlanetHeightData* heightData, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes) {
    // Iterate all block indices where flora must be generated
    for (ui16 blockIndex : chunk->floraToGenerate) {
        // Get position
        m_center.x = blockIndex & 0x1F; // & 0x1F = % 32
        m_center.y = blockIndex / CHUNK_LAYER;
        m_center.z = (blockIndex & 0x3FF) / CHUNK_WIDTH; // & 0x3FF = % 1024
        const PlanetHeightData& hd = heightData[blockIndex & 0x3FF]; // & 0x3FF = % CHUNK_LAYER
        const Biome* b = hd.biome;
        // Seed the generator
        const VoxelPosition3D& vpos = chunk->getVoxelPosition();
        m_rGen.seed(vpos.pos.x + m_center.x, vpos.pos.y + m_center.y, vpos.pos.z + m_center.z);
        // Get age
        f32 age = (f32)m_rGen.genlf();
        // Determine which to generate
        if (hd.flora < b->flora.size()) {
            // It's a flora
            generateFlora(b->flora[hd.flora].data, age, fNodes, wNodes, NO_CHUNK_OFFSET, blockIndex);
        } else {
            // It's a tree
            generateTree(b->trees[hd.flora - b->flora.size()].data, age, fNodes, wNodes, b->genData, NO_CHUNK_OFFSET, blockIndex);
        }
    }
}

#ifdef _WINDOWS
#pragma region lerping
#endif//_WINDOWS

// Lerps and rounds
#define LERP_UI16(var) FastConversion<f32, ui16>::floor((b.var - a.var) * l + a.var + 0.5f)
#define LERP_I32(var) fastFloor((f32)(b.var - a.var) * l + (f32)a.var + 0.5f)

inline void lerpFruitProperties(TreeFruitProperties& rvProps, const TreeFruitProperties& a, const TreeFruitProperties& b, f32 l) {
    rvProps.chance = lerp(a.chance, b.chance, l);
    if (l < 0.5) {
        rvProps.flora = a.flora;
    } else {
        rvProps.flora = b.flora;
    }
}

inline void lerpLeafProperties(TreeLeafProperties& rvProps, const TreeLeafProperties& a, const TreeLeafProperties& b, f32 l) {
    const TreeLeafProperties* blockP;
    if (l < 0.5f) {
        blockP = &a;
    } else {
        blockP = &b;
    }
    rvProps.type = blockP->type;
    switch (rvProps.type) {
        case TreeLeafType::ROUND:
            rvProps.round.blockID = blockP->round.blockID;
            if (a.type == b.type) {
                rvProps.round.vRadius = LERP_UI16(round.vRadius);
                rvProps.round.hRadius = LERP_UI16(round.hRadius);
            } else {
                rvProps.round.vRadius = blockP->round.vRadius;
                rvProps.round.hRadius = blockP->round.hRadius;
            }
            break;
        case TreeLeafType::PINE:
            rvProps.pine.blockID = blockP->pine.blockID;
            if (a.type == b.type) {
                rvProps.pine.oRadius = LERP_UI16(pine.oRadius);
                rvProps.pine.iRadius = LERP_UI16(pine.iRadius);
                rvProps.pine.period = LERP_UI16(pine.period);
            } else {
                rvProps.pine.oRadius = blockP->pine.oRadius;
                rvProps.pine.iRadius = blockP->pine.iRadius;
                rvProps.pine.period = blockP->pine.period;
            }
            break;
        case TreeLeafType::MUSHROOM:
            rvProps.mushroom.capBlockID = blockP->mushroom.capBlockID;
            rvProps.mushroom.gillBlockID = blockP->mushroom.gillBlockID;
            rvProps.mushroom.interp = blockP->mushroom.interp;
            if (a.type == b.type) {
                rvProps.mushroom.tvRadius = LERP_UI16(mushroom.tvRadius);
                rvProps.mushroom.thRadius = LERP_UI16(mushroom.thRadius);
                rvProps.mushroom.bvRadius = LERP_UI16(mushroom.bvRadius);
                rvProps.mushroom.bhRadius = LERP_UI16(mushroom.bhRadius);
                rvProps.mushroom.bLength = LERP_UI16(mushroom.bLength);
                rvProps.mushroom.capWidth = LERP_UI16(mushroom.capWidth);
                rvProps.mushroom.gillWidth = LERP_UI16(mushroom.gillWidth);
            } else {
                rvProps.mushroom.tvRadius = blockP->mushroom.tvRadius;
                rvProps.mushroom.thRadius = blockP->mushroom.thRadius;
                rvProps.mushroom.bvRadius = blockP->mushroom.bvRadius;
                rvProps.mushroom.bhRadius = blockP->mushroom.bhRadius;
                rvProps.mushroom.bLength = blockP->mushroom.bLength;
                rvProps.mushroom.capWidth = blockP->mushroom.capWidth;
                rvProps.mushroom.gillWidth = blockP->mushroom.gillWidth;
            }
            break;
        default:
            break;
    }
    lerpFruitProperties(rvProps.fruitProps, a.fruitProps, b.fruitProps, l);
}

inline void lerpBranchProperties(TreeBranchProperties& rvProps, const TreeBranchProperties& a, const TreeBranchProperties& b, f32 l) {
    // Block IDs
    if (l < 0.5f) {
        rvProps.barkBlockID = a.barkBlockID;
        rvProps.coreBlockID = a.coreBlockID;
    } else {
        rvProps.barkBlockID = b.barkBlockID;
        rvProps.coreBlockID = b.coreBlockID;
    }
    // Lerp the rest
    rvProps.coreWidth = LERP_UI16(coreWidth);
    rvProps.barkWidth = LERP_UI16(barkWidth);
    rvProps.branchChance = lerp(a.branchChance, b.branchChance, l);
    rvProps.changeDirChance = lerp(a.changeDirChance, b.changeDirChance, l);
    rvProps.widthFalloff = lerp(a.widthFalloff, b.widthFalloff, l);
    if (rvProps.widthFalloff == 0.0f) rvProps.widthFalloff = 0.1f; // Avoid div by zero
    rvProps.angle.min = lerp(a.angle.min, b.angle.min, l);
    rvProps.angle.max = lerp(a.angle.max, b.angle.max, l);
    rvProps.subBranchAngle.min = lerp(a.subBranchAngle.min, b.subBranchAngle.min, l);
    rvProps.subBranchAngle.max = lerp(a.subBranchAngle.max, b.subBranchAngle.max, l);
    lerpLeafProperties(rvProps.leafProps, a.leafProps, b.leafProps, l);
    lerpFruitProperties(rvProps.fruitProps, a.fruitProps, b.fruitProps, l);
}

inline void lerpTrunkProperties(TreeTrunkProperties& rvProps, const TreeTrunkProperties& a, const TreeTrunkProperties& b, f32 heightRatio) {
    // TODO(Ben): Other interpolation types
    f32 l = (heightRatio - a.loc) / (b.loc - a.loc);
    // Hermite interpolation for smooth curve
    smoothInterpFactor(l, a.interp);
    rvProps.interp = a.interp;

    // Block IDs
    if (l < 0.5f) {
        rvProps.barkBlockID = a.barkBlockID;
        rvProps.coreBlockID = a.coreBlockID;
    } else {
        rvProps.barkBlockID = b.barkBlockID;
        rvProps.coreBlockID = b.coreBlockID;
    }
    // Lerp the rest
    rvProps.coreWidth = LERP_UI16(coreWidth);
    rvProps.barkWidth = LERP_UI16(barkWidth);
    rvProps.branchChance = lerp(a.branchChance, b.branchChance, l);
    rvProps.changeDirChance = lerp(a.changeDirChance, b.changeDirChance, l);
    rvProps.slope = LERP_I32(slope);
    lerpBranchProperties(rvProps.branchProps, a.branchProps, b.branchProps, l);
    lerpLeafProperties(rvProps.leafProps, a.leafProps, b.leafProps, l);
    lerpFruitProperties(rvProps.fruitProps, a.fruitProps, b.fruitProps, l);
}

struct DirLookup {
    int axis;
    int one;
    int sign;
};
const DirLookup DIR_AXIS_LOOKUP[4] = { {0, X_1, -1}, {2, Z_1, -1}, {0, X_1, 1}, {2, Z_1, 1} };

void FloraGenerator::generateTree(const NTreeType* type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, const PlanetGenData* genData, ui32 chunkOffset /*= NO_CHUNK_OFFSET*/, ui16 blockIndex /*= 0*/) {
    // Get the properties for this tree
    // TODO(Ben): Temp
    m_genData = genData;
    age = 1.0f;
    m_currChunkOff = 0;
    generateTreeProperties(type, age, m_treeData);
    m_nodeFields.reserve(m_treeData.height / CHUNK_WIDTH + 3);
    m_nodeFieldsMap.reserve(200);
    // Get handles
    m_wNodes = &wNodes;
    m_fNodes = &fNodes;

    f32v3 m_startPos = f32v3(m_center) +
        f32v3(CHUNK_WIDTH * getChunkXOffset(chunkOffset),
        CHUNK_WIDTH * getChunkYOffset(chunkOffset),
        CHUNK_WIDTH * getChunkZOffset(chunkOffset));

    // Interpolated trunk properties
    TreeTrunkProperties lerpedTrunkProps;
    const TreeTrunkProperties* trunkProps = nullptr;

    { // Generate the trunk
        int scNodeStep = m_treeData.height / m_treeData.branchPoints;
        if (scNodeStep == 0) scNodeStep = INT_MAX; // Prevent div by 0
        int scNodeOffset = m_treeData.height % scNodeStep;

        ui32 pointIndex = 0;
        for (m_h = 0; m_h < m_treeData.height; ++m_h) {
            // Get height ratio for interpolation purposes
            f32 heightRatio = (f32)m_h / m_treeData.height;
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
            // Check for potential branch point
            m_hasStoredTrunkProps = false;
            if ((m_h + scNodeOffset) % scNodeStep == 0) {
                f32 width = (f32)(trunkProps->branchProps.coreWidth + trunkProps->branchProps.barkWidth);
                if (width > 0.0f) {
                    m_scNodes.emplace_back(m_scRayNodes.size());
                    m_scRayNodes.emplace_back(f32v3(m_center) +
                                              f32v3(CHUNK_WIDTH * getChunkXOffset(chunkOffset),
                                              CHUNK_WIDTH * getChunkYOffset(chunkOffset),
                                              CHUNK_WIDTH * getChunkZOffset(chunkOffset)),
                                              SC_NO_PARENT,
                                              m_scTrunkProps.size());
                    m_scTrunkProps.push_back(*trunkProps);
                    m_hasStoredTrunkProps = true;
                }
            }
            // Build the trunk slice
            makeTrunkSlice(chunkOffset, *trunkProps);
            // Move up
            offsetPositive(m_center.y, Y_1, chunkOffset, 1);
            // Check for dir chance
            if (m_rGen.genlf() <= trunkProps->changeDirChance) {
                m_treeData.currentDir = m_rGen.gen() & 3; // & 3 == % 4
            }
            // Move sideways with slope if needed
            if (m_h % trunkProps->slope == (unsigned int)(trunkProps->slope - 1)) {
                // Place a block so we don't have any floating parts when width is 1
                if (trunkProps->coreWidth + trunkProps->barkWidth == 1) {
                    if (trunkProps->coreWidth) {
                        ui16 blockIndex = (ui16)(m_center.x + m_center.y * CHUNK_LAYER + m_center.z * CHUNK_WIDTH);
                        tryPlaceNode(m_wNodes, 3, trunkProps->coreBlockID, blockIndex, chunkOffset);
                    } else {
                        ui16 blockIndex = (ui16)(m_center.x + m_center.y * CHUNK_LAYER + m_center.z * CHUNK_WIDTH);
                        tryPlaceNode(m_wNodes, 3, trunkProps->barkBlockID, blockIndex, chunkOffset);
                    }
                }
                const DirLookup& dir = DIR_AXIS_LOOKUP[m_treeData.currentDir];
                offsetAxis(m_center[dir.axis], dir.one, chunkOffset, dir.sign);
            }
        }
    }
    if (trunkProps->leafProps.type == TreeLeafType::MUSHROOM) {
        generateMushroomCap(chunkOffset, m_center.x, m_center.y, m_center.z, trunkProps->leafProps);
    }

    // Branches
    if (m_treeData.branchVolumes.size()) {
        spaceColonization(m_startPos);
        std::vector<SCTreeNode>().swap(m_scNodes);
    }

    // Generate deferred branches so they don't conflict with space colonization
    for (auto& it : m_branchesToGenerate) {
        TreeBranchProperties& bp = m_scTrunkProps[it.trunkPropsIndex].branchProps;
        // Defer branching so it doesn't conflict with SC
        f32 angle = (f32)(m_rGen.genlf() * (bp.angle.max - bp.angle.min) + bp.angle.min);
        // Interpolate the angle
        f32v3 dir = glm::normalize(lerp(glm::normalize(f32v3(it.dx, 0.0f, it.dz)), f32v3(0.0f, 1.0f, 0.0f), angle / M_PI_2F));
        f32 width = (f32)(bp.coreWidth + bp.barkWidth);
        f32 length = width / bp.widthFalloff;
        // Determine float position
        f32v3 pos;
        pos.x = (f32)((it.blockIndex & 0x1F) + getChunkXOffset(it.chunkOffset) * CHUNK_WIDTH); // & 0x1F = % 32
        pos.y = (f32)(it.blockIndex / CHUNK_LAYER + getChunkYOffset(it.chunkOffset) * CHUNK_WIDTH);
        pos.z = (f32)((it.blockIndex & 0x3FF) / CHUNK_WIDTH + getChunkZOffset(it.chunkOffset) * CHUNK_WIDTH); // & 0x3FF = % 1024
        // Add root
        ui16 parent = m_scRayNodes.size();
        m_scRayNodes.emplace_back(pos, SC_NO_PARENT, it.trunkPropsIndex);
        // Determine chain length
        int numSegments = (int)(length * bp.changeDirChance);
        numSegments++;
        length /= numSegments;
        // Create branch chain
        int i = 0;
        while (true) {
            pos += dir * length;
            m_scRayNodes.emplace_back(pos, parent, it.trunkPropsIndex);
            // Check end condition
            if (++i == numSegments) break;
            // Get new dir
            newDirFromAngle(dir, bp.angle.min, bp.angle.max);
            parent = m_scRayNodes.size() - 1;
            if (parent >= 32767) {
                break;
            }
        }
        // Last node is leaf
        m_scLeafSet.insert(m_scRayNodes.size() - 1);
    }

    // Place nodes for branches
    if (m_scRayNodes.size()) {
        generateSCBranches();
        std::vector<SCRayNode>().swap(m_scRayNodes);
        std::set<ui32>().swap(m_scLeafSet);
    }

    // Place leaves last to prevent node overlap
    for (auto& it : m_leavesToPlace) {
        m_center.x = blockIndex & 0x1F; // & 0x1F = % 32
        m_center.y = blockIndex / CHUNK_LAYER;
        m_center.z = (blockIndex & 0x3FF) / CHUNK_WIDTH; // & 0x3FF = % 1024
        generateLeaves(it.chunkOffset, it.blockIndex & 0x1F,
                       it.blockIndex / CHUNK_LAYER,
                       (it.blockIndex & 0x3FF) / CHUNK_WIDTH,
                       *it.leafProps);

    }
    
    // Clear container memory
    std::vector<LeavesToPlace>().swap(m_leavesToPlace);
    std::vector<BranchToGenerate>().swap(m_branchesToGenerate);
    std::vector<TreeTrunkProperties>().swap(m_scTrunkProps);
    std::unordered_map<ui32, ui32>().swap(m_nodeFieldsMap);
    std::vector<NodeField>().swap(m_nodeFields);
}

void FloraGenerator::generateFlora(const FloraType* type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui32 chunkOffset /*= NO_CHUNK_OFFSET*/, ui16 blockIndex /*= 0*/) {
    FloraData data;
    generateFloraProperties(type, age, data);

    // Get position
    int x = blockIndex & 0x1F; // & 0x1F = % 32
    int y = blockIndex / CHUNK_LAYER;
    int z = (blockIndex & 0x3FF) / CHUNK_WIDTH; // & 0x3FF = % 1024

    do {
        if (data.dir == TREE_UP) {
            for (m_h = 0; m_h < data.height; ++m_h) {
                fNodes.emplace_back(data.block, blockIndex, chunkOffset);
                // Move up
                offsetPositive(y, Y_1, chunkOffset, 1);
                blockIndex = x + y * CHUNK_LAYER + z * CHUNK_WIDTH;
            }
        } else if (data.dir == TREE_DOWN) {
            for (m_h = 0; m_h < data.height; ++m_h) {
                fNodes.emplace_back(data.block, blockIndex, chunkOffset);
                // Move up
                offsetNegative(y, Y_1, chunkOffset, 1);
                blockIndex = x + y * CHUNK_LAYER + z * CHUNK_WIDTH;
            }
        } else {
            // TODO(Ben): Implement
        }
        // Go on to sub flora if one exists
        if (data.nextFlora) {
            generateFloraProperties(data.nextFlora, age, data);
        } else {
            break;
        }
    } while (true);
}
#undef LERP_UI16
#undef LERP_I32

#ifdef _WINDOWS
#pragma endregion

#pragma region age_lerping
#endif//_WINDOWS

#define AGE_LERP_F32(var) lerp(var.max, var.min, age)
// Lerps and rounds
#define AGE_LERP_I32(var) fastFloor((f32)(var.max - var.min) * age + (f32)var.min + 0.5f)
#define AGE_LERP_UI16(var) FastConversion<f32, ui16>::floor((f32)(var.max - var.min) * age + (f32)var.min + 0.5f)

inline void setFruitProps(TreeFruitProperties& fruitProps, const TreeTypeFruitProperties& typeProps, f32 age) {
    fruitProps.chance = AGE_LERP_F32(typeProps.chance);
    fruitProps.flora = typeProps.flora;
}

inline void setLeafProps(TreeLeafProperties& leafProps, const TreeTypeLeafProperties& typeProps, f32 age) {
    setFruitProps(leafProps.fruitProps, typeProps.fruitProps, age);
    leafProps.type = typeProps.type;
    switch (leafProps.type) {
        case TreeLeafType::ROUND:
            leafProps.round.blockID = typeProps.round.blockID;
            leafProps.round.vRadius = AGE_LERP_UI16(typeProps.round.vRadius);
            leafProps.round.hRadius = AGE_LERP_UI16(typeProps.round.hRadius);
            break;
        case TreeLeafType::PINE:
            leafProps.pine.blockID = typeProps.pine.blockID;
            leafProps.pine.oRadius = AGE_LERP_UI16(typeProps.pine.oRadius);
            leafProps.pine.iRadius = AGE_LERP_UI16(typeProps.pine.iRadius);
            leafProps.pine.period = AGE_LERP_UI16(typeProps.pine.period);
            break;
        case TreeLeafType::MUSHROOM:
            leafProps.mushroom.capBlockID = typeProps.mushroom.capBlockID;
            leafProps.mushroom.gillBlockID = typeProps.mushroom.gillBlockID;
            leafProps.mushroom.interp = typeProps.mushroom.interp;
            leafProps.mushroom.tvRadius = AGE_LERP_UI16(typeProps.mushroom.tvRadius);
            leafProps.mushroom.thRadius = AGE_LERP_UI16(typeProps.mushroom.thRadius);
            leafProps.mushroom.bvRadius = AGE_LERP_UI16(typeProps.mushroom.bvRadius);
            leafProps.mushroom.bhRadius = AGE_LERP_UI16(typeProps.mushroom.bhRadius);
            leafProps.mushroom.bLength = AGE_LERP_UI16(typeProps.mushroom.bLength);
            leafProps.mushroom.capWidth = AGE_LERP_UI16(typeProps.mushroom.capWidth);
            leafProps.mushroom.gillWidth = AGE_LERP_UI16(typeProps.mushroom.gillWidth);
            break;
        default:
            break;
    }
}

inline void setBranchProps(TreeBranchProperties& branchProps, const TreeTypeBranchProperties& typeProps, f32 age) {
    branchProps.coreBlockID = typeProps.coreBlockID;
    branchProps.barkBlockID = typeProps.barkBlockID;
    branchProps.barkWidth = AGE_LERP_UI16(typeProps.barkWidth);
    branchProps.coreWidth = AGE_LERP_UI16(typeProps.coreWidth);
    branchProps.angle = typeProps.angle;
    branchProps.subBranchAngle = typeProps.subBranchAngle;
    branchProps.widthFalloff = AGE_LERP_UI16(typeProps.widthFalloff);
    branchProps.branchChance = AGE_LERP_F32(typeProps.branchChance);
    branchProps.changeDirChance = AGE_LERP_F32(typeProps.changeDirChance);
    setLeafProps(branchProps.leafProps, typeProps.leafProps, age);
    setFruitProps(branchProps.fruitProps, typeProps.fruitProps, age);
}

void FloraGenerator::generateTreeProperties(const NTreeType* type, f32 age, OUT TreeData& tree) {
    tree.age = age;
    tree.height = AGE_LERP_UI16(type->height);
    tree.branchPoints = AGE_LERP_UI16(type->branchPoints);
    tree.branchStep = AGE_LERP_UI16(type->branchStep);
    tree.killMult = AGE_LERP_UI16(type->killMult);
    tree.infRadius = AGE_LERP_F32(type->infRadius);
    // Set branch volume properties
    tree.branchVolumes.resize(type->branchVolumes.size());
    for (size_t i = 0; i < tree.branchVolumes.size(); ++i) {
        BranchVolumeProperties& bp = tree.branchVolumes[i];
        const TreeTypeBranchVolumeProperties& tbp = type->branchVolumes[i];
        bp.height = AGE_LERP_UI16(tbp.height);
        bp.hRadius = AGE_LERP_UI16(tbp.hRadius);
        bp.vRadius = AGE_LERP_UI16(tbp.vRadius);
        bp.points = AGE_LERP_UI16(tbp.points);
    }
    // Set trunk properties
    tree.trunkProps.resize(type->trunkProps.size());
    // TODO(Ben): no rand!!!
    tree.currentDir = rand() & 3; // & 3 == % 4
    for (size_t i = 0; i < tree.trunkProps.size(); ++i) {
        TreeTrunkProperties& tp = tree.trunkProps[i];
        const TreeTypeTrunkProperties& ttp = type->trunkProps[i];
        tp.loc = ttp.loc;
        tp.coreWidth = AGE_LERP_UI16(ttp.coreWidth);
        tp.barkWidth = AGE_LERP_UI16(ttp.barkWidth);
        tp.branchChance = AGE_LERP_F32(ttp.branchChance);
        tp.coreBlockID = ttp.coreBlockID;
        tp.barkBlockID = ttp.barkBlockID;
        tp.interp = ttp.interp;
        // TODO(Ben): no rand!!!
        Range<i32> slopeRange;
        slopeRange.min = rand() % (ttp.slope.min.max - ttp.slope.min.min + 1) + ttp.slope.min.min;
        slopeRange.max = rand() % (ttp.slope.max.max - ttp.slope.max.min + 1) + ttp.slope.max.min;
        tp.slope = AGE_LERP_I32(slopeRange);
        // TODO(Ben): NO RAND
        tp.changeDirChance = ((f32)rand() / RAND_MAX) * (ttp.changeDirChance.max - ttp.changeDirChance.min) + ttp.changeDirChance.min;
        setFruitProps(tp.fruitProps, ttp.fruitProps, age);
        setLeafProps(tp.leafProps, ttp.leafProps, age);
        setBranchProps(tp.branchProps, ttp.branchProps, age);
    }
}
void FloraGenerator::generateFloraProperties(const FloraType* type, f32 age, OUT FloraData& flora) {
    flora.block = type->block;
    flora.slope = AGE_LERP_UI16(type->slope);
    flora.dSlope = AGE_LERP_UI16(type->dSlope);
    flora.height = AGE_LERP_UI16(type->height);
    flora.nextFlora = type->nextFlora;
    switch (type->dir) {
        case FloraDir::SIDE:
            flora.dir = rand() % 4; // TODO(Ben): Rand bad!
            break;
        case FloraDir::UP:
            flora.dir = TREE_UP;
            break;
        case FloraDir::DOWN:
            flora.dir = TREE_DOWN;
            break;
    }
}
#undef AGE_LERP_F32
#undef AGE_LERP_I32
#undef AGE_LERP_UI16

#ifdef _WINDOWS
#pragma endregion
#endif//_WINDOWS

void FloraGenerator::spaceColonization(const f32v3& startPos) {
    std::vector<f32v3> attractPoints;

    int numPoints = 500;

    f32 branchStep = (f32)m_treeData.branchStep;
    f32 infRadius = m_treeData.infRadius;
    f32 infRadius2 = infRadius * infRadius;
    f32 killRadius = (f32)(m_treeData.branchStep * m_treeData.killMult);
    f32 killRadius2 = killRadius * killRadius;

    for (auto& it : m_treeData.branchVolumes) {
        const f32v3 volCenter(startPos.x, startPos.y + it.height, startPos.z);
        const f32 hRadius = (f32)it.hRadius;
        const f32 hDiameter = hRadius * 2.0f;
        const f32 hRadius2 = hRadius * hRadius;
        const f32 vRadius = (f32)it.vRadius;
        const f32 vDiameter = vRadius * 2.0f;
        const f32 vRadius2 = vRadius * vRadius;
        // Generate attraction points
        for (int i = 0; i < it.points; i++) {
            // TODO(Ben): Worry about double and float casting
            f32v3 p(m_rGen.genlf() * hDiameter - hRadius, m_rGen.genlf() * vDiameter - vRadius, m_rGen.genlf() * hDiameter - hRadius);
            // Ellipsoid check
            f32 d = (p.x * p.x) / hRadius2 + (p.y * p.y) / vRadius2 + (p.z * p.z) / hRadius2;
           
            if (d <= 1.0f) {
                attractPoints.push_back(p + volCenter);
            }
        }
    }

    // Iteratively construct the tree
    int iter = 0;
    while (++iter < 10000) {
        if (attractPoints.size() < 5 || m_scNodes.empty()) return;

        for (int i = (int)attractPoints.size() - 1; i >= 0; --i) {
            f32 closestDist = FLT_MAX;
            int closestIndex = -1;
            // Get closest node and attract it towards attract point
            for (size_t j = 0; j < m_scNodes.size(); j++) {
                auto& tn = m_scNodes[j];
                f32v3 v = attractPoints[i] - m_scRayNodes[tn.rayNode].pos;
                f32 dist2 = selfDot(v);
                if (dist2 <= killRadius2) {
                    attractPoints[i] = attractPoints.back();
                    attractPoints.pop_back();
                    closestIndex = -1;
                    break;
                } else if (dist2 <= infRadius2 && dist2 < closestDist) {
                    closestDist = dist2;
                    closestIndex = j;
                }
            }
            if (closestIndex != -1) {
                auto& tn = m_scNodes[closestIndex];
                tn.dir += (attractPoints[i] - m_scRayNodes[tn.rayNode].pos) / closestDist;
            }
        }

        // Generate new nodes and erase unneeded ones
        for (int i = (int)m_scNodes.size() - 1; i >= 0; --i) {
            SCTreeNode& tn = m_scNodes.at(i);
            const SCRayNode& n = m_scRayNodes[tn.rayNode];
            // Self dot?
            if (tn.dir.x && tn.dir.y && tn.dir.z) {
                f32v3 pos = n.pos + glm::normalize(tn.dir) * branchStep;
                tn.dir = f32v3(0.0f);
                ui32 nextIndex = m_scRayNodes.size();
                // Change leaf node
                // TODO(Ben): This can be a vector mebby?
                auto it = m_scLeafSet.find(tn.rayNode);
                if (it != m_scLeafSet.end()) m_scLeafSet.erase(it);
                m_scLeafSet.insert(nextIndex);
    
                // Have to make temp copies with emplace_back
                ui16 trunkPropsIndex = n.trunkPropsIndex;
                m_scRayNodes.emplace_back(pos, tn.rayNode, trunkPropsIndex);
                m_scNodes.emplace_back(nextIndex);

            } else {
                // Remove it since its close to nothing
     //           m_scNodes[i] = m_scNodes.back();
     //           m_scNodes.pop_back();
            }
        }
    }
}

// Priority can not be bigger than 3
inline void FloraGenerator::tryPlaceNode(std::vector<FloraNode>* nodes, ui8 priority, ui16 blockID, ui16 blockIndex, ui32 chunkOffset) {
    if (m_currChunkOff != chunkOffset) {
        m_currChunkOff = chunkOffset;
        auto it = m_nodeFieldsMap.find(chunkOffset);
        if (it == m_nodeFieldsMap.end()) {
            m_currNodeField = m_nodeFields.size();
            m_nodeFields.emplace_back();
            m_nodeFieldsMap.insert(std::pair<ui32, int>(chunkOffset, m_currNodeField));
        } else {
            m_currNodeField = it->second;
        }
    }
    // For memory compression we pack 4 nodes into each val
    NodeField& nf = m_nodeFields[m_currNodeField];
    ui8& val = nf.vals[blockIndex >> 2];
    ui8 shift = (blockIndex & 0x3) << 1;
    if ((((val >> shift) & 0x3) < priority)) {
        nodes->emplace_back(blockID, blockIndex, chunkOffset);
        // Overwrite priority
        val &= ~(0x3 << shift);
        val |= (priority << shift);
    }
}

// TODO(Ben): Need to handle different shapes than just round
void FloraGenerator::makeTrunkSlice(ui32 chunkOffset, const TreeTrunkProperties& props) {
    // This function is so clever
    int width = (int)(props.coreWidth + props.barkWidth);
    if (width == 0) return;
    int innerWidth2 = (int)(props.coreWidth * props.coreWidth);
    int woodWidth2 = width * width;
    int woodWidth2m1 = (width - 1) * (width - 1);
    // Should get layer right outside outer layer
    int woodWidth2p1 = (width + 1) * (width + 1);
    // For leaves
    int leafWidth = 0;
    ui16 leafBlockID = 0;
    // Determine outer leaf shape
    switch (props.leafProps.type) {
        case TreeLeafType::ROUND:
            leafWidth = props.leafProps.round.hRadius;
            leafBlockID = props.leafProps.round.blockID;
            break;
        case TreeLeafType::PINE:
            if (props.leafProps.pine.period == 1) {
                leafWidth = props.leafProps.pine.oRadius;
            } else {
                leafWidth = fastFloor((1.0f - (f32)(m_h % props.leafProps.pine.period) / (props.leafProps.pine.period - 1)) *
                                      (props.leafProps.pine.oRadius - props.leafProps.pine.iRadius) + 0.5f)
                                      + props.leafProps.pine.iRadius;
            }
            leafBlockID = props.leafProps.pine.blockID;
            break;
        default:
            break;
    }
    width += leafWidth;
    int leafWidth2 = width * width;

    // Get position at back left corner
    int x = m_center.x;
    int z = m_center.z;
    width += 1; // Pad out one so we can check branches on small trees
    offsetNegative(x, z, X_1, Z_1, chunkOffset, width);
    x += width;
    z += width;
    // Y voxel offset
    int yOff = m_center.y * CHUNK_LAYER;

    // Distribute branch chance over the circumference of the core
    f64 branchChance = props.branchChance / (M_2_PI * (f64)(props.coreWidth + props.barkWidth));

    for (int dz = -width; dz <= width; ++dz) {
        for (int dx = -width; dx <= width; ++dx) {
            int dist2 = dx * dx + dz * dz;
            if (dist2 < woodWidth2) { // Wood block
                // Get position
                i32v2 pos(x + dx, z + dz);
                ui32 chunkOff = chunkOffset;
                addChunkOffset(pos, chunkOff);
                ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);
                if (dist2 > woodWidth2m1) {
                    if (m_rGen.gen() % OUTER_SKIP_MOD) {
                        if (dist2 < innerWidth2) {
                            tryPlaceNode(m_wNodes, 3, props.coreBlockID, blockIndex, chunkOff);
                        } else {
                            tryPlaceNode(m_wNodes, 3, props.barkBlockID, blockIndex, chunkOff);
                        }
                    }
                } else {
                    if (dist2 < innerWidth2) {
                        tryPlaceNode(m_wNodes, 3, props.coreBlockID, blockIndex, chunkOff);
                    } else {
                        tryPlaceNode(m_wNodes, 3, props.barkBlockID, blockIndex, chunkOff);
                    }
                }
            } else if (dist2 < woodWidth2p1) { // Fruit and branching
                // Get position
                i32v2 pos(x + dx, z + dz);
                ui32 chunkOff = chunkOffset;
                addChunkOffset(pos, chunkOff);
                ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);

                if (m_rGen.genlf() < branchChance) {
                    // TODO(Ben): If check
                    if (!m_hasStoredTrunkProps) {
                        m_scTrunkProps.push_back(props);
                        m_hasStoredTrunkProps = true;
                    }
                    m_branchesToGenerate.emplace_back(blockIndex, chunkOff, dx, dz, m_scTrunkProps.size() - 1);
                } else if (leafWidth && leafBlockID) {
                    tryPlaceNode(m_fNodes, 1, leafBlockID, blockIndex, chunkOff);
                }
            } else if (dist2 < leafWidth2 && leafBlockID) { // Leaves
                // Get position
                i32v2 pos(x + dx, z + dz);
                ui32 chunkOff = chunkOffset;
                addChunkOffset(pos, chunkOff);
                ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);
                tryPlaceNode(m_fNodes, 1, leafBlockID, blockIndex, chunkOff);
            }
        }
    }
}

inline f32 fastFloorf(f32 x) {
    return FastConversion<f32, f32>::floor(x);
}
inline f32 fastCeilf(f32 x) {
    return FastConversion<f32, f32>::ceiling(x);
}

void FloraGenerator::generateBranch(ui32 chunkOffset, int x, int y, int z, f32 length, f32 width, f32 endWidth, f32v3 dir, bool makeLeaves, bool hasParent, const TreeBranchProperties& props) {
    
    if (width < 1.0f) return;
    int startX = x;
    int startY = y;
    int startZ = z;
    ui32 startChunkOffset = chunkOffset;

    f32v3 start(0.0f);
    f32v3 end = dir * length;

    // Compute bounding box
    f32 minX = 0.0f, maxX = 0.0f;
    f32 minY = 0.0f, maxY = 0.0f;
    f32 minZ = 0.0f, maxZ = 0.0f;

    if (end.x < minX) minX = end.x;
    if (end.x > maxX) maxX = end.x;
    if (end.y < minY) minY = end.y;
    if (end.y > maxY) maxY = end.y;
    if (end.z < minZ) minZ = end.z;
    if (end.z > maxZ) maxZ = end.z;

    // Pad the box by width + 1
    f32 wp1 = glm::max(width, endWidth) + 1;
    minX -= wp1;
    maxX += wp1;
    minY -= wp1;
    maxY += wp1;
    minZ -= wp1;
    maxZ += wp1;

    // Round down to voxel position
    i32v3 min(fastFloor(minX), fastFloor(minY), fastFloor(minZ));
    i32v3 max(fastFloor(maxX), fastFloor(maxY), fastFloor(maxZ));

    // Offset to back corner
    offsetByPos(x, y, z, chunkOffset, min);

    // Make start and end relative to min
    start -= min;
    end -= min;

    const f32v3 ray = (end - start);
    const f32 l2 = selfDot(ray);
    // Iterate the volume and check points against line segment
    for (int i = 0; i < max.y - min.y; i++) {
        for (int j = 0; j < max.z - min.z; j++) {
            for (int k = 0; k < max.x - min.x; k++) {
                // http://stackoverflow.com/a/1501725/3346893
                const f32v3 vec(k, i, j);
                const f32v3 v2 = vec - start;
                const f32 t = glm::dot(v2, ray) / l2;

                // Compute distance2
                f32 dist2;
                f32 innerWidth;
                bool canFruit = false;
                if (t < 0.0) {
                    dist2 = selfDot(v2);
                    innerWidth = width;
                } else if (t > 1.0) {
                    // Parent will fill these nodes in.
                    if (hasParent) continue;
                    dist2 = selfDot(vec - end);
                    innerWidth = endWidth;
                } else {
                    const f32v3 projection = start + t * ray;
                    dist2 = selfDot(vec - projection);
                    // Lerp the branch width
                    innerWidth = lerp(width, endWidth, t);
                    canFruit = true;
                }
                    
                f32 width2 = innerWidth * innerWidth;
                f32 width2p1 = (innerWidth + 1) * (innerWidth + 1);
                f32 width2m1 = (innerWidth - 1) * (innerWidth - 1);
                if (width2m1 == 0) width2m1 = FLT_MAX;
      
                if (dist2 < width2) {
                    i32v3 pos(x + k, y + i, z + j);
                    ui32 chunkOff = chunkOffset;
                    addChunkOffset(pos, chunkOff);
                    ui16 newIndex = (ui16)(pos.x + pos.y * CHUNK_LAYER + pos.z * CHUNK_WIDTH);
                    if (dist2 > width2m1 + 1) {
                        if (m_rGen.gen() % OUTER_SKIP_MOD) tryPlaceNode(m_wNodes, 3, props.coreBlockID, newIndex, chunkOff);
                    } else {
                        tryPlaceNode(m_wNodes, 3, props.coreBlockID, newIndex, chunkOff);
                    }
                } else if (canFruit && dist2 < width2p1 && props.fruitProps.flora != FLORA_ID_NONE) {
                    // Distribute fruit chance over the circumference of the branch
                    f64 fruitChance = props.fruitProps.chance / (M_2_PI * (f64)(innerWidth));
                    if (m_rGen.genlf() <= fruitChance) {
                        i32v3 pos(x + k, y + i, z + j);
                        ui32 chunkOff = chunkOffset;
                        addChunkOffset(pos, chunkOff);
                        ui16 newIndex = (ui16)(pos.x + pos.y * CHUNK_LAYER + pos.z * CHUNK_WIDTH);
                        generateFlora(&m_genData->flora.at(props.fruitProps.flora), 1.0f, *m_fNodes, *m_wNodes, chunkOff, newIndex);
                    }
                }
            }
        }
    }

    if (makeLeaves) {
        ui16 newIndex = (ui16)(startX + startY * CHUNK_LAYER + startZ * CHUNK_WIDTH);
        m_leavesToPlace.emplace_back(newIndex, startChunkOffset, &props.leafProps);
    }  
}

void FloraGenerator::generateSCBranches() {

    // Check for performance issues.
    if (m_scRayNodes.size() > 4000) {
        if (m_scRayNodes.size() > 32768) {
            printf("ERROR: Tree has %zuray nodes but limited to 32768\n", m_scRayNodes.size());
            m_scRayNodes.clear();
        } else {
            printf("Performance warning: tree has %zu ray nodes\n", m_scRayNodes.size());
        }
    }

    std::vector <ui32> lNodesToAdd;
    // Set widths and sub branches
    for (auto& l : m_scLeafSet) {
        ui32 i = l;
        while (true) {
            SCRayNode& a = m_scRayNodes[i];
            a.wasVisited = true; // a helpful flag
            i = a.parent;
            if (i == SC_NO_PARENT) break;
            SCRayNode& b = m_scRayNodes[i];
            TreeBranchProperties& tbp = m_scTrunkProps[b.trunkPropsIndex].branchProps;
            f32 nw = a.width + tbp.widthFalloff * m_treeData.branchStep;

            if (b.wasVisited) {
                if (b.width >= nw) break;
                b.width = nw;
                if (b.width > tbp.barkWidth + tbp.coreWidth) {
                    b.width = (f32)(tbp.barkWidth + tbp.coreWidth);
                }
            } else {
                b.width = nw;
                if (b.width > tbp.barkWidth + tbp.coreWidth) {
                    b.width = (f32)(tbp.barkWidth + tbp.coreWidth);
                }
                
                // Calculate sub branching
                if (tbp.branchChance > 0.0f) {
                    f32v3 dir = glm::normalize(b.pos - a.pos);
                    f32 length = glm::length(dir);
                    dir /= length;
                    for (f32 v = 0.0f; v < length; v += 1.0f) {
                        if (m_rGen.genlf() < tbp.branchChance) {
                            f32 width = lerp(a.width, b.width, v / length);
                            // TODO(Ben): Width falloff aint working right?
                            f32 sLength = width / tbp.widthFalloff;
                            // Determine float position
                            f32v3 pos = a.pos + dir * v;
                            
                            // Add root
                            ui16 parent = m_scRayNodes.size();
                            ui16 tpIndex = a.trunkPropsIndex;
                            // a and b are invalidated with emplace_back
                            m_scRayNodes.emplace_back(pos, SC_NO_PARENT, tpIndex);
                            m_scRayNodes.back().width = width;
                            // Determine chain length
                            int numSegments = (int)(sLength * tbp.changeDirChance);
                            numSegments++;
                            sLength /= numSegments;
                            // Create branch chain
                            int j = 0;
                            f32v3 nDir = dir;
                            while (true) {
                                // Get new dir
                                newDirFromAngle(nDir, tbp.subBranchAngle.min, tbp.subBranchAngle.max);
                                pos += nDir * sLength;
                                m_scRayNodes.emplace_back(pos, parent, tpIndex);
                                m_scRayNodes.back().width = width * ((f32)(numSegments - j - 1) / numSegments) + 1.0f;
                                // Check end condition
                                if (++j == numSegments) break;
                                parent = m_scRayNodes.size() - 1;
                                if (parent >= 32767) {
                                    break;
                                }
                            }
                            // Last node is leaf
                            lNodesToAdd.push_back(m_scRayNodes.size() - 1);
                        }
                    }
                }
            }
        }
    }
    for (auto& i : lNodesToAdd) {
        m_scLeafSet.insert(i);
    }

    // Make branches
    int a = 0;
    for (auto& l : m_scLeafSet) {
        ui32 i = l;
        bool hasLeaves = true;
        while (true) {
            SCRayNode& a = m_scRayNodes[i];
            i = a.parent;
            if (i == SC_NO_PARENT) break;
            a.parent = SC_NO_PARENT;
            SCRayNode& b = m_scRayNodes[i];

            f32v3 dir = b.pos - a.pos;
            f32 length = glm::length(dir);
            dir /= length;
           
            i32v3 iPosA(a.pos);
            int x = 0;
            int y = 0;
            int z = 0;
            ui32 chunkOffset = NO_CHUNK_OFFSET;
            offsetByPos(x, y, z, chunkOffset, iPosA);
            generateBranch(chunkOffset, x, y, z, length, a.width, b.width, dir, hasLeaves,
                           b.parent != SC_NO_PARENT, m_scTrunkProps[b.trunkPropsIndex].branchProps);
            hasLeaves = false;
        }
    }
}

inline void FloraGenerator::generateLeaves(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props) {
    // TODO(Ben): OTHER TYPES
    switch (props.type) {
        case TreeLeafType::ROUND:
            if (props.round.vRadius == props.round.hRadius) {
                generateRoundLeaves(chunkOffset, x, y, z, props);
            } else {
                generateEllipseLeaves(chunkOffset, x, y, z, props);
            }
            break;
        case TreeLeafType::MUSHROOM:
            generateMushroomCap(chunkOffset, x, y, z, props);
            break;
        default:
            // Pine leaves cant generate on branches
            break;
    }
}

// Faster than ellipse for when the leaves are perfectly round
void FloraGenerator::generateRoundLeaves(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props) {
    int radius = (int)(props.round.hRadius);
    int radius2 = radius * radius;
    int radius2m1 = (radius - 1) * (radius - 1);
    if (radius2m1 == 0) radius2m1 = INT_MAX;
    // Get position at back left corner
    offsetNegative(x, y, z, chunkOffset, radius - 1);

    x += radius;
    y += radius;
    z += radius;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dz = -radius; dz <= radius; ++dz) {
            for (int dx = -radius; dx <= radius; ++dx) {
                int dist2 = dx * dx + dy * dy + dz * dz;
                if (dist2 < radius2) {
                    i32v3 pos(x + dx, y + dy, z + dz);
                    ui32 chunkOff = chunkOffset;
                    addChunkOffset(pos, chunkOff);
                    ui16 blockIndex = (ui16)(pos.x + pos.y * CHUNK_LAYER + pos.z * CHUNK_WIDTH);
                    if (dist2 > radius2m1) {
                        if (m_rGen.gen() % OUTER_SKIP_MOD) tryPlaceNode(m_fNodes, 1, props.round.blockID, blockIndex, chunkOff);
                    } else {
                        tryPlaceNode(m_fNodes, 1, props.round.blockID, blockIndex, chunkOff);
                    }
                }
            }
        }
    }
}

void FloraGenerator::generateEllipseLeaves(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props) {
    // Offset to bottom
    int offset = props.round.vRadius;
    int tmp = y;
    offsetNegative(y, Y_1, chunkOffset, offset);
    // Use equation of ellipse
    f32 fOffset = (f32)offset;
    f32 a = (f32)props.round.hRadius;
    f32 b = (f32)(props.round.vRadius * props.round.vRadius);
    for (f32 i = 0.0f; i < fOffset * 2.0f + 1.0f; i += 1.0f) {
        f32 ey = fOffset - i;
        f32 radius = sqrtf(1.0f - (ey * ey) / b) * a;
        f32 radius2 = radius * radius;
        f32 radius2m1 = (radius - 1) * (radius - 1);
        if (radius2m1 == 0) radius2m1 = FLT_MAX;
        const int yOff = y * CHUNK_LAYER;
        // Offset to back left
        int offset = fastFloor(radius);
        int x2 = x;
        int z2 = z;
        ui32 innerChunkOffset = chunkOffset;
        offsetNegative(x2, z2, X_1, Z_1, innerChunkOffset, offset);
        x2 += offset;
        z2 += offset;
        // Make the layer
        for (int dz = -offset; dz <= offset; ++dz) {
            for (int dx = -offset; dx <= offset; ++dx) {
                int dist2 = dx * dx + dz * dz;
                if ((f32)dist2 < radius2) {
                    i32v2 pos(x2 + dx, z2 + dz);
                    ui32 chunkOff = innerChunkOffset;
                    addChunkOffset(pos, chunkOff);
                    ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);
                    if (dist2 > radius2m1) {
                        if (m_rGen.gen() % OUTER_SKIP_MOD) tryPlaceNode(m_fNodes, 1, props.round.blockID, blockIndex, chunkOff);
                    } else {
                        tryPlaceNode(m_fNodes, 1, props.round.blockID, blockIndex, chunkOff);
                    }
                }
            }
        }
        
        offsetPositive(y, Y_1, chunkOffset, 1);
    }
}

void FloraGenerator::generateMushroomCap(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props) {
    
    int startY = y;
    ui32 startChunkOffset = chunkOffset;
    if (props.mushroom.tvRadius) { // Top half
        // Offset to middle of cap
        int offset = props.mushroom.tvRadius;
        offsetNegative(y, Y_1, chunkOffset, offset);
        // Parameters
        f32 fOffset = (f32)offset;
        f32 a = (f32)props.mushroom.thRadius;
        f32 b = (f32)(props.mushroom.tvRadius * props.mushroom.tvRadius);
        f32 thickness = (f32)(props.mushroom.capWidth + props.mushroom.gillWidth);

        // Top half
        for (f32 i = 0.0f; i < fOffset + 1.0f; i += 1.0f) {
            // Equation of ellipse
            f32 radius = sqrtf(1.0f - (i * i) / b) * a;
            f32 capRadius2 = radius - props.mushroom.capWidth;
            capRadius2 = capRadius2 * capRadius2;
            f32 innerRadius2 = radius - thickness;
            if (innerRadius2 < 0.0f) {
                innerRadius2 = 0.0f;
            } else {
                innerRadius2 = innerRadius2 * innerRadius2;
            }
            f32 radius2 = radius * radius;
            const int yOff = y * CHUNK_LAYER;
            // Offset to back left
            int innerOffset = fastFloor(radius);
            int x2 = x;
            int z2 = z;
            ui32 innerChunkOffset = chunkOffset;
            offsetNegative(x2, z2, X_1, Z_1, innerChunkOffset, innerOffset);
            x2 += innerOffset;
            z2 += innerOffset;
            // Make the layer
            for (int dz = -innerOffset; dz <= innerOffset; ++dz) {
                for (int dx = -innerOffset; dx <= innerOffset; ++dx) {
                    f32 dist2 = (f32)(dx * dx + dz * dz);
                    if (dist2 < radius2 && dist2 > innerRadius2) {
                        i32v2 pos(x2 + dx, z2 + dz);
                        ui32 chunkOff = innerChunkOffset;
                        addChunkOffset(pos, chunkOff);
                        ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);
                        if (dist2 >= capRadius2) {
                            tryPlaceNode(m_fNodes, 1, props.mushroom.capBlockID, blockIndex, chunkOff);
                        } else {
                            tryPlaceNode(m_fNodes, 1, props.mushroom.gillBlockID, blockIndex, chunkOff);
                        }
                    }
                }
            }
            offsetPositive(y, Y_1, chunkOffset, 1);
        }
    }

    if (props.mushroom.bLength && props.mushroom.bvRadius) { // Bottom half
        y = startY;
        chunkOffset = startChunkOffset;
        // Offset to bottom of cap and skip layers as needed
        int skipped = props.mushroom.bvRadius - props.mushroom.bLength;
        if (skipped < 0) return;
        int offset = props.mushroom.bvRadius - skipped;
        offsetNegative(y, Y_1, chunkOffset, offset + props.mushroom.tvRadius + 1);
        // Parameters
        f32 fOffset = (f32)props.mushroom.bvRadius;
        f32 a = (f32)props.mushroom.bhRadius;
        f32 b = (f32)(props.mushroom.bvRadius * props.mushroom.bvRadius);
        f32 thickness = (f32)(props.mushroom.capWidth + props.mushroom.gillWidth);

        // Top half
        f32 end = fOffset + 1.0f;
        f32 fSkipped = (f32)skipped;
        for (f32 i = fSkipped; i < end; i += 1.0f) {
            // Get lerp factor
            f32 l = (i - fSkipped) / (end - fSkipped);
            smoothInterpFactor(l, props.mushroom.interp);
            // Lerp the a
            f32 a = ((f32)props.mushroom.thRadius - (f32)props.mushroom.bhRadius) * l + props.mushroom.bhRadius;
            // Equation of ellipse
            f32 ey = fOffset - i;
            f32 radius = sqrtf(1.0f - (ey * ey) / b) * a;
            f32 capRadius2 = radius - props.mushroom.capWidth;
            capRadius2 = capRadius2 * capRadius2;
            f32 innerRadius2 = radius - thickness;
            if (innerRadius2 < 0.0f) {
                innerRadius2 = 0.0f;
            } else {
                innerRadius2 = innerRadius2 * innerRadius2;
            }
            f32 radius2 = radius * radius;
            const int yOff = y * CHUNK_LAYER;
            // Offset to back left
            int innerOffset = fastFloor(radius);
            int x2 = x;
            int z2 = z;
            ui32 innerChunkOffset = chunkOffset;
            offsetNegative(x2, z2, X_1, Z_1, innerChunkOffset, innerOffset);
            x2 += innerOffset;
            z2 += innerOffset;
            // Make the layer
            for (int dz = -innerOffset; dz <= innerOffset; ++dz) {
                for (int dx = -innerOffset; dx <= innerOffset; ++dx) {
                    f32 dist2 = (f32)(dx * dx + dz * dz);
                    if (dist2 < radius2 && dist2 > innerRadius2) {
                        i32v2 pos(x2 + dx, z2 + dz);
                        ui32 chunkOff = innerChunkOffset;
                        addChunkOffset(pos, chunkOff);
                        ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);
                        if (dist2 >= capRadius2) {
                            tryPlaceNode(m_fNodes, 1, props.mushroom.capBlockID, blockIndex, chunkOff);
                        } else {
                            tryPlaceNode(m_fNodes, 1, props.mushroom.gillBlockID, blockIndex, chunkOff);
                        }
                    }
                }
            }
            offsetPositive(y, Y_1, chunkOffset, 1);
        }
    }
}

inline void FloraGenerator::newDirFromAngle(f32v3& dir, f32 minAngle, f32 maxAngle) {
    f32 angle = (f32)(m_rGen.genlf() * (maxAngle - minAngle) + minAngle);
    f32v3 relDir(0.0f, cos(angle), sin(angle));
    relDir = glm::angleAxis((f32)(m_rGen.genlf() * 360.0), f32v3(0.0f, 1.0f, 0.0f)) * relDir;
    // Transform relDir relative to dir with change of basis matrix
    f32v3 nz = glm::cross(dir, f32v3(0.0f, 1.0f, 0.0f));
    f32v3 nx = glm::cross(dir, nz);
    f32m3 trans(nx, dir, nz);
    dir = trans * relDir;
}