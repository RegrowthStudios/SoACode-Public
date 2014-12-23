#include "stdafx.h"
#include "FloraGenerator.h"

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkUpdater.h"
#include "utils.h"
#include "WorldStructs.h"

KEG_ENUM_INIT_BEGIN(TreeLeafShape, TreeLeafShape, type)
type->addValue("unknown", TreeLeafShape::UNKNOWN);
type->addValue("round", TreeLeafShape::ROUND);
type->addValue("cluster", TreeLeafShape::CLUSTER);
type->addValue("pine", TreeLeafShape::PINE);
type->addValue("mushroom", TreeLeafShape::MUSHROOM);
KEG_ENUM_INIT_END

KEG_TYPE_INIT_BEGIN(TreeBranchingProps, TreeBranchingProps, type)
using namespace Keg;
type->addValue("width", Value::basic(BasicType::I32_V2, offsetof(TreeBranchingProps, width)));
type->addValue("length", Value::basic(BasicType::I32_V2, offsetof(TreeBranchingProps, length)));
type->addValue("chance", Value::basic(BasicType::F32_V2, offsetof(TreeBranchingProps, chance)));
type->addValue("direction", Value::basic(BasicType::I32, offsetof(TreeBranchingProps, direction)));
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN(TreeType, TreeType, type)
using namespace Keg;
type->addValue("name", Value::basic(BasicType::STRING, offsetof(TreeType, name)));
type->addValue("idCore", Value::basic(BasicType::I32, offsetof(TreeType, idCore)));
type->addValue("idLeaves", Value::basic(BasicType::I32, offsetof(TreeType, idLeaves)));
type->addValue("idBark", Value::basic(BasicType::I32, offsetof(TreeType, idOuter)));
type->addValue("idRoot", Value::basic(BasicType::I32, offsetof(TreeType, idRoot)));
type->addValue("idSpecialBlock", Value::basic(BasicType::I32, offsetof(TreeType, idSpecial)));

type->addValue("trunkHeight", Value::basic(BasicType::I32_V2, offsetof(TreeType, trunkHeight)));
type->addValue("trunkHeightBase", Value::basic(BasicType::I32_V2, offsetof(TreeType, trunkBaseHeight)));

type->addValue("trunkWidthBase", Value::basic(BasicType::I32_V2, offsetof(TreeType, trunkBaseWidth)));
type->addValue("trunkWidthMid", Value::basic(BasicType::I32_V2, offsetof(TreeType, trunkMidWidth)));
type->addValue("trunkWidthTop", Value::basic(BasicType::I32_V2, offsetof(TreeType, trunkTopWidth)));
type->addValue("trunkCoreWidth", Value::basic(BasicType::I32, offsetof(TreeType, coreWidth)));

type->addValue("trunkSlopeEnd", Value::basic(BasicType::I32_V2, offsetof(TreeType, trunkEndSlope)));
type->addValue("trunkSlopeStart", Value::basic(BasicType::I32_V2, offsetof(TreeType, trunkStartSlope)));

type->addValue("branchingPropsBottom", Value::custom("TreeBranchingProps", offsetof(TreeType, branchingPropsBottom)));
type->addValue("branchingPropsTop", Value::custom("TreeBranchingProps", offsetof(TreeType, branchingPropsTop)));

type->addValue("droopyLeavesLength", Value::basic(BasicType::I32_V2, offsetof(TreeType, droopyLength)));
type->addValue("leafCapSize", Value::basic(BasicType::I32_V2, offsetof(TreeType, leafCapSize)));

type->addValue("leafCapShape", Value::custom("TreeLeafShape", offsetof(TreeType, leafCapShape), true));
type->addValue("branchLeafShape", Value::custom("TreeLeafShape", offsetof(TreeType, branchLeafShape), true));

type->addValue("branchLeafSizeMod", Value::basic(BasicType::I32, offsetof(TreeType, branchLeafSizeMod)));
type->addValue("branchLeafYMod", Value::basic(BasicType::I32, offsetof(TreeType, branchLeafYMod)));

type->addValue("droopyLeavesSlope", Value::basic(BasicType::I32, offsetof(TreeType, droopyLeavesSlope)));
type->addValue("droopyLeavesDSlope", Value::basic(BasicType::I32, offsetof(TreeType, droopyLeavesDSlope)));

type->addValue("mushroomCapCurlLength", Value::basic(BasicType::I32, offsetof(TreeType, mushroomCapCurlLength)));
type->addValue("mushroomCapGillThickness", Value::basic(BasicType::I32, offsetof(TreeType, mushroomCapGillThickness)));
type->addValue("mushroomCapStretchMod", Value::basic(BasicType::I32, offsetof(TreeType, mushroomCapLengthMod)));
type->addValue("mushroomCapThickness", Value::basic(BasicType::I32, offsetof(TreeType, mushroomCapThickness)));

type->addValue("branchChanceCapMod", Value::basic(BasicType::F32, offsetof(TreeType, capBranchChanceMod)));
type->addValue("trunkChangeDirChance", Value::basic(BasicType::F32, offsetof(TreeType, trunkChangeDirChance)));
type->addValue("rootDepth", Value::basic(BasicType::F32, offsetof(TreeType, rootDepthMult)));
type->addValue("branchStart", Value::basic(BasicType::F32, offsetof(TreeType, branchStart)));

type->addValue("hasThickCapBranches", Value::basic(BasicType::BOOL, offsetof(TreeType, hasThickCapBranches)));
type->addValue("droopyLeavesActive", Value::basic(BasicType::BOOL, offsetof(TreeType, hasDroopyLeaves)));
type->addValue("mushroomCapInverted", Value::basic(BasicType::BOOL, offsetof(TreeType, isMushroomCapInverted)));
type->addValue("isSlopeRandom", Value::basic(BasicType::BOOL, offsetof(TreeType, isSlopeRandom)));
KEG_TYPE_INIT_END

bool FloraGenerator::generateTree(const TreeData& treeData, Chunk* startChunk) {
    _treeData = &treeData;

    if (!generateTrunk()) return false;

    return true;
}

bool FloraGenerator::generateTrunk() {

    const int& treeHeight = _treeData->treeHeight;
    int blockIndex = _treeData->startc;
    ui16 chunkOffset = 0x1CE7; // == 0001110011100111

    float heightRatio;
    int trunkSlope;

    for (int h = 0; h < treeHeight; h++) {
        // Calculate height ratio for interpolation
        heightRatio = h / (float)treeHeight;
        // Make this slice
        if (!makeTrunkSlice(blockIndex, chunkOffset, h, heightRatio)) return false;
       
        // Calculate trunk slope
        trunkSlope = lerp(_treeData->trunkStartSlope, _treeData->trunkEndSlope, heightRatio);

        // TODO(Ben): allow for inverted trunk for hanging plants
        // Move the block index up
        directionalMove(blockIndex, chunkOffset, TreeDir::TREE_UP);

        // Move block index sideways according to slope
        if (h % trunkSlope == trunkSlope - 1) {
            directionalMove(blockIndex, chunkOffset, (TreeDir)_treeData->trunkDir);
        }
    }
    return true;
}

int computeSide(int x, int z, int coreWidth) {
    // side:
    //  8 == all directions
    //  0  1  2
    //  3 -1  4
    //  5  6  7

    // if the core is only a single block, we can grow out in all directions
    if (coreWidth == 1) {
        return 8;
    }

    // Back face
    if (z == 0) {
        if (x == 0) {
            return 0;
        } else if (x == coreWidth - 1) {
            return 2;
        } else {
            return 1;
        }
    }

    // Front face
    if (z == (coreWidth - 1)) {
        if (x == 0) {
            return 5;
        } else if (x == (coreWidth - 1)) {
            return 7;
        } else {
            return 6;
        }
    }

    // Left face
    if (x == 0) {
        return 3;
    }

    // Right face
    if (x == (coreWidth - 1)) {
        return 4;
    }

    //Interior
    return -1;
}

bool FloraGenerator::makeTrunkSlice(int blockIndex, ui16 chunkOffset, int h, float heightRatio) {
    TreeType *treeType = _treeData->treeType;
    const int& coreWidth = treeType->coreWidth;
    int innerBlockIndex;
    ui16 innerChunkOffset;
    int leafBlock = treeType->idLeaves | (_treeData->leafColor << 12); //This could be in treeData
    float branchMod = 1.0f;
    if (coreWidth > 1) branchMod = 1.0f / ((coreWidth*coreWidth) - ((coreWidth - 2)*(coreWidth - 2)));

    float branchChance = lerp(_treeData->botBranchChance, _treeData->topBranchChance,
                              (float)(h - _treeData->branchStart) / (_treeData->treeHeight - _treeData->branchStart)) * branchMod;

    int branchWidth = lerp(_treeData->botBranchWidth, _treeData->topBranchWidth, heightRatio);
    int branchLength = lerp(_treeData->botBranchLength, _treeData->topBranchLength, heightRatio);

    // Calculate thickness of the outer layer
    int thickness;
    if (h <= _treeData->treeBaseHeight) {
        thickness = lerp(_treeData->trunkBaseWidth, _treeData->trunkMidWidth,
                         (float)h / _treeData->treeBaseHeight);
    } else {
        thickness = lerp(_treeData->trunkMidWidth, _treeData->trunkTopWidth,
                         (float)(h - _treeData->treeBaseHeight) / (_treeData->treeHeight - _treeData->treeBaseHeight));
    }

    for (int z = 0; z < coreWidth; z++) {
        innerBlockIndex = blockIndex;
        innerChunkOffset = chunkOffset;
        for (int x = 0; x < coreWidth; x++) {

            // Place core node
            _wnodes->emplace_back(innerBlockIndex, innerChunkOffset, treeType->idCore);

            // Check if we are on the outer edge of the core
            if (z == 0 || x == 0 || z == coreWidth - 1 || x == coreWidth - 1) {

                // side:
                //  8 == all directions
                //  0  1  2
                //  3 -1  4
                //  5  6  7
                if (thickness) {
                    // Special case for coreWidth == 1
                    if (!makeTrunkOuterRing(innerBlockIndex, innerChunkOffset,  x, z, coreWidth, thickness, treeType->idOuter, _wnodes)) return false;
                }

                // Check for roots and branches
                if (h == 0) { //roots
                    int dr = rand() % 4;
                    if (!recursiveMakeBranch(innerBlockIndex, innerChunkOffset, branchLength * treeType->rootDepthMult, (TreeDir)dr, (TreeDir)dr, MAX(branchWidth, thickness), true)) return false;
                } else if (h > treeType->branchStart) {
                    //branches
                    float r = rand() % RAND_MAX / ((float)RAND_MAX);
                    if (r <= branchChance) {
                        int dr = rand() % 4;
                        int bdir;
                        if (h < treeType->branchStart + (_treeData->treeHeight - treeType->branchStart) / 2) {
                            bdir = treeType->branchingPropsBottom.direction;
                        } else {
                            bdir = treeType->branchingPropsTop.direction;
                        }
                        if (bdir == 3) {
                            if (dr == _treeData->trunkDir) { //angle down
                                bdir = 2;
                            } else if ((dr + 2) % 4 == _treeData->trunkDir) { //angle up
                                bdir = 0;
                            } else {
                                bdir = 1;
                            }
                        }
                        if (!recursiveMakeBranch(innerBlockIndex, innerChunkOffset, branchLength, (TreeDir)dr, (TreeDir)dr, MAX(branchWidth, thickness), false)) return false;
                    }
                }

                // Check for cap leaves
                if (h == _treeData->treeHeight - 1) {
                    if (treeType->hasDroopyLeaves) {
                        if (!makeDroopyLeaves(innerBlockIndex, innerChunkOffset, _treeData->droopyLength, leafBlock, _wnodes)) return false;
                    }
                    switch (treeType->leafCapShape) {
                        case TreeLeafShape::ROUND:
                            if (!makeSphere(innerBlockIndex, innerChunkOffset, _treeData->topLeafSize, leafBlock, _lnodes)) return false;
                            break;
                        case TreeLeafShape::CLUSTER:
                            if (!makeCluster(innerBlockIndex, innerChunkOffset, _treeData->topLeafSize, leafBlock, _lnodes)) return false;
                            break;
                        case TreeLeafShape::MUSHROOM:
                            if (makeMushroomCap(innerBlockIndex, innerChunkOffset, leafBlock, _treeData->topLeafSize)) return false;
                            break;
                    }
                }

                // Pine tree leaves should be placed along trunk
                if (treeType->leafCapShape == TreeLeafShape::PINE) {
                    if (h >= _treeData->branchStart) {
                        int leafThickness = (int)(((_treeData->treeHeight - h) / 
                            (float)(_treeData->treeHeight - _treeData->branchStart))*_treeData->topLeafSize) - 
                            (h % 2) + thickness + coreWidth;
                        if (!makeTrunkOuterRing(innerBlockIndex, innerChunkOffset,  x, z, coreWidth, leafThickness, leafBlock, _lnodes)) return false;
                    } 
                }
              
            }

            // Move along X axis
            directionalMove(innerBlockIndex, innerChunkOffset, TREE_RIGHT);
        }
        // Move along Z axis
        directionalMove(blockIndex, chunkOffset, TREE_FRONT);
    }
}

bool FloraGenerator::makeTrunkOuterRing(int blockIndex, ui16 chunkOffset, int x, int z, int coreWidth, int thickness, int blockID, std::vector<TreeNode>* nodes) {
    TreeType* treeType = _treeData->treeType;
    if (coreWidth == 1) {
        if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
            TREE_LEFT, TREE_BACK, TREE_FRONT, false,
            treeType->idOuter, _wnodes)) return false;
        if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
            TREE_RIGHT, TREE_FRONT, TREE_BACK, false,
            treeType->idOuter, _wnodes)) return false;
        if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
            TREE_BACK, TREE_NO_DIR, TREE_NO_DIR, true,
            treeType->idOuter, _wnodes)) return false;
        if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
            TREE_FRONT, TREE_NO_DIR, TREE_NO_DIR, true,
            treeType->idOuter, _wnodes)) return false;
    } else {
        int side = computeSide(x, z, coreWidth);

        switch (side) {
            case 0:
                if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
                    TREE_LEFT, TREE_BACK, TREE_NO_DIR, false,
                    treeType->idOuter, _wnodes)) return false;
                break;
            case 1:
                if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
                    TREE_BACK, TREE_NO_DIR, TREE_NO_DIR, false,
                    treeType->idOuter, _wnodes)) return false;
                break;
            case 2:
                if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
                    TREE_RIGHT, TREE_NO_DIR, TREE_BACK, false,
                    treeType->idOuter, _wnodes)) return false;
                break;
            case 3:
                if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
                    TREE_LEFT, TREE_NO_DIR, TREE_NO_DIR, false,
                    treeType->idOuter, _wnodes)) return false;
                break;
            case 4:
                if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
                    TREE_RIGHT, TREE_NO_DIR, TREE_NO_DIR, false,
                    treeType->idOuter, _wnodes)) return false;
                break;
            case 5:
                if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
                    TREE_LEFT, TREE_FRONT, TREE_NO_DIR, false,
                    treeType->idOuter, _wnodes)) return false;
                break;
            case 6:
                if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
                    TREE_FRONT, TREE_NO_DIR, TREE_NO_DIR, false,
                    treeType->idOuter, _wnodes)) return false;
                break;
            case 7:
                if (!recursiveMakeSlice(blockIndex, chunkOffset, thickness,
                    TREE_RIGHT, TREE_FRONT, TREE_NO_DIR, false,
                    treeType->idOuter, _wnodes)) return false;
                break;
            default:
                break;
        }
    }
}

void FloraGenerator::directionalMove(int& blockIndex, ui16& chunkOffset, TreeDir dir) {
    // Constants for chunkOffset
    #define X_1 0x400
    #define Y_1 0x20
    #define Z_1 0x1
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

bool FloraGenerator::generateFlora(Chunk *chunk, std::vector<TreeNode>& wnodes, std::vector<TreeNode>& lnodes) {
    int c;
   
    std::vector <PlantData> &plantsToLoad = chunk->plantsToLoad;
    std::vector <TreeData> &treesToLoad = chunk->treesToLoad;

    _wnodes = &wnodes;
    _lnodes = &lnodes;

    // For placing flora
    Chunk* _lockedChunk = nullptr;

    //load plants
    for (int i = plantsToLoad.size() - 1; i >= 0; i--) {
        c = plantsToLoad[i].startc;
        Block &block = GETBLOCK(plantsToLoad[i].ft->baseBlock);
        bool occ = block.blockLight || block.lightColorPacked;

        if (c >= CHUNK_LAYER) {
            if (_lockedChunk) { _lockedChunk->unlock(); }
            _lockedChunk = chunk;
            _lockedChunk->lock();
            if (chunk->getBlockID(c - CHUNK_LAYER) != NONE) {
                ChunkUpdater::placeBlockNoUpdate(chunk, c, plantsToLoad[i].ft->baseBlock);
            }
        } else if (chunk->bottom && chunk->bottom->isAccessible) {
            if (_lockedChunk) { _lockedChunk->unlock(); }
            _lockedChunk = chunk->bottom;
            _lockedChunk->lock();
            if (chunk->bottom->getBlockID(c - CHUNK_LAYER + CHUNK_SIZE) != NONE) {
                _lockedChunk->unlock();
                _lockedChunk = chunk;
                _lockedChunk->lock();
                ChunkUpdater::placeBlockNoUpdate(chunk, c, plantsToLoad[i].ft->baseBlock);
            }
        } else {
            if (_lockedChunk) _lockedChunk->unlock();
            return false;
        }

        plantsToLoad.pop_back();
    }
    // Make sure to unlock
    if (_lockedChunk) _lockedChunk->unlock();
    //we don't want flora to set the dirty bit
    chunk->dirty = false;

    //Load Trees
    for (int i = treesToLoad.size() - 1; i >= 0; i--) {

        if (!generateTree(treesToLoad[i], chunk)) {
            if (_lockedChunk) _lockedChunk->unlock();
            return false;
        }

        treesToLoad.pop_back();
    }
    

    return true;
}

int FloraGenerator::makeLODTreeData(TreeData &td, TreeType *tt, int x, int z, int X, int Z) {
    srand(Z*X - x*z - globalTreeSeed);

    float mod = ((PseudoRand(globalTreeSeed + X*CHUNK_SIZE + z - X, Z*Z - x*z - globalTreeSeed) + 1.0) / 2.0);
    td.treeHeight = (int)(mod*(tt->trunkHeight.max - tt->trunkHeight.min) + tt->trunkHeight.min);
    td.topLeafSize = mod * (tt->leafCapSize.max - tt->leafCapSize.min) + tt->leafCapSize.min;
    td.treeType = tt;

    if (tt->possibleAltLeafFlags.size()) {
        int rnd = rand() % (tt->possibleAltLeafFlags.size() + 1);
        if (rnd == 0) {
            td.leafColor = 0;
        } else {
            td.leafColor = tt->possibleAltLeafFlags[rnd - 1];
        }
    } else {
        td.leafColor = 0;
    }

    return 0;
}

int FloraGenerator::makeTreeData(Chunk *chunk, TreeData &td, TreeType *tt) {
    int c = td.startc;
    int x = c%CHUNK_WIDTH;
    int z = (c%CHUNK_LAYER) / CHUNK_WIDTH;
    srand(chunk->gridPosition.z*chunk->gridPosition.x - x*z - globalTreeSeed);

    float mod = ((PseudoRand(globalTreeSeed + chunk->gridPosition.x*CHUNK_SIZE + z - chunk->gridPosition.x, chunk->gridPosition.z*chunk->gridPosition.z - x*z - globalTreeSeed) + 1.0) / 2.0);
    td.ageMod = mod;
    td.treeHeight = (int)(mod*(tt->trunkHeight.max - tt->trunkHeight.min) + tt->trunkHeight.min);
    td.droopyLength = mod * (tt->droopyLength.max - tt->droopyLength.min) + tt->droopyLength.min;
    td.branchStart = (int)(tt->branchStart*td.treeHeight);
    td.topLeafSize = mod * (tt->leafCapSize.max - tt->leafCapSize.min) + tt->leafCapSize.min;
    td.trunkDir = (int)((PseudoRand(chunk->gridPosition.x + z - x, x*z - z*z + x - chunk->gridPosition.z) + 1.0)*2.0);
    if (td.trunkDir == 4) td.trunkDir = 3;
    if (tt->isSlopeRandom == 0) {
        td.trunkStartSlope = mod*(tt->trunkStartSlope.max - tt->trunkStartSlope.min) + tt->trunkStartSlope.min;
        td.trunkEndSlope = mod*(tt->trunkEndSlope.max - tt->trunkEndSlope.min) + tt->trunkEndSlope.min;
    } else {
        float mod2 = (rand() % 101) / 100.0f;
        td.trunkStartSlope = mod2*(tt->trunkStartSlope.max - tt->trunkStartSlope.min) + tt->trunkStartSlope.min;
        mod2 = (rand() % 101) / 100.0f;
        td.trunkEndSlope = mod2*(tt->trunkEndSlope.max - tt->trunkEndSlope.min) + tt->trunkEndSlope.min;
    }
    td.treeBaseHeight = mod*(tt->trunkBaseHeight.max - tt->trunkBaseHeight.min) + tt->trunkBaseHeight.min;
    td.trunkBaseWidth = mod*(tt->trunkBaseWidth.max - tt->trunkBaseWidth.min) + tt->trunkBaseWidth.min;
    td.trunkMidWidth = mod*(tt->trunkMidWidth.max - tt->trunkMidWidth.min) + tt->trunkMidWidth.min;
    td.trunkTopWidth = mod*(tt->trunkTopWidth.max - tt->trunkTopWidth.min) + tt->trunkTopWidth.min;
    lerpBranch(tt->branchingPropsTop, tt->branchingPropsBottom, td, mod);
    td.treeType = tt;


    if (tt->possibleAltLeafFlags.size()) {
        int rnd = rand() % (tt->possibleAltLeafFlags.size() + 1);
        if (rnd == 0) {
            td.leafColor = 0;
        } else {
            td.leafColor = tt->possibleAltLeafFlags[rnd - 1];
        }
    } else {
        td.leafColor = 0;
    }

    return 0;
}

i32 FloraGenerator::getTreeIndex(Biome *biome, i32 x, i32 z) {
    float noTreeChance = 1.0f - biome->treeChance;
    float treeSum = 0.0f;
    int index = -1;

    for (Uint32 i = 0; i < biome->possibleTrees.size(); i++) {
        treeSum += biome->possibleTrees[i].probability; //precompute
    }

    float range = treeSum / biome->treeChance; //this gives us a an upperlimit for rand
    float cutOff = range * noTreeChance; //this is the no tree chance in our rand range
    float random = (PseudoRand(getPositionSeed(x, z)) + 1.0)*0.5*range; //get a random number for -1 to 1, and scales it to 0 to range

    if (random < cutOff) { //this happens most of the time, so we check this first to return early!
        return -1; //most of the time we finish here
    }

    for (Uint32 i = 0; i < biome->possibleTrees.size(); i++) {
        cutOff += biome->possibleTrees[i].probability;
        if (random < cutOff) {
            return biome->possibleTrees[i].treeIndex;
            break;
        }
    }
    return -1;
}

bool FloraGenerator::recursiveMakeSlice(int blockIndex, ui16 chunkOffset, i32 step, TreeDir dir, TreeDir rightDir, TreeDir leftDir, bool makeNode, i32 blockID, std::vector<TreeNode>* nodes) {

    while (step >= 0) {

        // Check for block placement
        if (makeNode) {
            nodes->emplace_back(blockIndex, chunkOffset, blockID);

            if (step == 0) {
                // TODO(Ben): This is bad
                if (PseudoRand(chunkOffset + blockIndex, -chunkOffset - blockIndex) > 0.85) nodes->pop_back();
            }
        } else {
            makeNode = true;
        }

        // Branching
        if (step) {
            // Right Direction
            if (rightDir != TreeDir::TREE_NO_DIR) {
                // Move over to the next block
                int nextBlockIndex = blockIndex;
                ui16 nextChunkOffset = chunkOffset;
                directionalMove(nextBlockIndex, nextChunkOffset, rightDir);
                // Recursive call
                if (!recursiveMakeSlice(nextBlockIndex, nextChunkOffset, step - 1, rightDir,
                    TREE_NO_DIR, TREE_NO_DIR, true, blockID, nodes)) return false;
            }
            // Left Direction
            if (leftDir != TreeDir::TREE_NO_DIR) {
                // Move over to the next block
                int nextBlockIndex = blockIndex;
                ui16 nextChunkOffset = chunkOffset;
                directionalMove(nextBlockIndex, nextChunkOffset, leftDir);
                // Recursive call
                if (!recursiveMakeSlice(nextBlockIndex, nextChunkOffset, step - 1, leftDir,
                    TREE_NO_DIR, TREE_NO_DIR, true, blockID, nodes)) return false;
            }
        }
        // Move in direction
        directionalMove(blockIndex, chunkOffset, dir);
        // Check end condition
        step--;
    }
    return true;
}

bool FloraGenerator::makeCluster(int blockIndex, ui16 chunkOffset, int size, int blockID, std::vector<TreeNode>* nodes) {
    
    int innerBlockIndex = blockIndex;
    ui16 innerChunkOffset = chunkOffset;

    // Center and up
    for (int i = size; i > 0; i--) {
        if (!recursiveMakeSlice(innerBlockIndex, innerChunkOffset, i,
                           TREE_LEFT, TREE_BACK, TREE_FRONT, true,
                           blockID, nodes)) return false;
        if (!recursiveMakeSlice(innerBlockIndex, innerChunkOffset, i,
                           TREE_RIGHT, TREE_FRONT, TREE_BACK, false,
                           blockID, nodes)) return false;
        directionalMove(innerBlockIndex, innerChunkOffset, TREE_UP);
    }

    innerBlockIndex = blockIndex;
    innerChunkOffset = chunkOffset;

    directionalMove(innerBlockIndex, innerChunkOffset, TREE_DOWN);

    // Down
    for (int i = size-1; i > 0; i--) {
        if (!recursiveMakeSlice(innerBlockIndex, innerChunkOffset, i,
                           TREE_LEFT, TREE_BACK, TREE_FRONT, true,
                           blockID, nodes)) return false;
        if (!recursiveMakeSlice(innerBlockIndex, innerChunkOffset, i,
                            TREE_RIGHT, TREE_FRONT, TREE_BACK, false,
                            blockID, nodes)) return false;
        directionalMove(innerBlockIndex, innerChunkOffset, TREE_DOWN);
    }
    return true;
}

// TODO(Ben): refactor this
int FloraGenerator::makeMushroomLeaves(int c, ui16 chunkOffset, int dir, bool branch, bool makeNode, int ntype, int lamntype, int dx, int dy, int dz, int rad, TreeType *tt) {
    int code;
    int dist;

    if (branch) {
        code = makeMushroomLeaves(c, chunkOffset, (dir + 3) % 4, 0, 0, ntype, lamntype, dx, dy, dz, rad, tt);
        if (code) return code;
        code = makeMushroomLeaves(c, chunkOffset, (dir + 1) % 4, 0, 0, ntype, lamntype, dx, dy, dz, rad, tt);
        if (code) return code;
    }

    if (dir == 0) {
        dx--;
    } else if (dir == 1) {
        dz--;
    } else if (dir == 2) {
        dx++;
    } else {
        dz++;
    }

    dist = (int)(sqrt((double)(dx*dx + dy*dy + dz*dz)));

    if (dist > rad - tt->mushroomCapThickness) {
        if (makeNode) {
            _lnodes->emplace_back(c, chunkOffset, ntype);
        }
    } else if (dist > rad - (tt->mushroomCapThickness + tt->mushroomCapGillThickness)) {
        if (makeNode) {
            _lnodes->emplace_back(c, chunkOffset, lamntype);
        }
    }

    if (dist >= rad) {
      //  if (PseudoRand(chunk->gridPosition.x + c - chunk->gridPosition.z, chunk->gridPosition.z + c) > 0.8 && _lnodes.size()) _lnodes.pop_back();
        return 0;
    }

    if (dir == 0) //left
    {
        if (c % CHUNK_WIDTH) {
            return makeMushroomLeaves(c - 1, chunkOffset, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        } else {
            return makeMushroomLeaves(c + CHUNK_WIDTH - 1, chunkOffset, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        }
    } else if (dir == 1) //back
    {
        if ((c % CHUNK_LAYER) - CHUNK_WIDTH >= 0) {
            return makeMushroomLeaves(c - CHUNK_WIDTH, chunkOffset, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        } else {
            return makeMushroomLeaves(c + CHUNK_LAYER - CHUNK_WIDTH, chunkOffset, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        }
    } else if (dir == 2) //right
    {
        if ((c % CHUNK_WIDTH) < CHUNK_WIDTH - 1) {
            return makeMushroomLeaves(c + 1, chunkOffset, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        } else {
            return makeMushroomLeaves(c - CHUNK_WIDTH + 1, chunkOffset, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        }
    } else if (dir == 3) //front
    {
        if ((c % CHUNK_LAYER) + CHUNK_WIDTH < CHUNK_LAYER) {
            return makeMushroomLeaves(c + CHUNK_WIDTH, chunkOffset, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        } else {
            return makeMushroomLeaves(c - CHUNK_LAYER + CHUNK_WIDTH, chunkOffset, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        }
    }


    return 1;
}

bool FloraGenerator::makeDroopyLeaves(int blockIndex, ui16 chunkOffset, int length, int blockID, std::vector<TreeNode>* nodes) {
    // Place first node
    nodes->emplace_back(blockIndex, chunkOffset, blockID);

    int slope = _treeData->treeType->droopyLeavesSlope;
    int dslope = _treeData->treeType->droopyLeavesDSlope;
    if (slope == 0) slope = -1;

    // Loop through the 4 cardinal directions
    for (int dir = 0; dir < 4; dir++) {
        int innerBlockIndex = blockIndex;
        ui16 innerChunkOffset = chunkOffset;
       
        int slopestep = 0;
        int step = 0;
        for (int i = 0; i < length; i++) {
            bool side = false;

            if (slope > 0) { //shallow slope
                side = true;
                if (step >= slope) {
                    step = -1;
                    slopestep++;
                }
            } else { //steep slope
                if (step >= -slope && slope > -3) {
                    step = -1;
                    slopestep++;
                    side = true;
                }
            }

            // Move sideways
            if (side) {
                directionalMove(innerBlockIndex, innerChunkOffset, (TreeDir)dir);
            } else { // Move downward
                directionalMove(innerBlockIndex, innerChunkOffset, TREE_DOWN);
            }

            // For changing calculation
            if (slopestep >= dslope) {
                slope--;
                if (slope == 0) slope = -1;
                slopestep = 0;
            }

            // Place node
            nodes->emplace_back(innerBlockIndex, innerChunkOffset, blockID);

            step++;
        }
    }
    return true;
}

// TODO(Ben): Refactor this
int FloraGenerator::makeMushroomCap(int c, ui16 chunkOffset, int block, int rad) {
    int code;
    int step;
    TreeType* treeType = _treeData->treeType;
    int k = treeType->mushroomCapLengthMod + 1;
    int size;

    if (treeType->mushroomCapLengthMod >= 0) {
        size = rad + rad*treeType->mushroomCapLengthMod;
    } else {
        size = rad / 2;
    }

    step = 0;
    for (int i = 0; i < size; i++) {
        if (treeType->isMushroomCapInverted == 1) {
            directionalMove(c, chunkOffset, TREE_UP);
        } else {
            directionalMove(c, chunkOffset, TREE_DOWN);
        }


        if (k == treeType->mushroomCapLengthMod + 1) {
            k = 0;
            step++;
        } else if (treeType->mushroomCapLengthMod < 0) {
            k = 0;
            step += 2;
        }

        code = makeMushroomLeaves(c, chunkOffset, 0, 1, 1, block, treeType->idSpecial, 0, rad - step, 0, rad, treeType);
        if (code) return code;
        code = makeMushroomLeaves(c, chunkOffset, 2, 1, 0, block, treeType->idSpecial, 0, rad - step, 0, rad, treeType);
        if (code) return code;
        k++;
    }
    if (treeType->isMushroomCapInverted == 0) { //round bottom portion
        while (step > rad - treeType->mushroomCapCurlLength) {
            step -= 2;

            directionalMove(c, chunkOffset, TREE_DOWN);

            code = makeMushroomLeaves(c, chunkOffset, 0, 1, 1, block, treeType->idSpecial, 0, rad - step, 0, rad, treeType);
            if (code) return code;
            code = makeMushroomLeaves(c, chunkOffset, 2, 1, 0, block, treeType->idSpecial, 0, rad - step, 0, rad, treeType);
            if (code) return code;
        }
    }

    return 0;
}

bool FloraGenerator::recursiveMakeBranch(int blockIndex, ui16 chunkOffset, int step, TreeDir dir, TreeDir initDir, int thickness, bool isRoot) {
    TreeType* treeType = _treeData->treeType;
    int leafBlock = treeType->idLeaves | (_treeData->leafColor << 12);
    int woodBlock = treeType->idOuter;
    if (isRoot) woodBlock = treeType->idRoot;
    int startStep = step;

    // How far the thickness attenuates to at the end of the branch
    const float thicknessAttenuation = 0.5f;
    
    // Move once in dir without placing anything
    directionalMove(blockIndex, chunkOffset, dir);
    step--;

    // Hard coded directions for recursiveMakeSlice
    // 3 axis, 4 dirs
    static const TreeDir TREE_DIRS[3][4] = {
        {TREE_BACK, TREE_UP, TREE_FRONT, TREE_DOWN}, // X axis
        {TREE_BACK, TREE_RIGHT, TREE_FRONT, TREE_LEFT}, // Y axis
        {TREE_LEFT, TREE_UP, TREE_RIGHT, TREE_DOWN}  // Z axis
    };

    // Pick treeDirs based on axis
    const TreeDir* treeDirs;
    switch (dir) {
        default:
        case TREE_LEFT:
        case TREE_RIGHT:
            treeDirs = TREE_DIRS[0];
            break;
        case TREE_BACK:
        case TREE_FRONT:
            treeDirs = TREE_DIRS[1];
            break;
        case TREE_UP:
        case TREE_DOWN:
            treeDirs = TREE_DIRS[2];
            break;
    }
    
    while (step >= 0) {
        // Place the block node
        _wnodes->emplace_back(blockIndex, chunkOffset, woodBlock);
        // Get thickness with linear interpolation based on length
        int newThickness = lerp(0, thickness, ((float)step / startStep) * (1.0 - thicknessAttenuation) + thicknessAttenuation);
        
        // Thickness using recursiveMakeSlice
        if (newThickness) {
            if (!recursiveMakeSlice(blockIndex, chunkOffset, newThickness,
                               treeDirs[3], treeDirs[0], treeDirs[2], false,
                               treeType->idOuter, _wnodes)) return false;
            if (!recursiveMakeSlice(blockIndex, chunkOffset, newThickness,
                               treeDirs[1], treeDirs[2], treeDirs[0], false,
                               treeType->idOuter, _wnodes)) return false;
            if (!recursiveMakeSlice(blockIndex, chunkOffset, newThickness,
                               treeDirs[0], TREE_NO_DIR, TREE_NO_DIR, true,
                               treeType->idOuter, _wnodes)) return false;
            if (!recursiveMakeSlice(blockIndex, chunkOffset, newThickness,
                               treeDirs[2], TREE_NO_DIR, TREE_NO_DIR, true,
                               treeType->idOuter, _wnodes)) return false;
        }

        // Leaves
        if (step == 0 && !isRoot) {
            if (treeType->hasDroopyLeaves) {
                if (!makeDroopyLeaves(blockIndex, chunkOffset, _treeData->droopyLength, leafBlock, _wnodes)) return false;
            }

            if (treeType->branchLeafShape == TreeLeafShape::ROUND) {
                if (!makeSphere(blockIndex, chunkOffset, newThickness + 3 + treeType->branchLeafSizeMod, leafBlock, _lnodes)) return false;
            } else if (treeType->branchLeafShape == TreeLeafShape::CLUSTER) {
                if (!makeCluster(blockIndex, chunkOffset, newThickness + 3 + treeType->branchLeafSizeMod, leafBlock, _lnodes)) return false;
            }
        }

        // Get random move direction. 1/3 chance left, 1/3 chance right, 1/3 up or down
        // TODO(Ben): This is bad
        int r = (int)((PseudoRand(chunkOffset*blockIndex + blockIndex + step, blockIndex*blockIndex - chunkOffset + step) + 1)*1.999); //between 0 and 2
        // If r == 1 then we are up/down so we should randomly flip direction
        if (r == 1) {
            if (isRoot) {
                r = 3;
            }
        }

        // Random branching chance
        // TODO(Ben): This is bad
        if (newThickness && rand() % 1000 < 200 - newThickness * 4) {
            int dr = rand() % 4;
            if (dr == r) dr = (dr + 1) % 4;
            if (dr == (initDir + 2) % 4) dr = (dr + 1) % 4; //must branch orthogonally
            if (!(recursiveMakeBranch(blockIndex, chunkOffset, step - 1, treeDirs[r], initDir, newThickness, isRoot))) return false;
        }

        // Move the block
        directionalMove(blockIndex, chunkOffset, treeDirs[r]);

        step--;
    }
    return true;
}

bool FloraGenerator::makeSphere(int blockIndex, ui16 chunkOffset, int radius, int blockID, std::vector<TreeNode>* nodes) {
    // Shift over to the back, bottom, left hand corner
    for (int i = 0; i < radius; i++) {
        directionalMove(blockIndex, chunkOffset, TREE_LEFT);
        directionalMove(blockIndex, chunkOffset, TREE_BACK);
        directionalMove(blockIndex, chunkOffset, TREE_DOWN);
    }

    const int radius2 = radius * radius;
    const int width = radius * 2 + 1;
    const int center = radius;

    int distance2;
    int dx, dy, dz;

    int zBlockIndex, xBlockIndex;
    ui16 zChunkOffset, xChunkOffset;

    // Loop through the bounding volume of the sphere
    for (int y = 0; y < width; y++) {
        dy = y - center;
        dy *= dy;
        zBlockIndex = blockIndex;
        zChunkOffset = chunkOffset;
        for (int z = 0; z < width; z++) {
            dz = z - center;
            dz *= dz;
            xBlockIndex = zBlockIndex;
            xChunkOffset = chunkOffset;
            for (int x = 0; x < width; x++) {
                dx = x - center;
                dx *= dx;
                distance2 = dx + dy + dz;
                // Check the distance for placing a node
                if (distance2 <= radius2) {
                    nodes->emplace_back(xBlockIndex, xChunkOffset, blockID);
                }
                // Move right
                directionalMove(xBlockIndex, xChunkOffset, TREE_RIGHT);
            }
            // Move front
            directionalMove(zBlockIndex, zChunkOffset, TREE_FRONT);
        }
        // Move up
        directionalMove(blockIndex, chunkOffset, TREE_UP);
    }
    return true;
}

void lerpBranch(const TreeBranchingProps& top, const TreeBranchingProps& bottom, TreeData& outProps, const f32& ratio) {
    outProps.botBranchChance = ratio * (bottom.chance.max - bottom.chance.min) + bottom.chance.min;
    outProps.topBranchChance = ratio * (top.chance.max - top.chance.min) + top.chance.min;
    outProps.botBranchLength = ratio * (bottom.length.max - bottom.length.min) + bottom.length.min;
    outProps.topBranchLength = ratio * (top.length.max - top.length.min) + top.length.min;
    outProps.botBranchWidth = ratio * (bottom.width.max - bottom.width.min) + bottom.width.min;
    outProps.topBranchWidth = ratio * (top.width.max - top.width.min) + top.width.min;
}
