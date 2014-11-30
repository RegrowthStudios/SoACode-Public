#pragma once
#include "compat.h"

#include "Errors.h"


extern std::vector<ui32v2> SCREEN_RESOLUTIONS;

class GraphicsOptions {
public:
    i32 cloudDetail, lodDistance, lodDetail, isFancyTrees, enableParticles, chunkLoadTime;
    i32 voxelRenderDistance, hudMode;
    i32 currTextureRes, defaultTextureRes;
    i32 motionBlur;
    i32 msaa, maxMsaa;
    f32 specularExponent, specularIntensity, lookSensitivity;
    f32 hdrExposure, gamma;
    f32 secColorMult, fov;
    i32 maxFPS;
    f32 voxelLODThreshold, voxelLODThreshold2;
    bool isVsync, needsWindowReload, needsFboReload, needsFullscreenToggle;
    nString texturePackString, currTexturePack, defaultTexturePack;
};
extern GraphicsOptions graphicsOptions;

class SoundOptions {
public:
    i32 musicVolume, effectVolume;
};
extern SoundOptions soundOptions;

class GameOptions {
public:
    f32 mouseSensitivity;
    bool invertMouse;
};
extern GameOptions gameOptions;

class MenuOptions {
public:
    nString newGameString, loadGameString, selectPlanetName, markerName;
    i32 markerR, markerG, markerB;
};
extern MenuOptions menuOptions;

extern void initializeOptions();