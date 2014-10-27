#pragma once

#ifndef App_h_
#define App_h_

#include "MainGame.h"

class InitScreen;
class LoadScreen;
class MainMenuScreen;
class GamePlayScreen;
class FrameBuffer;
class MeshManager;
class TexturePackLoader;

class App : public MainGame {
public:
    virtual ~App();

    virtual void addScreens();
    virtual void onInit();
    virtual void onExit();

    /// Draws the frambuffer to the screen with motion blur or
    /// HDR rendering
    void drawFrameBuffer(const f32m4& VP) const;

    // Accessible Pointers To Screens
    InitScreen* scrInit;
    LoadScreen* scrLoad;
    MainMenuScreen* scrMainMenu;
    GamePlayScreen* scrGamePlay;

    FrameBuffer* frameBuffer;
    MeshManager* meshManager; ///< Stores chunk, terrain, particle, and physics block meshes
};

#endif // App_h_