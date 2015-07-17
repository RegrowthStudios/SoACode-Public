#include "stdafx.h"
#include "NFloraGenerator.h"
#include "soaUtils.h"

#define X_1 0x400
#define Y_1 0x20
#define Z_1 0x1

void NFloraGenerator::generateChunkFlora(const Chunk* chunk, const PlanetHeightData* heightData, OUT std::vector<FloraNode>& fNodes, OUT std::vector<FloraNode>& wNodes) {
    f32 age = rand() / (float)RAND_MAX; //TODO(Ben): Temporary
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
    rvProps.coreWidth = lerp(a.coreWidth, b.coreWidth, l);
    rvProps.barkWidth = lerp(a.barkWidth, b.barkWidth, l);
    rvProps.branchChance = lerp(a.branchChance, b.branchChance, l);
    rvProps.length = lerp(a.length, b.length, l);
    // TODO(Ben): Lerp other properties
}

inline void lerpTrunkProperties(TreeTrunkProperties& rvProps, const TreeTrunkProperties& a, const TreeTrunkProperties& b, f32 heightRatio) {
    // TODO(Ben): Other interpolation types
    f32 l = (heightRatio - a.loc) / (b.loc - a.loc);
    // Hermite interpolation for smooth curve
    l = hermite(l);
    // Block IDs
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
    lerpBranchProperties(rvProps.branchProps, a.branchProps, b.branchProps, l);
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
    branchProps.coreBlockID = typeProps.coreBlockID;
    branchProps.barkBlockID = typeProps.barkBlockID;
    branchProps.barkWidth = AGE_LERP(typeProps.barkWidth);
    branchProps.coreWidth = AGE_LERP(typeProps.coreWidth);
    branchProps.length = AGE_LERP(typeProps.length);
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

inline void addChunkOffset(i32v2& pos, ui16& chunkOffset) {
    // Modify chunk offset
    chunkOffset += X_1 * (pos.x / CHUNK_WIDTH); // >> 5 = / 32
    chunkOffset += Z_1 * (pos.y / CHUNK_WIDTH);
    // Modulo 32
    pos &= 0x1f;
}

inline void addChunkOffset(i32v3& pos, ui16& chunkOffset) {
    // Modify chunk offset
    chunkOffset += X_1 * (pos.x / CHUNK_WIDTH); // >> 5 = / 32
    chunkOffset += Y_1 * (pos.y / CHUNK_WIDTH);
    chunkOffset += Z_1 * (pos.z / CHUNK_WIDTH);
    // Modulo 32
    pos &= 0x1f;
}

// TODO(Ben): Need to handle different shapes than just round
void NFloraGenerator::makeTrunkSlice(ui16 chunkOffset, const TreeTrunkProperties& props) {
    // This function is so clever
    int width = (int)(props.coreWidth + props.barkWidth);
    int innerWidth2 = (int)(props.coreWidth * props.coreWidth);
    int width2 = width * width;
    // Should get layer right outside outer layer
    int width2p1 = (width + 1) * (width + 1);
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
                i32v2 pos(x + dx + width, z + dz + width);
                ui16 chunkOff = chunkOffset;
                addChunkOffset(pos, chunkOff);
                ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);
                if (dist2 < innerWidth2) {
                    m_wNodes->emplace_back(props.coreBlockID, blockIndex, chunkOff);
                } else {
                    m_wNodes->emplace_back(props.barkBlockID, blockIndex, chunkOff);
                }
            } else if (dist2 <= width2p1) {
                // TODO(Ben): Eliminate all uses of rand()
                float r = rand() / (float)RAND_MAX;
                if (r < props.branchChance) {
                    i32v2 pos(x + dx + width, z + dz + width);
                    ui16 chunkOff = chunkOffset;
                    addChunkOffset(pos, chunkOff);
                    ui16 blockIndex = (ui16)(pos.x + yOff + pos.y * CHUNK_WIDTH);
                    generateBranch(chunkOff, pos.x, m_centerY, pos.y, 30.0f, 2.0f,
                                   glm::normalize(f32v3(dx, 0.0f, dz)), props.branchProps);
                }
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

void NFloraGenerator::generateBranch(ui16 chunkOffset, int x, int y, int z, f32 length, f32 width, const f32v3& dir, const TreeBranchProperties& props) {
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
    int iMinX = fastFloor(minX);
    int iMaxX = fastFloor(maxX);
    int iMinY = fastFloor(minY);
    int iMaxY = fastFloor(maxY);
    int iMinZ = fastFloor(minZ);
    int iMaxZ = fastFloor(maxZ);
  
    // Get voxel position at bottom back left corner.
    // TODO(Ben): Clean this up with a macro or something
    if (minX < 0.0f) {
        x = (CHUNK_WIDTH - x) - iMinX;
        chunkOffset -= X_1 * (x / CHUNK_WIDTH); // >> 5 = / 32
        x = CHUNK_WIDTH - (x & 0x1f);
    } else {
        x = x + iMinX;
        chunkOffset += X_1 * (x / CHUNK_WIDTH);
        x &= 0x1f;
    }
    start.x -= minX;
    end.x -= minX;
    if (minY < 0.0f) {
        y = (CHUNK_WIDTH - y) - iMinY;
        chunkOffset -= Y_1 * (y / CHUNK_WIDTH); // >> 5 = / 32
        y = CHUNK_WIDTH - (y & 0x1f);
    } else {
        y = y + iMinY;
        chunkOffset += Y_1 * (y / CHUNK_WIDTH);
        y &= 0x1f;
    }
    start.y -= minY;
    end.y -= minY;
    if (minZ < 0.0f) {
        z = (CHUNK_WIDTH - z) - iMinZ;
        chunkOffset -= Z_1 * (z / CHUNK_WIDTH); // >> 5 = / 32
        z = CHUNK_WIDTH - (z & 0x1f);
    } else {
        z = z + iMinZ;
        chunkOffset += Z_1 * (z / CHUNK_WIDTH);
        z &= 0x1f;
    }
    start.z -= minZ;
    end.z -= minZ;

    const f32v3 ray = (end - start);
    const f32 l2 = selfDot(ray);
    f32v3 vec(0.0f);
    // Iterate the volume and check points against line segment
    for (int i = 0; i < iMaxY - iMinY; i++) {
        for (int j = 0; j < iMaxZ - iMinZ; j++) {
            for (int k = 0; k < iMaxX - iMinX; k++) {
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
                    ui16 chunkOff = chunkOffset;
                    addChunkOffset(pos, chunkOff);
                    ui16 newIndex = (ui16)(pos.x + pos.y * CHUNK_LAYER + pos.z * CHUNK_WIDTH);
                    m_wNodes->emplace_back(props.barkBlockID, newIndex, chunkOff);
                } else if (dist2 < width2p1) {
                    // Outer edge, check for branching and fruit
                    if (rand() / (f32)RAND_MAX < props.branchChance) {
                        i32v3 pos(x + k, y + i, z + j);
                        ui16 chunkOff = chunkOffset;
                        addChunkOffset(pos, chunkOff);
                        ui16 newIndex = (ui16)(pos.x + pos.y * CHUNK_LAYER + pos.z * CHUNK_WIDTH);
                        m_wNodes->emplace_back(props.coreBlockID, newIndex, chunkOff);
                    }
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