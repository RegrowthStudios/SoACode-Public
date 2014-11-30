#pragma once
#include <ZLIB/unzip.h>

// Tree Variable Enumerations
enum TreeVariableVarType {
    TREE_VARIABLE_VAR_TYPE_I32 = 0,
    TREE_VARIABLE_VAR_TYPE_F32 = 1
};
enum TreeVariableControlType {
    TREE_VARIABLE_CONTROL_TYPE_SLIDER = 0,
    TREE_VARIABLE_CONTROL_TYPE_LIST_BOX = 1
};
class TreeVariable {
public:
    TreeVariable() {}
    TreeVariable(f32 min, f32 max, f32 step, i32 byteOffset, i32 varType) :
        step(step),
        byteOffset(byteOffset),
        varType(varType),
        controlType(TREE_VARIABLE_CONTROL_TYPE_SLIDER),
        byteOffset2(-1),
        editorAccessible(1) {
        this->min = min;
        this->max = max;
    }

    f32 min, max;
    f32 step;
    i32 byteOffset;
    i32 byteOffset2;
    i32 varType;
    i32 controlType;
    bool editorAccessible;
    std::vector<nString> listNames;
};

// Biome Variable Enumerations
enum BiomeVariableVarType {
    BIOME_VARIABLE_VAR_TYPE_I32 = 0,
    BIOME_VARIABLE_VAR_TYPE_F32 = 1,
    BIOME_VARIABLE_VAR_TYPE_UI8 = 2,
    BIOME_VARIABLE_VAR_TYPE_UI16 = 3
};
enum BiomeVariableControlType {
    BIOME_VARIABLE_CONTROL_TYPE_SLIDER = 0,
    BIOME_VARIABLE_CONTROL_TYPE_LIST_BOX = 1
};
class BiomeVariable {
public:
    BiomeVariable() {}
    BiomeVariable(f32 min, f32 max, f32 step, i32 byteOffset, i32 varType) :
        step(step),
        byteOffset(byteOffset),
        varType(varType),
        controlType(BIOME_VARIABLE_CONTROL_TYPE_SLIDER),
        byteOffset2(-1),
        editorAccessible(1) {
        this->min = min;
        this->max = max;
    }

    f32 min, max;
    f32 step;
    i32 byteOffset;
    i32 byteOffset2;
    i32 varType;
    i32 controlType;
    bool editorAccessible;
    std::vector<nString> listNames;
};

// Biome Variable Enumerations
enum NoiseVariableVarType {
    NOISE_VARIABLE_VAR_TYPE_I32 = 0,
    NOISE_VARIABLE_VAR_TYPE_F64 = 1
};
enum NoiseVariableControlType {
    NOISE_VARIABLE_CONTROL_TYPE_SLIDER = 0,
    NOISE_VARIABLE_CONTROL_TYPE_LIST_BOX = 1
};
class NoiseVariable {
public:
    NoiseVariable() {}
    NoiseVariable(f32 min, f32 max, f32 step, i32 byteOffset, i32 varType) :
        step(step),
        byteOffset(byteOffset),
        varType(varType),
        controlType(NOISE_VARIABLE_CONTROL_TYPE_SLIDER),
        byteOffset2(-1) {
            this->min = min;
            this->max = max;
        }

    f32 min, max;
    f32 step;
    i32 byteOffset;
    i32 byteOffset2;
    i32 varType;
    i32 controlType;
    std::vector<nString> listNames;
};