///
/// Flora.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 15 Jul 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Flora struct definitions.
///

#pragma once

#ifndef Flora_h__
#define Flora_h__

#include <Vorb/io/Keg.h>
#include <Vorb/utils.h>

#define FLORA_ID_NONE 0xFFFFu
// Both trees and flora share FloraID
typedef ui16 FloraID;

/* TreeType is a breed of tree, TreeData is an individual tree.*/

enum class TreeLeafType {
    NONE,
    ROUND,
    PINE,
    MUSHROOM
};
KEG_ENUM_DECL(TreeLeafType);

struct TreeTypeFruitProperties {
    FloraID flora = FLORA_ID_NONE;
    Range<f32> chance;
};
struct TreeFruitProperties {
    FloraID flora = FLORA_ID_NONE;
    f32 chance;
};

struct TreeTypeLeafProperties {
    TreeLeafType type;
    TreeTypeFruitProperties fruitProps;
    // Mutually exclusive union based on type
    union {
        UNIONIZE(struct {
            Range<ui16> vRadius;
            Range<ui16> hRadius;
            ui16 blockID;
        } round;);
        UNIONIZE(struct {
            Range<ui16> oRadius;
            Range<ui16> iRadius;
            Range<ui16> period;
            ui16 blockID;
        } pine;);
        UNIONIZE(struct {
            Range<ui16> tvRadius;
            Range<ui16> thRadius;
            Range<ui16> bvRadius;
            Range<ui16> bhRadius;
            Range<ui16> capWidth;
            Range<ui16> gillWidth;
            ui16 gillBlockID;
            ui16 capBlockID;
        } mushroom;);
    };
};
struct TreeLeafProperties {
    TreeLeafType type;
    TreeFruitProperties fruitProps;
    // Mutually exclusive union based on type
    union {
        UNIONIZE(struct {
            ui16 vRadius;
            ui16 hRadius;
            ui16 blockID;
        } round;);
        UNIONIZE(struct {
            ui16 oRadius;
            ui16 iRadius;
            ui16 period;
            ui16 blockID;
        } pine;);
        UNIONIZE(struct {
            ui16 tvRadius;
            ui16 thRadius;
            ui16 bvRadius;
            ui16 bhRadius;
            ui16 capWidth;
            ui16 gillWidth;
            ui16 gillBlockID;
            ui16 capBlockID;
        } mushroom;);
    };
};

struct TreeTypeBranchProperties {
    Range<ui16> coreWidth;
    Range<ui16> barkWidth;
    Range<ui16> length;
    Range<f32> branchChance;
    Range<f32> angle;
    Range<Range<i32>> segments;
    f32 endSizeMult;
    ui16 coreBlockID = 0;
    ui16 barkBlockID = 0;
    TreeTypeFruitProperties fruitProps;
    TreeTypeLeafProperties leafProps;
};
struct TreeBranchProperties {
    f32 branchChance;
    ui16 coreWidth;
    ui16 barkWidth;
    ui16 length;
    ui16 coreBlockID = 0;
    ui16 barkBlockID = 0;
    Range<f32> angle;
    Range<i32> segments;
    f32 endSizeMult;
    TreeFruitProperties fruitProps;
    TreeLeafProperties leafProps;
};

struct TreeTypeTrunkProperties {
    f32 loc;
    Range<ui16> coreWidth;
    Range<ui16> barkWidth;
    Range<f32> branchChance;
    Range<Range<i32>> slope;
    ui16 coreBlockID = 0;
    ui16 barkBlockID = 0;
    TreeTypeFruitProperties fruitProps;
    TreeTypeLeafProperties leafProps;
    TreeTypeBranchProperties branchProps;
};
struct TreeTrunkProperties {
    f32 loc;
    ui16 coreWidth;
    ui16 barkWidth;
    f32 branchChance;
    Range<i32> slope;
    ui16 coreBlockID;
    ui16 barkBlockID;
    TreeFruitProperties fruitProps;
    TreeLeafProperties leafProps;
    TreeBranchProperties branchProps;
};

struct NTreeType {
    // All ranges are for scaling between baby tree and adult tree
    Range<ui16> height;
    // Data points for trunk properties. Properties get interpolated between these from
    // base of tree to top of tree.
    std::vector<TreeTypeTrunkProperties> trunkProps;
};

// Specification for an individual tree
struct TreeData {
    f32 age; ///< 0 - 1
    ui16 height;
    std::vector<TreeTrunkProperties> trunkProps;
};

enum class FloraDir {
    UP, SIDE, DOWN
};
KEG_ENUM_DECL(FloraDir);

struct FloraType {
    ui16 block;
    Range<ui16> height;
    Range<ui16> slope;
    Range<ui16> dSlope;
    FloraDir dir;
    const FloraType* nextFlora = nullptr;
};

// Specification for individual flora
struct FloraData {
    ui16 block;
    ui16 height;
    ui16 dir;
    ui16 slope;
    ui16 dSlope;
    const FloraType* nextFlora;
};

#endif // Flora_h__
