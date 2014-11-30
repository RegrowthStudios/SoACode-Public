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

    fileManager.initialize();
}

int loadOptions() {
    std::vector <std::vector <IniValue> > iniValues;
    std::vector <nString> iniSections;
    if (fileManager.loadIniFile("Data/options.ini", iniValues, iniSections)) return 1;

    int iVal;
    IniValue *iniVal;
    for (size_t i = 0; i < iniSections.size(); i++) {
        for (size_t j = 0; j < iniValues[i].size(); j++) {
            iniVal = &(iniValues[i][j]);

            iVal = fileManager.getIniVal(iniVal->key);
            switch (iVal) {
            case INI_ATMOSPHERE_SEC_COLOR_EXPOSURE:
                graphicsOptions.secColorMult = iniVal->getFloat(); break;
            case INI_EFFECT_VOLUME:
                soundOptions.effectVolume = iniVal->getInt(); break;
            case INI_ENABLE_PARTICLES:
                graphicsOptions.enableParticles = iniVal->getInt(); break;
            case INI_FOV:
                graphicsOptions.fov = iniVal->getFloat(); break;
            case INI_GAMMA:
                graphicsOptions.gamma = iniVal->getFloat(); break;
            case INI_MOUSESENSITIVITY:
                gameOptions.mouseSensitivity = iniVal->getFloat(); break;
            case INI_MUSIC_VOLUME:
                soundOptions.musicVolume = iniVal->getInt(); break;
            case INI_RENDER_DISTANCE:
                graphicsOptions.voxelRenderDistance = iniVal->getInt(); break;
            case INI_TERRAIN_QUALITY:
                graphicsOptions.lodDetail = iniVal->getInt(); break;
            case INI_TEXTUREPACK:
                graphicsOptions.texturePackString = iniVal->getStr(); break;
            case INI_INVERTMOUSE:
                gameOptions.invertMouse = iniVal->getBool(); break;
            case INI_MAXFPS:
                graphicsOptions.maxFPS = iniVal->getFloat(); break;
            case INI_VSYNC:
                graphicsOptions.isVsync = iniVal->getBool(); break;
            case INI_MOTIONBLUR:
                graphicsOptions.motionBlur = iniVal->getInt(); break;
            case INI_MSAA:
                graphicsOptions.msaa = iniVal->getInt(); break;
            default:
                break;
            }
        }
    }
    return 0;
}

int saveOptions() {
    std::vector <std::vector <IniValue> > iniValues;
    std::vector <nString> iniSections;

    iniSections.push_back("");
    iniValues.push_back(std::vector<IniValue>());

    iniSections.push_back("GraphicsOptions");
    iniValues.push_back(std::vector<IniValue>());

    iniValues.back().push_back(IniValue("atmosphereSecColorExposure", graphicsOptions.secColorMult));
    iniValues.back().push_back(IniValue("enableParticles", std::to_string(graphicsOptions.enableParticles)));
    iniValues.back().push_back(IniValue("fov", graphicsOptions.fov));
    iniValues.back().push_back(IniValue("gamma", graphicsOptions.gamma));
    iniValues.back().push_back(IniValue("renderDistance", graphicsOptions.voxelRenderDistance));
    iniValues.back().push_back(IniValue("terrainQuality", graphicsOptions.lodDetail));
    iniValues.back().push_back(IniValue("texturePack", graphicsOptions.texturePackString));
    iniValues.back().push_back(IniValue("maxFps", graphicsOptions.maxFPS));
    iniValues.back().push_back(IniValue("motionBlur", graphicsOptions.motionBlur));
    iniValues.back().push_back(IniValue("msaa", graphicsOptions.msaa));
    iniValues.back().push_back(IniValue("vSync", std::to_string(graphicsOptions.isVsync)));

    iniSections.push_back("GameOptions");
    iniValues.push_back(std::vector<IniValue>());
    iniValues.back().push_back(IniValue("invertMouse", std::to_string(gameOptions.invertMouse)));
    iniValues.back().push_back(IniValue("mouseSensitivity", gameOptions.mouseSensitivity));

    iniSections.push_back("SoundOptions");
    iniValues.push_back(std::vector<IniValue>());
    iniValues.back().push_back(IniValue("effectVolume", std::to_string(soundOptions.effectVolume)));
    iniValues.back().push_back(IniValue("musicVolume", std::to_string(soundOptions.musicVolume)));

    if (fileManager.saveIniFile("Data/options.ini", iniValues, iniSections)) return 1;
    return 0;
}