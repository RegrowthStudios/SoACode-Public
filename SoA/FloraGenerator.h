#pragma once
#include <Vorb/utils.h>
#include <Vorb/io/Keg.h>

class Chunk;
class TreeType;
class TreeData;

//This node is used in tree generation
class TreeNode {
public:
    TreeNode(ui16 BlockIndex, ui16 ChunkOffset, ui16 BlockType) : blockIndex(BlockIndex), chunkOffset(ChunkOffset), blockType(BlockType) {}

    ui16 blockIndex;
    ui16 blockType;
    ui16 chunkOffset; ///< 0 XXXXX YYYYY ZZZZZ for positional offset. 00111 == 0
};

//static flora generation class. Both trees and plants are considered flora
class FloraGenerator {
public:

    // Don't change TreeDir order
    enum TreeDir {
        TREE_LEFT = 0, TREE_BACK, TREE_RIGHT, TREE_FRONT, TREE_UP, TREE_DOWN, TREE_NO_DIR
    };

    bool generateFlora(Chunk* chunk, std::vector<TreeNode>& wnodes, std::vector<TreeNode>& lnodes);
    static i32 makeLODTreeData(TreeData &td, TreeType *tt, i32 x, i32 z, i32 X, i32 Z);
    static i32 makeTreeData(Chunk* chunk, TreeData &td, TreeType *tt);
    static i32 getTreeIndex(Biome *biome, i32 x, i32 z);

private:
    // generates a tree and stores the resulting nodes
    bool generateTree(const TreeData& treeData, Chunk* startChunk);
    bool generateTrunk();
    bool makeTrunkSlice(int blockIndex, ui16 chunkOffset, int h, float heightRatio);
    bool makeTrunkOuterRing(int blockIndex, ui16 chunkOffset, int x, int z, int coreWidth, int thickness, int blockID, std::vector<TreeNode>* nodes);
    void directionalMove(int& blockIndex, ui16 &chunkOffset, TreeDir dir);
    bool recursiveMakeSlice(int blockIndex, ui16 chunkOffset, i32 step, TreeDir dir, TreeDir rightDir, TreeDir leftDir, bool makeNode, i32 blockID, std::vector<TreeNode>* nodes);

    bool recursiveMakeBranch(int blockIndex, ui16 chunkOffset, int step, TreeDir dir, TreeDir initDir, int thickness, bool isRoot);
    bool makeSphere(int blockIndex, ui16 chunkOffset, int radius, int blockID, std::vector<TreeNode>* nodes);
    bool makeCluster(int blockIndex, ui16 chunkOffset, int size, int blockID, std::vector<TreeNode>* nodes);
    i32 makeMushroomLeaves(i32 c, ui16 chunkOffset, i32 dir, bool branch, bool makeNode, i32 ntype, i32 lamntype, i32 dx, i32 dy, i32 dz, i32 rad, TreeType *tt);
    bool makeDroopyLeaves(int blockIndex, ui16 chunkOffset, int length, int blockID, std::vector<TreeNode>* nodes);
    i32 makeMushroomCap(i32 c, ui16 chunkOffset, i32 block, i32 rad);

    std::vector<TreeNode>* _wnodes;
    std::vector<TreeNode>* _lnodes;

    const TreeData* _treeData;
};

enum class TreeLeafShape {
    UNKNOWN,
    ROUND,
    CLUSTER,
    PINE,
    MUSHROOM
};
KEG_ENUM_DECL(TreeLeafShape);

class TreeBranchingProps {
public:
    I32Range width;
    I32Range length;
    F32Range chance;
    i32 direction;
};
KEG_TYPE_DECL(TreeBranchingProps);



//This is data specific to a breed of tree
class TreeType {
public:
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
class TreeData {
public:
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

//This is data specific to a breed of plant
class PlantType {
public:
    nString name;
    i32 baseBlock;
};

//This is data specific to an instance of a plant
class PlantData {
public:
    PlantData(PlantType *plantType, i32 C) : ft(plantType), startc(C) {}
    PlantType *ft;
    i32 startc;
};