#pragma once

#ifndef App_h_
#define App_h_

#include <Vorb/ui/MainGame.h>
#include "SoaOptions.h"
#include "CommonState.h"

class DevScreen;
class GameplayLoadScreen;
class GameplayScreen;
class InitScreen;
class MainMenuLoadScreen;
class MainMenuScreen;
class TexturePackLoader;

class App : public vui::MainGame {
public:
    virtual ~App();

    virtual void addScreens();
    virtual void onInit();
    virtual void onExit();

    // Accessible Pointers To Screens
    InitScreen* scrInit = nullptr;
    MainMenuLoadScreen* scrLoad = nullptr;
    MainMenuScreen* scrMainMenu = nullptr;
    GameplayLoadScreen* scrGameplayLoad = nullptr;
    GameplayScreen* scrGamePlay = nullptr;

    DevScreen* scrDev = nullptr;
    std::vector<vui::IGameScreen*> scrTests;

    CommonState state;
};

#endif // App_h_
