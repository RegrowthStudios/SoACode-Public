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

vector <TreeNode> FloraGenerator::wnodes;
vector <TreeNode> FloraGenerator::lnodes;

int FloraGenerator::generateFlora(Chunk *chunk) {
    int c;
    int xz, y;
    wnodes.clear();
    lnodes.clear();
    vector <PlantData> &plantsToLoad = chunk->plantsToLoad;
    vector <TreeData> &treesToLoad = chunk->treesToLoad;

    //load plants
    for (int i = plantsToLoad.size() - 1; i >= 0; i--) {
        c = plantsToLoad[i].startc;
        Block &block = GETBLOCK(plantsToLoad[i].ft->baseBlock);
        bool occ = block.blockLight || block.isLight;

        if (c >= CHUNK_LAYER) {
            if (chunk->getBlockID(c - CHUNK_LAYER) != (ui16)Blocks::NONE) {
                ChunkUpdater::placeBlock(chunk, c, plantsToLoad[i].ft->baseBlock);
            }
        } else if (chunk->bottom && chunk->bottom->isAccessible) {
            if (chunk->bottom->getBlockID(c - CHUNK_LAYER + CHUNK_SIZE) != (ui16)Blocks::NONE) {
                ChunkUpdater::placeBlock(chunk->bottom, c, plantsToLoad[i].ft->baseBlock);
            }
        } else {
            return 0;
        }

        plantsToLoad.pop_back();
    }
    //we dont want flora to set the dirty bit
    chunk->dirty = false;

    //Load Trees
    for (int i = treesToLoad.size() - 1; i >= 0; i--) {
        c = treesToLoad[i].startc;

        if (generateTreeNodes(chunk, c, treesToLoad[i]) != 0) return 0;

        placeTreeNodes();

        treesToLoad.pop_back();
        wnodes.clear();
        lnodes.clear();
    }

    chunk->dirty = false;
    return 1;
}

int FloraGenerator::generateTreeNodes(Chunk *chunk, int sc, TreeData &treeData) {
    TreeType *tt = treeData.treeType;
    int code;
    int c = sc;
    int c1;
    int c2;
    int leafBlock = tt->idLeaves;
    SETFLAGS(leafBlock, treeData.leafColor);

    int leafStep;
    int treeHeight = treeData.treeHeight;
    int leafRadius = treeData.topLeafSize;
    int coreWidth = tt->coreWidth;
    int thickness;
    int branchStart = treeData.branchStart;
    int branchLength;
    int trunkSlope;
    int trunkDir = treeData.trunkDir;
    int slopeCounter = 0;
    bool makeWood;
    Chunk *ch1, *ch2;
    Chunk *ch = chunk;
    float r;
    int branchWidth;
    float branchChance;
    float branchMod = 1.0f;
    float heightRatio;
    if (coreWidth > 1) branchMod = 1.0f / ((coreWidth*coreWidth) - ((coreWidth - 2)*(coreWidth - 2))); //divide the chance by how many chances we get per layer
    //cout << treeHeight << " " << leafRadius << " " << coreWidth << " " << branchStart << " " << branchMod << endl;
    int trunkStep = 0;

    while (trunkStep <= treeHeight) {
        makeWood = 0;
        c1 = c;
        ch1 = ch;
        if (trunkStep <= treeData.treeBaseHeight) {
            thickness = (int)(trunkStep / ((float)(treeData.treeBaseHeight)) * (treeData.trunkMidWidth - treeData.trunkBaseWidth) + treeData.trunkBaseWidth);
        } else {
            thickness = (int)((trunkStep - treeData.treeBaseHeight) / ((float)(treeHeight - treeData.treeBaseHeight)) * (treeData.trunkTopWidth - treeData.trunkMidWidth) + treeData.trunkMidWidth);
        }
        branchChance = ((trunkStep - branchStart) / ((float)treeHeight - branchStart) * (treeData.topBranchChance - treeData.botBranchChance) + treeData.botBranchChance) * branchMod;
        heightRatio = trunkStep / ((float)(treeHeight));
        branchLength = (int)(heightRatio * (treeData.topBranchLength - treeData.botBranchLength) + treeData.botBranchLength);
        branchWidth = (int)(heightRatio * (treeData.topBranchWidth - treeData.botBranchWidth) + treeData.botBranchWidth);
        trunkSlope = (int)(heightRatio * (treeData.trunkEndSlope - treeData.trunkStartSlope) + treeData.trunkStartSlope);
        for (int i = 0; i < coreWidth; i++) { //z
            c2 = c1;
            ch2 = ch1;
            for (int j = 0; j < coreWidth; j++) { //x
                if (tt->leafCapShape != TreeLeafShape::UNKNOWN && trunkStep == treeHeight - leafRadius - 1) {
                    branchChance += tt->capBranchChanceMod; //we want two branches on average
                    if (tt->hasThickCapBranches && branchWidth < coreWidth + thickness) branchWidth = coreWidth + thickness;
                }
                if ((tt->leafCapShape != TreeLeafShape::ROUND || trunkStep < treeHeight - leafRadius && trunkStep < treeHeight) && !(tt->leafCapShape == TreeLeafShape::MUSHROOM && (!tt->isMushroomCapInverted && trunkStep >= treeHeight - 1)) && !(tt->leafCapShape == TreeLeafShape::PINE && trunkStep >= treeHeight - 1)) {
                    makeWood = 1;
                    wnodes.push_back(TreeNode(c2, tt->idCore, ch2));

                    //thickness
                    if (i == 0 || j == 0 || i == coreWidth - 1 || j == coreWidth - 1) {
                        if (thickness > 1) {
                            code = makeLeaves(ch2, thickness, c2, 0, 1, thickness, 1, tt->idOuter);
                            if (code) return code;
                            code = makeLeaves(ch2, thickness, c2, 2, 1, thickness, 1, tt->idOuter);
                            if (code) return code;
                        } else if (thickness == 1) {
                            code = makeLeaves(ch2, 1, c2, 0, 1, 0, 1, tt->idOuter);
                            if (code) return code;
                            code = makeLeaves(ch2, 1, c2, 1, 1, 0, 1, tt->idOuter);
                            if (code) return code;
                            code = makeLeaves(ch2, 1, c2, 2, 1, 0, 1, tt->idOuter);
                            if (code) return code;
                            code = makeLeaves(ch2, 1, c2, 3, 1, 0, 1, tt->idOuter);
                            if (code) return code;
                        }

                        if (trunkStep == 0) { //roots
                            int dr = rand() % 4;
                            code = makeBranch(ch2, branchLength*tt->rootDepthMult, branchLength*tt->rootDepthMult, c2, 4, 0, &treeData, dr, MAX(branchWidth, thickness), 0, 1, 2);
                            if (code) return code;
                        } else if (trunkStep > branchStart) {
                            //branches
                            r = rand() % RAND_MAX / ((float)RAND_MAX);
                            if (r <= branchChance) {
                                int dr = rand() % 4;
                                int bdir;
                                if (trunkStep < branchStart + (treeHeight - branchStart) / 2) {
                                    bdir = tt->branchingPropsBottom.direction;
                                } else {
                                    bdir = tt->branchingPropsTop.direction;
                                }
                                if (bdir == 3) {
                                    if (dr == trunkDir) { //angle down
                                        bdir = 2;
                                    } else if ((dr + 2) % 4 == trunkDir) { //angle up
                                        bdir = 0;
                                    } else {
                                        bdir = 1;
                                    }
                                }

                                code = makeBranch(ch2, branchLength, branchLength, c2, dr, 0, &treeData, dr, branchWidth, treeData.treeType->branchLeafSizeMod, 0, bdir);

                                if (code) return code;
                            }
                        }
                    }
                }
                if (trunkStep == treeHeight && tt->hasDroopyLeaves) {
                    code = makeDroopyLeaves(ch2, treeData.droopyLength, c2, &treeData, leafBlock);
                    if (code) return code;
                }
                if (tt->leafCapShape == TreeLeafShape::ROUND && trunkStep > treeHeight - leafRadius * 2 - 1) //round leaves
                {
                    if (trunkStep == treeHeight - leafRadius) {
                        leafStep = leafRadius;
                    } else if (trunkStep < treeHeight - leafRadius) {
                        leafStep = trunkStep - (treeHeight - leafRadius * 2 - 1);
                    } else {
                        leafStep = treeHeight - trunkStep + 1;
                    }

                    code = makeRoundLeaves(ch2, c2, 0, 1, 1, leafBlock, 0, leafRadius - leafStep, 0, leafRadius);
                    if (code) return code;
                    code = makeRoundLeaves(ch2, c2, 2, 1, 0, leafBlock, 0, leafRadius - leafStep, 0, leafRadius);
                    if (code) return code;
                } else if (tt->leafCapShape == TreeLeafShape::CLUSTER && trunkStep == treeHeight) { //cluster leaves
                    code = makeCluster(ch2, leafRadius, c2, leafBlock);
                    if (code) return code;
                } else if (tt->leafCapShape == TreeLeafShape::PINE) { //pine leaves
                    if (trunkStep >= branchStart) //leaves
                    {
                        leafStep = (int)(((treeHeight - trunkStep) / (float)(treeHeight - branchStart))*leafRadius) - (trunkStep % 2) + thickness + coreWidth;
                        code = makeLeaves(ch2, leafStep, c2, 0, 1, leafStep, 1, leafBlock);
                        if (code) return code;
                        code = makeLeaves(ch2, leafStep, c2, 2, 1, leafStep, 0, leafBlock);
                        if (code) return code;
                    } else if (trunkStep == treeHeight) { //make the cap
                        code = makeLeaves(ch2, leafStep, c2, 0, 1, leafStep, 1, leafBlock);
                        if (code) return code;
                        code = makeLeaves(ch2, leafStep, c2, 2, 1, leafStep, 0, leafBlock);
                        if (code) return code;
                        leafStep -= 2;
                        if (leafStep < 0) leafStep = 0;
                        if (c2 / CHUNK_LAYER < CHUNK_WIDTH - 1) {
                            code = makeLeaves(ch2, leafStep, c2 + CHUNK_LAYER, 0, 1, leafStep, 1, leafBlock);
                            if (code) return code;
                            code = makeLeaves(ch2, leafStep, c2 + CHUNK_LAYER, 2, 1, leafStep, 0, leafBlock);
                            if (code) return code;
                        } else if (ch2->top && ch2->top->isAccessible) {
                            code = makeLeaves(ch2->top, leafStep, c2 + CHUNK_LAYER - CHUNK_SIZE, 0, 1, leafStep, 1, leafBlock);
                            if (code) return code;
                            code = makeLeaves(ch2->top, leafStep, c2 + CHUNK_LAYER - CHUNK_SIZE, 2, 1, leafStep, 0, leafBlock);
                            if (code) return code;
                        } else {
                            return 1;
                        }
                    }
                } else if (tt->leafCapShape == TreeLeafShape::MUSHROOM && trunkStep == treeHeight) { //mushroom shape cap
                    code = makeMushroomCap(ch2, c2, treeData, leafBlock, leafRadius);
                    if (code) return code;
                }


                //move c2 to the right
                c2++;
                if (j != coreWidth - 1) {
                    if (c2%CHUNK_WIDTH == 0) {
                        if (ch2->right && ch2->right->isAccessible) {
                            ch2 = ch2->right;
                            c2 -= CHUNK_WIDTH;
                        } else {
                            return 1;
                        }
                    }
                }
            }

            //move c1 forward
            c1 += CHUNK_WIDTH;
            if (i != coreWidth - 1) {
                if ((c1%CHUNK_LAYER) / CHUNK_WIDTH == 0) {
                    if (ch1->front && ch1->front->isAccessible) {
                        ch1 = ch1->front;
                        c1 -= CHUNK_LAYER;
                    } else {
                        return 1;
                    }
                }
            }
        }

        trunkStep++;
        slopeCounter++;
        if (slopeCounter >= trunkSlope) {
            slopeCounter -= trunkSlope;
            switch (trunkDir) {
            case 0: //left
                if (c%CHUNK_WIDTH) {
                    c--;
                } else if (ch->left && ch->left->isAccessible) {
                    c = c + CHUNK_WIDTH - 1;
                    ch = ch->left;
                } else {
                    return 1;
                }
                break;
            case 1:   //back
                if ((c%CHUNK_LAYER) / CHUNK_WIDTH) {
                    c -= CHUNK_WIDTH;
                } else if (ch->back && ch->back->isAccessible) {
                    c = c + CHUNK_LAYER - CHUNK_WIDTH;
                    ch = ch->back;
                } else {
                    return 1;
                }
                break;
            case 2:  //right
                if (c%CHUNK_WIDTH < CHUNK_WIDTH - 1) {
                    c++;
                } else if (ch->right && ch->right->isAccessible) {
                    c = c - CHUNK_WIDTH + 1;
                    ch = ch->right;
                } else {
                    return 1;
                }
                break;
            case 3:  //front
                if ((c%CHUNK_LAYER) / CHUNK_WIDTH < CHUNK_WIDTH - 1) {
                    c += CHUNK_WIDTH;
                } else if (ch->front && ch->front->isAccessible) {
                    c = c - CHUNK_LAYER + CHUNK_WIDTH;
                    ch = ch->front;
                } else {
                    return 1;
                }
                break;
            }
            if (makeWood) {
                wnodes.push_back(TreeNode(c, tt->idCore, ch));
            }
        }

        c += CHUNK_LAYER;
        if (c >= CHUNK_SIZE) {
            if (ch->top && ch->top->isAccessible) {
                ch = ch->top;
                c -= CHUNK_SIZE;
            } else {
                return 1;
            }
        }

        if (tt->trunkChangeDirChance > 0.0f) {
            float rnd = (rand() % RAND_MAX) / ((float)RAND_MAX);
            if (rnd >= tt->trunkChangeDirChance) {
                trunkDir = (int)((PseudoRand(ch->position.x - c*c + c, c*c - c * 3 - ch->position.z) + 1.0)*2.0);
                if (trunkDir == 4) trunkDir = 3;
            }
        }
    }
    return 0;
}

void FloraGenerator::placeTreeNodes() {
    int c;
    int xz;
    int y;
    Chunk *owner;

    for (Uint32 j = 0; j < wnodes.size(); j++) { //wood nodes
        c = wnodes[j].c;
        owner = wnodes[j].owner;
        ChunkUpdater::placeBlock(owner, c, wnodes[j].blockType);

        if (c >= CHUNK_LAYER) {
            if (owner->getBlockID(c - CHUNK_LAYER) == (ui16)Blocks::DIRTGRASS) owner->setBlockData(c - CHUNK_LAYER, (ui16)Blocks::DIRT); //replace grass with dirt
        } else if (owner->bottom && owner->bottom->isAccessible) {
            if (owner->bottom->getBlockID(c + CHUNK_SIZE - CHUNK_LAYER) == (ui16)Blocks::DIRTGRASS) owner->bottom->setBlockData(c + CHUNK_SIZE - CHUNK_LAYER, (ui16)Blocks::DIRT);
        }
    }

    for (Uint32 j = 0; j < lnodes.size(); j++) { //leaf nodes
        c = lnodes[j].c;
        owner = lnodes[j].owner;
        if (owner->getBlockID(c) == (ui16)Blocks::NONE) {
            ChunkUpdater::placeBlock(owner, c, lnodes[j].blockType);
        }
    }
}

void FloraGenerator::placeTreeNodesAndRecord() {
    int c;
    int xz;
    int y;
    int tmp;
    Chunk *owner;

    for (Uint32 j = 0; j < wnodes.size(); j++) { //wood nodes
        c = wnodes[j].c;
        owner = wnodes[j].owner;
        tmp = owner->getBlockData(c);
        ChunkUpdater::placeBlock(owner, c, wnodes[j].blockType);
        wnodes[j].blockType = tmp;

        if (c >= CHUNK_LAYER) {
            if (owner->getBlockID(c - CHUNK_LAYER) == (ui16)Blocks::DIRTGRASS) owner->setBlockData(c - CHUNK_LAYER, (ui16)Blocks::DIRT); //replace grass with dirt
        } else if (owner->bottom && owner->bottom->isAccessible) {
            if (owner->bottom->getBlockID(c + CHUNK_SIZE - CHUNK_LAYER) == (ui16)Blocks::DIRTGRASS) owner->bottom->setBlockData(c + CHUNK_SIZE - CHUNK_LAYER, (ui16)Blocks::DIRT);
        }
    }

    for (Uint32 j = 0; j < lnodes.size(); j++) { //leaf nodes
        c = lnodes[j].c;
        owner = lnodes[j].owner;
        tmp = owner->getBlockData(c);

        if (tmp == (ui16)Blocks::NONE) {
            ChunkUpdater::placeBlock(owner, c, lnodes[j].blockType);
        }
        lnodes[j].blockType = tmp;
    }
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
    srand(chunk->position.z*chunk->position.x - x*z - globalTreeSeed);

    float mod = ((PseudoRand(globalTreeSeed + chunk->position.x*CHUNK_SIZE + z - chunk->position.x, chunk->position.z*chunk->position.z - x*z - globalTreeSeed) + 1.0) / 2.0);
    td.ageMod = mod;
    td.treeHeight = (int)(mod*(tt->trunkHeight.max - tt->trunkHeight.min) + tt->trunkHeight.min);
    td.droopyLength = mod * (tt->droopyLength.max - tt->droopyLength.min) + tt->droopyLength.min;
    td.branchStart = (int)(tt->branchStart*td.treeHeight);
    td.topLeafSize = mod * (tt->leafCapSize.max - tt->leafCapSize.min) + tt->leafCapSize.min;
    td.trunkDir = (int)((PseudoRand(chunk->position.x + z - x, x*z - z*z + x - chunk->position.z) + 1.0)*2.0);
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

int FloraGenerator::getTreeIndex(Biome *biome, int x, int z) {
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

int FloraGenerator::makeLeaves(Chunk *chunk, int step, int c, int dir, bool branch, int startStep, bool makeNode, int ntype) {
    int code, step2;
    if (makeNode) {
        lnodes.push_back(TreeNode(c, ntype, chunk));

        if (step == 0) {
            if (PseudoRand(chunk->position.x + c - chunk->position.z, chunk->position.z + c) > 0.85) lnodes.pop_back();
        }
    }

    if (branch) {
        if (step >= startStep - 1) {
            step2 = startStep;
        } else {
            step2 = step + 1;
        }
        code = makeLeaves(chunk, step2, c, (dir + 3) % 4, 0, startStep, 0, ntype);
        if (code) return code;
        code = makeLeaves(chunk, step2, c, (dir + 1) % 4, 0, startStep, 0, ntype);
        if (code) return code;
    }

    if (step == 0) return 0;

    if (dir == 0) //left
    {
        if (c % CHUNK_WIDTH) {
            return makeLeaves(chunk, step - 1, c - 1, dir, branch, startStep, 1, ntype);
        } else {
            if (chunk->left && chunk->left->isAccessible) {
                return makeLeaves(chunk->left, step - 1, c + CHUNK_WIDTH - 1, dir, branch, startStep, 1, ntype);
            }
            return 1;
        }
    } else if (dir == 1) //back
    {
        if ((c % CHUNK_LAYER) - CHUNK_WIDTH >= 0) {
            return makeLeaves(chunk, step - 1, c - CHUNK_WIDTH, dir, branch, startStep, 1, ntype);
        } else {
            if (chunk->back && chunk->back->isAccessible) {
                return makeLeaves(chunk->back, step - 1, c + CHUNK_LAYER - CHUNK_WIDTH, dir, branch, startStep, 1, ntype);
            }
            return 1;
        }
    } else if (dir == 2) //right
    {
        if ((c % CHUNK_WIDTH) < CHUNK_WIDTH - 1) {
            return makeLeaves(chunk, step - 1, c + 1, dir, branch, startStep, 1, ntype);
        } else {
            if (chunk->right && chunk->right->isAccessible) {
                return makeLeaves(chunk->right, step - 1, c - CHUNK_WIDTH + 1, dir, branch, startStep, 1, ntype);
            }
            return 1;
        }
    } else if (dir == 3) //front
    {
        if ((c % CHUNK_LAYER) + CHUNK_WIDTH < CHUNK_LAYER) {
            return makeLeaves(chunk, step - 1, c + CHUNK_WIDTH, dir, branch, startStep, 1, ntype);
        } else {
            if (chunk->front && chunk->front->isAccessible) {
                return makeLeaves(chunk->front, step - 1, c - CHUNK_LAYER + CHUNK_WIDTH, dir, branch, startStep, 1, ntype);
            }
            return 1;
        }
    }


    return 1;
}

int FloraGenerator::makeCluster(Chunk *chunk, int size, int c, int ntype) {
    int c2;
    int code;
    Chunk *ch2;
    if (size == 1) {
        code = makeLeaves(chunk, 1, c, 0, 1, 0, 0, ntype);
        if (code) return code;
        code = makeLeaves(chunk, 1, c, 1, 1, 0, 0, ntype);
        if (code) return code;
        code = makeLeaves(chunk, 1, c, 2, 1, 0, 0, ntype);
        if (code) return code;
        code = makeLeaves(chunk, 1, c, 3, 1, 0, 0, ntype);
        if (code) return code;
    } else {
        int code = makeLeaves(chunk, size, c, 0, 1, size, 0, ntype);
        if (code) return code;
        code = makeLeaves(chunk, size, c, 2, 1, size, 0, ntype);
        if (code) return code;
    }

    //up
    for (int i = size - 1; i >= 0; i--) {
        if (chunk->getTopBlockData(c, c / CHUNK_LAYER, &c2, &ch2) == VISITED_NODE) return 1;
        int code = makeLeaves(ch2, i, c2, 0, 1, i, 1, ntype);
        if (code) return code;
        code = makeLeaves(ch2, i, c2, 2, 1, i, 0, ntype);
        if (code) return code;

    }

    //down
    for (int i = size - 1; i >= 0; i--) {
        if (chunk->getBottomBlockData(c, c / CHUNK_LAYER, &c2, &ch2) == VISITED_NODE) return 1;
        int code = makeLeaves(ch2, i, c2, 0, 1, i, 1, ntype);
        if (code) return code;
        code = makeLeaves(ch2, i, c2, 2, 1, i, 0, ntype);
        if (code) return code;
    }
    return 0;
}

int FloraGenerator::makeRoundLeaves(Chunk *chunk, int c, int dir, bool branch, bool makeNode, int ntype, int dx, int dy, int dz, int rad) {
    int code;
    if (makeNode && chunk->getBlockID(c) == (ui16)Blocks::NONE) {
        lnodes.push_back(TreeNode(c, ntype, chunk));
    }

    if (branch) {
        code = makeRoundLeaves(chunk, c, (dir + 3) % 4, 0, 0, ntype, dx, dy, dz, rad);
        if (code) return code;
        code = makeRoundLeaves(chunk, c, (dir + 1) % 4, 0, 0, ntype, dx, dy, dz, rad);
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

    if ((int)(sqrt((double)(dx*dx + dy*dy + dz*dz))) >= rad) {
        if (PseudoRand(chunk->position.x + c - chunk->position.z, chunk->position.z + c) > 0.8 && lnodes.size()) lnodes.pop_back();
        return 0;
    }

    if (dir == 0) //left
    {
        if (c % CHUNK_WIDTH) {
            return makeRoundLeaves(chunk, c - 1, dir, branch, 1, ntype, dx, dy, dz, rad);
        } else {
            if (chunk->left && chunk->left->isAccessible) {
                return makeRoundLeaves(chunk->left, c + CHUNK_WIDTH - 1, dir, branch, 1, ntype, dx, dy, dz, rad);
            }
            return 1;
        }
    } else if (dir == 1) //back
    {
        if ((c % CHUNK_LAYER) - CHUNK_WIDTH >= 0) {
            return makeRoundLeaves(chunk, c - CHUNK_WIDTH, dir, branch, 1, ntype, dx, dy, dz, rad);
        } else {
            if (chunk->back && chunk->back->isAccessible) {
                return makeRoundLeaves(chunk->back, c + CHUNK_LAYER - CHUNK_WIDTH, dir, branch, 1, ntype, dx, dy, dz, rad);
            }
            return 1;
        }
    } else if (dir == 2) //right
    {
        if ((c % CHUNK_WIDTH) < CHUNK_WIDTH - 1) {
            return makeRoundLeaves(chunk, c + 1, dir, branch, 1, ntype, dx, dy, dz, rad);
        } else {
            if (chunk->right && chunk->right->isAccessible) {
                return makeRoundLeaves(chunk->right, c - CHUNK_WIDTH + 1, dir, branch, 1, ntype, dx, dy, dz, rad);
            }
            return 1;
        }
    } else if (dir == 3) //front
    {
        if ((c % CHUNK_LAYER) + CHUNK_WIDTH < CHUNK_LAYER) {
            return makeRoundLeaves(chunk, c + CHUNK_WIDTH, dir, branch, 1, ntype, dx, dy, dz, rad);
        } else {
            if (chunk->front && chunk->front->isAccessible) {
                return makeRoundLeaves(chunk->front, c - CHUNK_LAYER + CHUNK_WIDTH, dir, branch, 1, ntype, dx, dy, dz, rad);
            }
            return 1;
        }
    }


    return 1;
}

int FloraGenerator::makeMushroomLeaves(Chunk *chunk, int c, int dir, bool branch, bool makeNode, int ntype, int lamntype, int dx, int dy, int dz, int rad, TreeType *tt) {
    int code;
    int dist;

    if (branch) {
        code = makeMushroomLeaves(chunk, c, (dir + 3) % 4, 0, 0, ntype, lamntype, dx, dy, dz, rad, tt);
        if (code) return code;
        code = makeMushroomLeaves(chunk, c, (dir + 1) % 4, 0, 0, ntype, lamntype, dx, dy, dz, rad, tt);
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
        if (makeNode && chunk->getBlockID(c) == (ui16)Blocks::NONE) {
            lnodes.push_back(TreeNode(c, ntype, chunk));
        }
    } else if (dist > rad - (tt->mushroomCapThickness + tt->mushroomCapGillThickness)) {
        if (makeNode && chunk->getBlockID(c) == (ui16)Blocks::NONE) {
            lnodes.push_back(TreeNode(c, lamntype, chunk));
        }
    }

    if (dist >= rad) {
        if (PseudoRand(chunk->position.x + c - chunk->position.z, chunk->position.z + c) > 0.8 && lnodes.size()) lnodes.pop_back();
        return 0;
    }

    if (dir == 0) //left
    {
        if (c % CHUNK_WIDTH) {
            return makeMushroomLeaves(chunk, c - 1, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        } else {
            if (chunk->left && chunk->left->isAccessible) {
                return makeMushroomLeaves(chunk->left, c + CHUNK_WIDTH - 1, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
            }
            return 1;
        }
    } else if (dir == 1) //back
    {
        if ((c % CHUNK_LAYER) - CHUNK_WIDTH >= 0) {
            return makeMushroomLeaves(chunk, c - CHUNK_WIDTH, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        } else {
            if (chunk->back && chunk->back->isAccessible) {
                return makeMushroomLeaves(chunk->back, c + CHUNK_LAYER - CHUNK_WIDTH, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
            }
            return 1;
        }
    } else if (dir == 2) //right
    {
        if ((c % CHUNK_WIDTH) < CHUNK_WIDTH - 1) {
            return makeMushroomLeaves(chunk, c + 1, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        } else {
            if (chunk->right && chunk->right->isAccessible) {
                return makeMushroomLeaves(chunk->right, c - CHUNK_WIDTH + 1, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
            }
            return 1;
        }
    } else if (dir == 3) //front
    {
        if ((c % CHUNK_LAYER) + CHUNK_WIDTH < CHUNK_LAYER) {
            return makeMushroomLeaves(chunk, c + CHUNK_WIDTH, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
        } else {
            if (chunk->front && chunk->front->isAccessible) {
                return makeMushroomLeaves(chunk->front, c - CHUNK_LAYER + CHUNK_WIDTH, dir, branch, 1, ntype, lamntype, dx, dy, dz, rad, tt);
            }
            return 1;
        }
    }


    return 1;
}

int FloraGenerator::makeDroopyLeaves(Chunk *chunk, int length, int c, TreeData *td, int ntype) {
    Chunk *ch;
    int c2;
    int slope;
    int dslope = td->treeType->droopyLeavesDSlope;
    int step;
    int slopestep;
    int i;
    int dir = 0;
    bool side, down;

    if (chunk->getBlockID(c) == (ui16)Blocks::NONE) {
        lnodes.push_back(TreeNode(c, ntype, chunk));
    }

    //left
    for (dir = 0; dir < 4; dir++) {
        c2 = c;
        ch = chunk;
        slope = td->treeType->droopyLeavesSlope;
        if (slope == 0) slope = -1;
        slopestep = step = 0;
        for (i = 0; i < length; i++) {
            side = down = 0;

            if (slope > 0) { //shallow slope
                side = 1;
                if (step >= slope) {
                    step = -1;
                    slopestep++;
                    down = 1;
                }
            } else { //steep slope
                down = 1;
                if (step >= -slope && slope > -3) {
                    step = -1;
                    slopestep++;
                    side = 1;
                }
            }

            if (side) {
                if (dir == 0) {
                    if (c2%CHUNK_WIDTH < CHUNK_WIDTH - 1) {
                        c2++;
                    } else if (ch->right && ch->right->isAccessible) {
                        c2 = c2 - CHUNK_WIDTH + 1;
                        ch = ch->right;
                    } else {
                        return 1;
                    }
                } else if (dir == 1) {
                    if (c2%CHUNK_WIDTH > 0) {
                        c2--;
                    } else if (ch->left && ch->left->isAccessible) {
                        c2 = c2 + CHUNK_WIDTH + -1;
                        ch = ch->left;
                    } else {
                        return 1;
                    }
                } else if (dir == 2) {
                    if ((c2%CHUNK_LAYER) / CHUNK_WIDTH < CHUNK_WIDTH - 1) {
                        c2 += CHUNK_WIDTH;
                    } else if (ch->front && ch->front->isAccessible) {
                        c2 = c2 + CHUNK_WIDTH - CHUNK_LAYER;
                        ch = ch->front;
                    } else {
                        return 1;
                    }
                } else if (dir == 3) {
                    if ((c2%CHUNK_LAYER) / CHUNK_WIDTH > 0) {
                        c2 -= CHUNK_WIDTH;
                    } else if (ch->back && ch->back->isAccessible) {
                        c2 = c2 - CHUNK_WIDTH + CHUNK_LAYER;
                        ch = ch->back;
                    } else {
                        return 1;
                    }
                }

                if (ch->getBlockID(c2) == (ui16)Blocks::NONE) {
                    lnodes.push_back(TreeNode(c2, ntype, ch));
                }
            }
            if (down) {
                if (c2 / CHUNK_LAYER > 0) {
                    c2 -= CHUNK_LAYER;
                } else if (ch->bottom && ch->bottom->isAccessible) {
                    c2 = c2 - CHUNK_LAYER + CHUNK_SIZE;
                    ch = ch->bottom;
                } else {
                    return 1;
                }

                if (ch->getBlockID(c2) == (ui16)Blocks::NONE) {
                    lnodes.push_back(TreeNode(c2, ntype, ch));
                }
            }

            if (slopestep >= dslope) {
                slope--;
                if (slope == 0) slope = -1;
                slopestep = 0;
            }

            step++;
        }
    }
    return 0;
}

int FloraGenerator::makeMushroomCap(Chunk *chunk, int c, TreeData &td, int block, int rad) {
    Chunk *ch = chunk;
    int c2 = c;
    int code;
    int step;
    int k = td.treeType->mushroomCapLengthMod + 1;
    int size;

    if (td.treeType->mushroomCapLengthMod >= 0) {
        size = rad + rad*td.treeType->mushroomCapLengthMod;
    } else {
        size = rad / 2;
    }

    step = 0;
    for (int i = 0; i < size; i++) {
        if (td.treeType->isMushroomCapInverted == 1) {
            c2 += CHUNK_LAYER;
            if (c2 >= CHUNK_SIZE) {
                if (ch->top && ch->top->isAccessible) {
                    c2 -= CHUNK_SIZE;
                    ch = ch->top;
                } else {
                    return 1;
                }
            }
        } else {
            c2 -= CHUNK_LAYER;
            if (c2 < 0) {
                if (ch->bottom && ch->bottom->isAccessible) {
                    c2 += CHUNK_SIZE;
                    ch = ch->bottom;
                } else {
                    return 1;
                }
            }
        }


        if (k == td.treeType->mushroomCapLengthMod + 1) {
            k = 0;
            step++;
        } else if (td.treeType->mushroomCapLengthMod < 0) {
            k = 0;
            step += 2;
        }

        code = makeMushroomLeaves(ch, c2, 0, 1, 1, block, td.treeType->idSpecial, 0, rad - step, 0, rad, td.treeType);
        if (code) return code;
        code = makeMushroomLeaves(ch, c2, 2, 1, 0, block, td.treeType->idSpecial, 0, rad - step, 0, rad, td.treeType);
        if (code) return code;
        k++;
    }
    if (td.treeType->isMushroomCapInverted == 0) { //round bottom portion
        while (step > rad - td.treeType->mushroomCapCurlLength) {
            step -= 2;

            c2 -= CHUNK_LAYER;
            if (c2 < 0) {
                if (ch->bottom && ch->bottom->isAccessible) {
                    c2 += CHUNK_SIZE;
                    ch = ch->bottom;
                } else {
                    return 1;
                }
            }

            code = makeMushroomLeaves(ch, c2, 0, 1, 1, block, td.treeType->idSpecial, 0, rad - step, 0, rad, td.treeType);
            if (code) return code;
            code = makeMushroomLeaves(ch, c2, 2, 1, 0, block, td.treeType->idSpecial, 0, rad - step, 0, rad, td.treeType);
            if (code) return code;
        }
    }

    return 0;
}

int FloraGenerator::makeBranch(Chunk *chunk, int step, int initStep, int c, int dir, bool makeNode, TreeData *td, int initDir, int initSize, int leafSizeMod, bool isRoot, int initBranchDir) {

    Chunk *ch, *ch2;
    ch = chunk;
    int code;
    int c2;
    int ymod;
    int branchDir;
    int size = 0;
    int leafBlock = td->treeType->idLeaves | (td->leafColor << 12);
    int woodBlock = td->treeType->idOuter;
    if (isRoot) woodBlock = td->treeType->idRoot;
    int MINinit = MIN(initSize, 3);
    int MAXinit = MAX(initSize - 3, MIN(initSize, 1));

    while (1) {
        if (initStep && initSize) size = (initStep - (initStep - step)) / ((float)initStep)*MINinit + MAXinit; //we lose 3 size levels

        if (makeNode && step) {

            wnodes.push_back(TreeNode(c, woodBlock, ch));

            makeCluster(ch, size, c, woodBlock);


            int r = (int)((PseudoRand(ch->position.x*c + c + step, c*c - ch->position.z + step) + 1)*7.5); //between 0 and 14

            if (r > 8 + isRoot * 2) {
                branchDir = initDir;
            } else if (r > 3) {
                branchDir = 4;
            } else if (dir == 0) {
                if (r > 1) {
                    branchDir = 3;
                } else {
                    branchDir = 1;
                }
            } else if (dir == 1) {
                if (r > 1) {
                    branchDir = 0;
                } else {
                    branchDir = 2;
                }
            } else if (dir == 2) {
                if (r > 1) {
                    branchDir = 1;
                } else {
                    branchDir = 3;
                }
            } else if (dir == 3) {
                if (r > 1) {
                    branchDir = 2;
                } else {
                    branchDir = 0;
                }
            }

            if (size && rand() % 1000 < 200 - size * 4) {
                int dr = rand() % 4;
                if (dr == branchDir) dr = (dr + 1) % 4;
                if (dr == (initDir + 2) % 4) dr = (dr + 1) % 4; //must branch orthogonally
                code = makeBranch(ch, step / 2, initStep, c, dr, 0, td, dr, size - 1, leafSizeMod, isRoot, initBranchDir);
                if (code) return code;
            }

        } else {
            branchDir = dir;
        }

        if (step == 1 && !isRoot) //leaves
        {
            ch2 = ch;
            c2 = c;
            if (size + 2 + leafSizeMod > ABS(td->treeType->branchLeafYMod)) {
                ymod = td->treeType->branchLeafYMod;
            } else {
                if (td->treeType->branchLeafYMod > 0) {
                    ymod = size + 2 + leafSizeMod;
                } else {
                    ymod = -(size + 2 + leafSizeMod);
                }
            }
            if (ymod > 0) {
                for (int j = 0; j < ymod; j++) {
                    if (c2 / CHUNK_LAYER < CHUNK_WIDTH - 1) {
                        c2 += CHUNK_LAYER;
                    } else if (ch2->top && ch2->top->isAccessible) {
                        c2 = c2 + CHUNK_LAYER - CHUNK_SIZE;
                        ch2 = ch2->top;
                    } else {
                        return 1;
                    }
                }
            } else if (ymod < 0) {
                for (int j = 0; j < -ymod; j++) {
                    if (c2 / CHUNK_LAYER > 0) {
                        c2 -= CHUNK_LAYER;
                    } else if (ch2->bottom && ch2->bottom->isAccessible) {
                        c2 = c2 - CHUNK_LAYER + CHUNK_SIZE;
                        ch2 = ch2->bottom;
                    } else {
                        return 1;
                    }
                }
            }

            if (td->treeType->hasDroopyLeaves) {
                code = makeDroopyLeaves(ch2, td->droopyLength, c2, td, leafBlock);
                if (code) return code;
            }

            if (td->treeType->branchLeafShape == TreeLeafShape::ROUND) {
                code = makeSphere(ch2, c2, leafBlock, size + 3 + leafSizeMod);
            } else if (td->treeType->branchLeafShape == TreeLeafShape::CLUSTER) {
                code = makeCluster(ch2, size + 3 + leafSizeMod, c2, leafBlock);
            } else {
                code = 0;
            }
            if (code) return code;
        }

        if (branchDir == (initDir + 2) % 4) {
            branchDir = initDir;
        }
        if (step == 0) return 0;

        if (branchDir == 0) { //left
            if (c % CHUNK_WIDTH) {
                c--;
            } else if (ch->left && ch->left->isAccessible) {
                ch = ch->left;
                c = c + CHUNK_WIDTH - 1;
            } else {
                return 1;
            }
        } else if (branchDir == 1) { //back
            if ((c % CHUNK_LAYER) - CHUNK_WIDTH >= 0) {
                c = c - CHUNK_WIDTH;
            } else if (ch->back && ch->back->isAccessible) {
                c = c + CHUNK_LAYER - CHUNK_WIDTH;
                ch = ch->back;
            } else {
                return 1;
            }
        } else if (branchDir == 2) { //right
            if ((c % CHUNK_WIDTH) < CHUNK_WIDTH - 1) {
                c++;
            } else if (ch->right && ch->right->isAccessible) {
                c = c - CHUNK_WIDTH + 1;
                ch = ch->right;
            } else {
                return 1;
            }
        } else if (branchDir == 3) { //front
            if ((c % CHUNK_LAYER) + CHUNK_WIDTH < CHUNK_LAYER) {
                c += CHUNK_WIDTH;
            } else if (ch->front && ch->front->isAccessible) {
                c = c - CHUNK_LAYER + CHUNK_WIDTH;
                ch = ch->front;
            } else {
                return 1;
            }
        } else if (branchDir == 4) { //top or bottom

            if (initBranchDir == 2 || isRoot) { //down
                if (c - CHUNK_LAYER >= 0) {
                    c -= CHUNK_LAYER;
                } else if (ch->bottom && ch->bottom->isAccessible) {
                    ch = ch->bottom;
                    c = c + CHUNK_SIZE - CHUNK_LAYER;
                } else {
                    return 1;
                }
            } else if (initBranchDir == 1) { //either dir
                int r = rand() % 2;
                if (r == 0) {
                    if (c - CHUNK_LAYER >= 0) {
                        c -= CHUNK_LAYER;
                    } else if (ch->bottom && ch->bottom->isAccessible) {
                        ch = ch->bottom;
                        c = c + CHUNK_SIZE - CHUNK_LAYER;
                    } else {
                        return 1;
                    }
                } else {
                    if (c + CHUNK_LAYER < CHUNK_SIZE) {
                        c += CHUNK_LAYER;
                    } else if (ch->top && ch->top->isAccessible) {
                        c = c%CHUNK_LAYER;
                        ch = ch->top;
                    } else {
                        return 1;
                    }
                }
            } else { //up
                if (c + CHUNK_LAYER < CHUNK_SIZE) {
                    c += CHUNK_LAYER;
                } else if (ch->top && ch->top->isAccessible) {
                    c = c%CHUNK_LAYER;
                    ch = ch->top;
                } else {
                    return 1;
                }
            }
        }
        makeNode = 1;
        step--;
    }
    return 1;
}

int FloraGenerator::makeSphere(Chunk *chunk, int c, int block, int rad) {
    int code;
    int c2;
    Chunk *ch = chunk;

    c2 = c;

    //up
    for (int i = 1; i < rad; i++) {
        code = makeRoundLeaves(ch, c2, 0, 1, 1, block, 0, i, 0, rad);
        if (code) return code;
        code = makeRoundLeaves(ch, c2, 2, 1, 0, block, 0, i, 0, rad);
        if (code) return code;
        c2 += CHUNK_LAYER;
        if (c2 >= CHUNK_SIZE) {
            if (ch->top && ch->top->isAccessible) {
                ch = ch->top;
                c2 -= CHUNK_SIZE;
            } else {
                return 1;
            }
        }
    }

    //down
    ch = chunk;
    c2 = c - CHUNK_LAYER;
    if (c2 < 0) {
        if (ch->bottom && ch->bottom->isAccessible) {
            ch = ch->bottom;
            c2 += CHUNK_SIZE;
        } else {
            return 1;
        }
    }
    for (int i = 1; i < rad; i++) {
        code = makeRoundLeaves(ch, c2, 0, 1, 1, block, 0, i, 0, rad);
        if (code) return code;
        code = makeRoundLeaves(ch, c2, 2, 1, 0, block, 0, i, 0, rad);
        if (code) return code;
        c2 -= CHUNK_LAYER;
        if (c2 < 0) {
            if (ch->bottom && ch->bottom->isAccessible) {
                ch = ch->bottom;
                c2 += CHUNK_SIZE;
            } else {
                return 1;
            }
        }
    }

    return 0;
}

void lerpBranch(const TreeBranchingProps& top, const TreeBranchingProps& bottom, TreeData& outProps, const f32& ratio) {
    outProps.botBranchChance = ratio * (bottom.chance.max - bottom.chance.min) + bottom.chance.min;
    outProps.topBranchChance = ratio * (top.chance.max - top.chance.min) + top.chance.min;
    outProps.botBranchLength = ratio * (bottom.length.max - bottom.length.min) + bottom.length.min;
    outProps.topBranchLength = ratio * (top.length.max - top.length.min) + top.length.min;
    outProps.botBranchWidth = ratio * (bottom.width.max - bottom.width.min) + bottom.width.min;
    outProps.topBranchWidth = ratio * (top.width.max - top.width.min) + top.width.min;
}
