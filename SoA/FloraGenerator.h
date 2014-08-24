#pragma once
#include "utils.h"
#include "Keg.h"

class Chunk;
struct TreeType;
struct TreeData;
struct TreeNode;
struct Biome;

//static flora generation class. Both trees and plants are considered flora
class FloraGenerator {
public:
    static i32 generateFlora(Chunk *chunk);
    static i32 generateTreeNodes(Chunk *chunk, i32 sc, TreeData &treeData);
    static i32 makeLODTreeData(TreeData &td, TreeType *tt, i32 x, i32 z, i32 X, i32 Z);
    static i32 makeTreeData(Chunk *chunk, TreeData &td, TreeType *tt);
    static i32 getTreeIndex(Biome *biome, i32 x, i32 z);

    //Places all the blocks for the tree
    static void placeTreeNodes();
    //Places all the blocks for the tree and records the old blocks
    static void placeTreeNodesAndRecord();

private:
    static i32 makeBranch(Chunk *chunk, i32 step, i32 initStep, i32 c, i32 dir, bool makeNode, TreeData *td, i32 initDir, i32 initSize, i32 leafSizeMod, bool isRoot, i32 initBranchDir);
    static i32 makeLeaves(Chunk *chunk, i32 step, i32 c, i32 dir, bool branch, i32 startStep, bool makeNode, i32 ntype);
    static i32 makeCluster(Chunk *chunk, i32 size, i32 c, i32 ntype);
    static i32 makeSphere(Chunk *chunk, i32 c, i32 block, i32 rad);
    static i32 makeRoundLeaves(Chunk *chunk, i32 c, i32 dir, bool branch, bool makeNode, i32 ntype, i32 dx, i32 dy, i32 dz, i32 rad);
    static i32 makeMushroomLeaves(Chunk *chunk, i32 c, i32 dir, bool branch, bool makeNode, i32 ntype, i32 lamntype, i32 dx, i32 dy, i32 dz, i32 rad, TreeType *tt);
    static i32 makeDroopyLeaves(Chunk *chunk, i32 length, i32 c, TreeData *td, i32 ntype);
    static i32 makeMushroomCap(Chunk *chunk, i32 c, TreeData &td, i32 block, i32 rad);

    static std::vector<TreeNode> wnodes;
    static std::vector<TreeNode> lnodes;

    //The root chunk for generation
    // Chunk *_chunk;
};

enum class TreeLeafShape {
    UNKNOWN,
    ROUND,
    CLUSTER,
    PINE,
    MUSHROOM
};
KEG_ENUM_DECL(TreeLeafShape);

struct TreeBranchingProps {
    I32Range width;
    I32Range length;
    F32Range chance;
    i32 direction;
};
KEG_TYPE_DECL(TreeBranchingProps);



//This is data specific to a breed of tree
struct TreeType {
    TreeType() {
        memset(this, 0, sizeof(TreeType)); //zero the memory
        name = "MISSING NAME";
        rootDepthMult = 1.0;
    }

    i32 idCore, idOuter, idLeaves, idRoot, idSpecial;

    I32Range trunkHeight;
    I32Range trunkBaseHeight;

    I32Range trunkBaseWidth;
    I32Range trunkMidWidth;
    I32Range trunkTopWidth;
    i32 coreWidth;

    I32Range trunkStartSlope;
    I32Range trunkEndSlope;

    TreeBranchingProps branchingPropsBottom, branchingPropsTop;

    I32Range droopyLength;
    I32Range leafCapSize;

    TreeLeafShape leafCapShape, branchLeafShape;

    i32 branchLeafSizeMod;
    i32 branchLeafYMod;

    i32 droopyLeavesSlope, droopyLeavesDSlope;

    i32 mushroomCapLengthMod;
    i32 mushroomCapCurlLength;
    i32 mushroomCapThickness;
    i32 mushroomCapGillThickness;

    f32 capBranchChanceMod;
    f32 trunkChangeDirChance;
    f32 rootDepthMult;
    f32 branchStart;

    bool hasThickCapBranches;
    bool hasDroopyLeaves;
    bool isSlopeRandom;
    bool isMushroomCapInverted;

    nString name;
    nString fileName;
    std::vector<i32> possibleAltLeafFlags;
};
KEG_TYPE_DECL(TreeType);

//This is data specific to an instance of a tree
struct TreeData {
    i32 type, startc;
    i32 trunkBaseWidth, trunkMidWidth, trunkTopWidth;
    i32 trunkStartSlope, trunkEndSlope;
    i32 trunkDir;
    i32 treeHeight;
    i32 treeBaseHeight;
    i32 botBranchLength;
    i32 topBranchLength;
    i32 botBranchWidth;
    i32 topBranchWidth;
    i32 topLeafSize;
    i32 leafColor;
    i32 branchStart;
    i32 droopyLength;
    f32 botBranchChance, topBranchChance, ageMod;
    TreeType *treeType;
};

typedef void(*TreeBranchInterpolator)(const TreeBranchingProps& top, const TreeBranchingProps& bottom, TreeData& outProps, const f32& ratio);
void lerpBranch(const TreeBranchingProps& top, const TreeBranchingProps& bottom, TreeData& outProps, const f32& ratio);

//This node is used in tree generation
struct TreeNode {
    TreeNode(ui16 C, ui16 BlockType, Chunk* Owner) : c(C), blockType(BlockType), owner(Owner) {}

    ui16 c;
    ui16 blockType;
    Chunk* owner;
};

//This is data specific to a breed of plant
struct PlantType {
    nString name;
    i32 baseBlock;
};

//This is data specific to an instance of a plant
struct PlantData {
    PlantData(PlantType *plantType, i32 C) : ft(plantType), startc(C) {}
    PlantType *ft;
    i32 startc;
};