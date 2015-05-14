#pragma once
#include "Errors.h"

extern std::vector<ui32v2> SCREEN_RESOLUTIONS;

struct SoaOptionFlags {
    bool needsWindowReload : 1;
    bool needsFBOReload : 1;
    bool needsShaderReload : 1;
    bool needsTextureReload : 1;
};

struct OptionValue {
    union {
        f64 lf;
        f32 f;
        ui64 lui;
        ui32 ui;
        i64 li;
        i32 i;
        bool b;
        char c;
    };
};

class SoaOption {
public:
    nString name;
    OptionValue defaultValue;
    OptionValue value;
    SoaOptionFlags flags;
};

struct SoaOptions {
    //****************** Graphics Options ******************//
    i32 lodDistance;
    i32 lodDetail = 1;
    i32 voxelRenderDistance = 144;
    i32 hudMode = 0;
    i32 currTextureRes, defaultTextureRes;

    i32 motionBlur = 8;
    i32 depthOfField = 0;
    i32 msaa = 0;
    i32 maxMsaa = 32;

    f32 specularExponent = 8.0f;
    f32 specularIntensity = 0.3f;
    f32 hdrExposure = 1.3f;
    f32 gamma = 1.0f;
    f32 secColorMult = 0.1f;
    f32 fov = 70.0f;
    f32 maxFPS = 60.0f;
    f32 voxelLODThreshold = 128.0f;
    f32 voxelLODThreshold2 = voxelLODThreshold * voxelLODThreshold;

    bool needsWindowReload;
    bool needsFboReload = false;
    bool needsFullscreenToggle = false;
    bool enableParticles = true;

    nString defaultTexturePack = "Default";
    nString currTexturePack = defaultTexturePack; 

    //****************** Sound Options ******************//
    f32 musicVolume = 1.0f;
    f32 effectVolume = 1.0f;

    //****************** Game Options ******************//
    f32 mouseSensitivity = 30.0f;
    bool invertMouse = false;

    //****************** Menu Options ******************//
    nString newGameString = "", loadGameString = "", selectPlanetName = "", markerName = "";
    i32 markerR = 0, markerG = 0, markerB = 0;
};

extern bool loadOptions(const cString filePath);
