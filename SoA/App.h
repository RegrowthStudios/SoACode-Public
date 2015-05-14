#pragma once

#ifndef App_h_
#define App_h_

#include <Vorb/ui/MainGame.h>
#include "SoaOptions.h"

class DevScreen;
class InitScreen;
class LoadScreen;
class MainMenuScreen;
class GameplayScreen;
class TexturePackLoader;

class App : public vui::MainGame {
public:
    virtual ~App();

    virtual void addScreens();
    virtual void onInit();
    virtual void onExit();

    // Accessible Pointers To Screens
    InitScreen* scrInit = nullptr;
    LoadScreen* scrLoad = nullptr;
    MainMenuScreen* scrMainMenu = nullptr;
    GameplayScreen* scrGamePlay = nullptr;

    SoaOptions options;

    DevScreen* scrDev = nullptr;
    std::vector<vui::IGameScreen*> scrTests;
};

#endif // App_h_
