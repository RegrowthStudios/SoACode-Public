#include "stdafx.h"
#include "NFloraGenerator.h"

#include <glm/gtx/quaternion.hpp>

#define X_1 0x100000
#define Y_1 0x400
#define Z_1 0x1

#pragma region helpers
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
#pragma endregion

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

void NFloraGenerator::generateChunkFlora(const Chunk* chunk, const PlanetHeightData* heightData, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes) {
    // Iterate all block indices where flora must be generated
    for (ui16 blockIndex : chunk->floraToGenerate) {
        // Get position
        m_centerX = blockIndex & 0x1F; // & 0x1F = % 32
        m_centerY = blockIndex / CHUNK_LAYER;
        m_centerZ = (blockIndex & 0x3FF) / CHUNK_WIDTH; // & 0x3FF = % 1024
        const PlanetHeightData& hd = heightData[blockIndex & 0x3FF]; // & 0x3FF = % CHUNK_LAYER
        const Biome* b = hd.biome;
        // Seed the generator
        const VoxelPosition3D& vpos = chunk->getVoxelPosition();
        m_rGen.seed(vpos.pos.x + m_centerX, vpos.pos.y + m_centerY, vpos.pos.z + m_centerZ);
        // Get age
        f32 age = m_rGen.genf();
        // Determine which to generate
        if (hd.flora < b->flora.size()) {
            // It's a flora
            generateFlora(b->flora[hd.flora].data, age, fNodes, wNodes, NO_CHUNK_OFFSET, blockIndex);
        } else {
            // It's a tree
            generateTree(b->trees[hd.flora - b->flora.size()].data, age, fNodes, wNodes, NO_CHUNK_OFFSET, blockIndex);
        }
    }
}

#pragma region lerping
// Lerps and rounds
#define LERP_UI16(var) FastConversion<ui16, f32>::floor((b.##var - a.##var) * l + a.##var + 0.5f)
#define LERP_I32(var) FastConversion<i32, f32>::floor((b.##var - a.##var) * l + a.##var + 0.5f)

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
    rvProps.length = LERP_UI16(length);
    rvProps.angle.min = lerp(a.angle.min, b.angle.min, l);
    rvProps.angle.max = lerp(a.angle.max, b.angle.max, l);
    rvProps.segments.min = LERP_I32(segments.min);
    rvProps.segments.max = LERP_I32(segments.max);
    lerpLeafProperties(rvProps.leafProps, a.leafProps, b.leafProps, l);
    // TODO(Ben): Lerp other properties
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
    rvProps.slope.min = LERP_I32(slope.min);
    rvProps.slope.max = LERP_I32(slope.max);
    lerpBranchProperties(rvProps.branchProps, a.branchProps, b.branchProps, l);
    lerpLeafProperties(rvProps.leafProps, a.leafProps, b.leafProps, l);
    // TODO(Ben): Lerp other properties
}

void NFloraGenerator::generateTree(const NTreeType* type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui32 chunkOffset /*= NO_CHUNK_OFFSET*/, ui16 blockIndex /*= 0*/) {
    // Get the properties for this tree
    generateTreeProperties(type, age, m_treeData);

    // Get handles
    m_wNodes = &wNodes;
    m_fNodes = &fNodes;

    // Interpolated trunk properties
    TreeTrunkProperties lerpedTrunkProps;
    const TreeTrunkProperties* trunkProps;

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
        // Build the trunk slice
        makeTrunkSlice(chunkOffset, *trunkProps);
        // Move up
        offsetPositive(m_centerY, Y_1, chunkOffset, 1);
    }
    if (trunkProps->leafProps.type == TreeLeafType::MUSHROOM) {
        generateMushroomCap(chunkOffset, m_centerX, m_centerY, m_centerZ, trunkProps->leafProps);
    }
}

void NFloraGenerator::generateFlora(const FloraType* type, f32 age, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes, ui32 chunkOffset /*= NO_CHUNK_OFFSET*/, ui16 blockIndex /*= 0*/) {
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
#pragma endregion

#pragma region age_lerping
#define AGE_LERP_F32(var) lerp(var.max, var.min, age)
// Lerps and rounds
#define AGE_LERP_I32(var) FastConversion<i32, f32>::floor((var.max - var.min) * age + var.min + 0.5f)
#define AGE_LERP_UI16(var) FastConversion<ui16, f32>::floor((var.max - var.min) * age + var.min + 0.5f)

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
    branchProps.segments.min = AGE_LERP_I32(typeProps.segments.min);
    branchProps.segments.max = AGE_LERP_I32(typeProps.segments.max);
    branchProps.angle = typeProps.angle;
    branchProps.endSizeMult = typeProps.endSizeMult;
    branchProps.length = AGE_LERP_UI16(typeProps.length);
    branchProps.branchChance = AGE_LERP_F32(typeProps.branchChance);
    setLeafProps(branchProps.leafProps, typeProps.leafProps, age);
    setFruitProps(branchProps.fruitProps, typeProps.fruitProps, age);
}

void NFloraGenerator::generateTreeProperties(const NTreeType* type, f32 age, OUT TreeData& tree) {
    tree.age = age;
    tree.height = AGE_LERP_UI16(type->height);
    tree.trunkProps.resize(type->trunkProps.size());
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
        setFruitProps(tp.fruitProps, ttp.fruitProps, age);
        setLeafProps(tp.leafProps, ttp.leafProps, age);
        setBranchProps(tp.branchProps, ttp.branchProps, age);
    }
}
void NFloraGenerator::generateFloraProperties(const FloraType* type, f32 age, OUT FloraData& flora) {
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
#pragma endregion

// TODO(Ben): Need to handle different shapes than just round
void NFloraGenerator::makeTrunkSlice(ui32 chunkOffset, const TreeTrunkProperties& props) {
    // This function is so clever
    int width = (int)(props.coreWidth + props.barkWidth);
    int innerWidth2 = (int)(props.coreWidth * props.coreWidth);
    int woodWidth2 = width * width;
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
    int x = m_centerX;
    int z = m_centerZ;
    width += 1; // Pad out one so we can check branches on small trees
    offsetNegative(x, z, X_1, Z_1, chunkOffset, width);
    x += width;
    z += width;
    // Y voxel offset
    int yOff = m_centerY * CHUNK_LAYER;

    for (int dz = -width; dz <= width; ++dz) {
        for (int dx = -width; dx <= width; ++dx) {
            int dist2 = dx * dx + dz * dz;
            if (dist2 < woodWidth2) { // Wood block
                // Get position
                i32v2 pos(x + dx, z + dz);
                ui32 chunkOff = chunkOffset;
                addChunkOffset(pos, chunkOff);
                ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);
                if (dist2 < innerWidth2) {
                    m_wNodes->emplace_back(props.coreBlockID, blockIndex, chunkOff);
                } else {
                    m_wNodes->emplace_back(props.barkBlockID, blockIndex, chunkOff);
                }
            } else if (dist2 < woodWidth2p1) { // Fruit and branching
                // Get position
                i32v2 pos(x + dx, z + dz);
                ui32 chunkOff = chunkOffset;
                addChunkOffset(pos, chunkOff);
                ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);

                if (m_rGen.genf() < props.branchChance) {

                    ui32 segments = round(m_rGen.genf() * (props.branchProps.segments.max - props.branchProps.segments.min) + props.branchProps.segments.min);
                    if (segments > 0) {
                        f32 angle = m_rGen.genf() * (props.branchProps.angle.max - props.branchProps.angle.min) + props.branchProps.angle.min;
                        // Interpolate the angle
                        // TODO(Ben): Disk offset
                        f32v3 dir = glm::normalize(lerp(glm::normalize(f32v3(dx, 0.0f, dz)), f32v3(0.0f, 1.0f, 0.0f), angle / 90.0f));
                        generateBranch(chunkOff, pos.x, m_centerY, pos.y, segments, props.branchProps.length / segments, props.branchProps.coreWidth + props.branchProps.barkWidth,
                                       dir, props.branchProps);
                    }
                } else if (leafBlockID) {
                    m_fNodes->emplace_back(leafBlockID, blockIndex, chunkOff);
                }
            } else if (dist2 < leafWidth2 && leafBlockID) { // Leaves
                // Get position
                i32v2 pos(x + dx, z + dz);
                ui32 chunkOff = chunkOffset;
                addChunkOffset(pos, chunkOff);
                ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);
                m_fNodes->emplace_back(leafBlockID, blockIndex, chunkOff);
            }
        }
    }
}

inline f32 fastFloorf(f32 x) {
    return FastConversion<f32, f32>::floor(x);
}
inline f32 fastCeilf(f64 x) {
    return FastConversion<f32, f32>::ceiling(x);
}

inline f32 minDistance(const f32v3& w, const f32v3& v, const f32v3& p) {
    const f32 l2 = selfDot(v - w);  
    if (l2 == 0.0) return glm::length(p - v);  

    const f32 t = glm::dot(p - v, w - v) / l2;
    if (t < 0.0) return glm::length(p - v);   
    else if (t > 1.0) return glm::length(p - w); 
    const f32v3 projection = v + t * (w - v); 
    return glm::length(p - projection);
}

void NFloraGenerator::generateBranch(ui32 chunkOffset, int x, int y, int z, ui32 segments, f32 length, f32 width, const f32v3& dir, const TreeBranchProperties& props) {
    if (width < 1.0f) return;
    int startX = x;
    int startY = y;
    int startZ = z;
    ui32 startChunkOffset = chunkOffset;
    f32 maxDist = (float)props.length;

    f32 width2 = width * width;
    f32 width2p1 = (width + 1) * (width + 1);

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
    f32 wp1 = width + 1;
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
    f32v3 vec(0.0f);
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
                if (t < 0.0) {
                    dist2 = selfDot(v2);
                } else if (t > 1.0) {
                    dist2 = selfDot(vec - end);
                } else {
                    const f32v3 projection = start + t * ray;
                    dist2 = selfDot(vec - projection);
                }
                
                if (dist2 < width2) {
                    i32v3 pos(x + k, y + i, z + j);
                    ui32 chunkOff = chunkOffset;
                    addChunkOffset(pos, chunkOff);
                    ui16 newIndex = (ui16)(pos.x + pos.y * CHUNK_LAYER + pos.z * CHUNK_WIDTH);
                    m_wNodes->emplace_back(props.coreBlockID, newIndex, chunkOff);
                } else if (dist2 < width2p1) {
                    // Outer edge, check for branching and fruit
                    if (segments > 1 && length >= 2.0f && m_rGen.genf() < props.branchChance) {
                        i32v3 pos(x + k, y + i, z + j);
                        ui32 chunkOff = chunkOffset;
                        addChunkOffset(pos, chunkOff);
                        f32 m = 1.0f;
                        f32v3 newDir = glm::normalize(f32v3(dir.x + m * m_rGen.genf(), dir.y + m * m_rGen.genf(), dir.z + m * m_rGen.genf()));
                        generateBranch(chunkOff, pos.x, pos.y, pos.z, segments, length, width - 1.0f, newDir, props);
                    }
                }
            }
        }
    }

    // Offset to the end of the ray
    x = startX;
    y = startY;
    z = startZ;
    offsetByPos(x, y, z, startChunkOffset, ray);
    
    // Continue on
    // TODO(Ben): Not recursion
    if (segments > 1 && width >= 2.0f) {
        f32 m = 0.5f;
        f32v3 newDir = glm::normalize(f32v3(dir.x + m * (m_rGen.genf() - 0.5f), dir.y + m * (m_rGen.genf() - 0.5f), dir.z + m * (m_rGen.genf() - 0.5f)));
        generateBranch(startChunkOffset, x, y, z, segments - 1, length, width - 1.0f, newDir, props);
    } else {
        // Finish with leaves
        generateLeaves(startChunkOffset, x, y, z, props.leafProps);
    }
}

inline void NFloraGenerator::generateLeaves(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props) {
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
void NFloraGenerator::generateRoundLeaves(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props) {
    int radius = (int)(props.round.hRadius);
    int radius2 = radius * radius;
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
                    m_fNodes->emplace_back(props.round.blockID, blockIndex, chunkOff);
                }
            }
        }
    }
}

void NFloraGenerator::generateEllipseLeaves(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props) {
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

                    m_fNodes->emplace_back(props.round.blockID, blockIndex, chunkOff);
                }
            }
        }
        
        offsetPositive(y, Y_1, chunkOffset, 1);
    }
}

void NFloraGenerator::generateMushroomCap(ui32 chunkOffset, int x, int y, int z, const TreeLeafProperties& props) {
    
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
                            m_fNodes->emplace_back(props.mushroom.capBlockID, blockIndex, chunkOff);
                        } else {
                            m_fNodes->emplace_back(props.mushroom.gillBlockID, blockIndex, chunkOff);
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
                            m_fNodes->emplace_back(props.mushroom.capBlockID, blockIndex, chunkOff);
                        } else {
                            m_fNodes->emplace_back(props.mushroom.gillBlockID, blockIndex, chunkOff);
                        }
                    }
                }
            }
            offsetPositive(y, Y_1, chunkOffset, 1);
        }
    }
}
