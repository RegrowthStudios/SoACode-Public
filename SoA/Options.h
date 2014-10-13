#pragma once
#include "compat.h"

#include "Errors.h"


extern std::vector<ui32v2> SCREEN_RESOLUTIONS;

struct GraphicsOptions {
    i32 cloudDetail, lodDistance, lodDetail, isFancyTrees, enableParticles, chunkLoadTime;
    i32 voxelRenderDistance, screenWidth, screenHeight, windowWidth, windowHeight, nativeWidth, nativeHeight, hudMode;
    i32 currTextureRes, defaultTextureRes;
    i32 motionBlur;
    i32 msaa, maxMsaa;
    f32 specularExponent, specularIntensity, lookSensitivity;
    f32 hdrExposure, gamma;
    f32 secColorMult, fov;
    i32 maxFPS;
    f32 voxelLODThreshold, voxelLODThreshold2;
    bool isFullscreen, isBorderless, isVsync, needsWindowReload, needsFboReload, needsFullscreenToggle;
    nString texturePackString, currTexturePack, defaultTexturePack;
};
extern GraphicsOptions graphicsOptions;

struct SoundOptions {
    i32 musicVolume, effectVolume;
};
extern SoundOptions soundOptions;

struct GameOptions {
    f32 mouseSensitivity;
    bool invertMouse;
};
extern GameOptions gameOptions;

struct MenuOptions {
    nString newGameString, loadGameString, selectPlanetName, markerName;
    i32 markerR, markerG, markerB;
};
extern MenuOptions menuOptions;

void initializeOptions();
int loadOptions();
int saveOptions();