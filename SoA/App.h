#pragma once

#ifndef App_h_
#define App_h_

#include "MainGame.h"

class DevScreen;
class InitScreen;
class LoadScreen;
class MainMenuScreen;
class GamePlayScreen;
class MeshManager;
class TexturePackLoader;
class SpaceSystem;
class StarSystemScreen;

class App : public MainGame {
public:
    virtual ~App();

    virtual void addScreens();
    virtual void onInit();
    virtual void onExit();

    // Accessible Pointers To Screens
    InitScreen* scrInit;
    LoadScreen* scrLoad;
    MainMenuScreen* scrMainMenu;
    GamePlayScreen* scrGamePlay;
    StarSystemScreen* scrStarSystem;


    SpaceSystem* spaceSystem; ///< Space ECS
    DevScreen* scrDev;
    std::vector<IGameScreen*> scrTests;

    MeshManager* meshManager; ///< Stores chunk, terrain, particle, and physics block meshes
};

#endif // App_h_