#pragma once
#include <SDL/SDL.h>

#include "GameWindow.h"
#include "GraphicsDevice.h"

class IGameScreen;
class ScreenList;


// Provides Temporal Information Since The Start Of The Application
struct GameTime {
public:
    // Total Time Since The Main Loop Started
    f64 total;
    // Elapsed Time For The Current Frame
    f64 elapsed;
};

// Encapsulates An Entry Point Into A Rendering Program
class MainGame {
public:
    MainGame();
    virtual ~MainGame();

    const GameWindow& getWindow() const {
        return _window;
    }

    void run();
    void exitGame();

    // Initialization Logic When Application Starts Up
    virtual void onInit() = 0;
    // The Method Where IGameScreens Must Be Added To _screenList
    virtual void addScreens() = 0;
    // Called When The Application Is Going To Close
    virtual void onExit() = 0;

    // Returns the FPS of the run() loop
    float getFps() const { return _fps; }
protected:
    // Initializes Necessary Children Systems (OpenGL, TTF, etc.)
    bool init();
    bool initSystems();

    // Steps Performed During Game Loop
    void refreshElapsedTime();
    void checkInput();
    void onUpdateFrame();
    void onRenderFrame();

    GraphicsDevice* _gDevice;
    GameWindow _window;

    float _fps;
    ui32 _lastMS;
    GameTime _curTime, _lastTime;

    bool _isRunning;
    ScreenList* _screenList;
    IGameScreen* _screen;
};