#include "stdafx.h"
#include "Options.h"

#include <SDL/SDL.h>

#include "FileSystem.h"
#include "GameManager.h"
#include "InputManager.h"

std::vector<ui32v2> SCREEN_RESOLUTIONS = std::vector<ui32v2>();

GraphicsOptions graphicsOptions;
SoundOptions soundOptions;
GameOptions gameOptions;
MenuOptions menuOptions;

void initializeOptions() {
    graphicsOptions.cloudDetail = 1;
    graphicsOptions.lookSensitivity = 0.0026;
    graphicsOptions.voxelRenderDistance = 144;
    graphicsOptions.lodDetail = 1;
    graphicsOptions.isFancyTrees = 1;
    graphicsOptions.enableParticles = 1;
    graphicsOptions.chunkLoadTime = 1;
    graphicsOptions.hdrExposure = 3.0f;
    graphicsOptions.gamma = 1.0f;
    graphicsOptions.secColorMult = 0.1f;
    graphicsOptions.isVsync = 0;
    graphicsOptions.needsWindowReload = 0;
    graphicsOptions.fov = 70;
    graphicsOptions.hudMode = 0;
    graphicsOptions.texturePackString = "Default";
    graphicsOptions.defaultTexturePack = "Default.zip";
    graphicsOptions.currTexturePack = graphicsOptions.texturePackString;
    graphicsOptions.needsFboReload = 0;
    graphicsOptions.needsFullscreenToggle = 0;
    graphicsOptions.maxFPS = 60.0f;
    graphicsOptions.motionBlur = 8;
    graphicsOptions.msaa = 0;
    graphicsOptions.maxMsaa = 32;
    graphicsOptions.voxelLODThreshold = 128.0f;
    graphicsOptions.voxelLODThreshold2 = graphicsOptions.voxelLODThreshold * graphicsOptions.voxelLODThreshold;

    gameOptions.invertMouse = 0;
    gameOptions.mouseSensitivity = 30.0;

    graphicsOptions.specularExponent = 8.0f;
    graphicsOptions.specularIntensity = 0.3f;

    soundOptions.effectVolume = 100;
    soundOptions.musicVolume = 100;

    menuOptions.markerR = menuOptions.markerG = menuOptions.markerB = 0;
    menuOptions.loadGameString = menuOptions.newGameString = menuOptions.selectPlanetName = menuOptions.markerName = "";
}