#pragma once
#include <SDL\SDL.h>

#include "GraphicsDevice.h"
#include "Options.h"

class FrameBuffer;
class IGameScreen;
class ScreenList;

#define DEFAULT_TITLE "SDL PROGRAM"
#define DEFAULT_WINDOW_WIDTH 600
#define DEFAULT_WINDOW_HEIGHT 480
#define DEFAULT_WINDOW_FLAGS (SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)
#define DEFAULT_SWAP_INTERVAL GameSwapInterval::V_SYNC

// Different Kinds Of Swap Intervals Available
enum class GameSwapInterval : i32 {
    UNLIMITED_FPS = 0,
    V_SYNC = 1,
    LOW_SYNC = 2,
    POWER_SAVER = 3
};

// The Current Displaying Mode
struct GameDisplayMode {
public:
    i32 screenWidth;
    i32 screenHeight;
    bool isFullscreen;
    bool isBorderless;
    GameSwapInterval swapInterval;
};

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
    ~MainGame();

    SDL_Window* getWindowHandle() const {
        return _window;
    }
    SDL_GLContext getGLContext() const {
        return _glc;
    }
    HGLRC getGLRHandle() const {
        return _hndGLRC;
    }
    FrameBuffer* getFrameBuffer() const {
        return _frameBuffer;
    }
    
    void getDisplayMode(GameDisplayMode* displayMode);
    void setDisplayMode(const GameDisplayMode& displayMode);

    void setWindowTitle(const cString title);

    void run();
    void exitGame();

    virtual void addScreens() = 0;
    virtual void onInit() = 0;
    virtual void onExit() = 0;
protected:
    void init();
    void refreshElapsedTime();
    void checkInput();
    void onUpdateFrame();
    void onRenderFrame();

    void initSystems();
    
    GraphicsDevice* _gDevice;
    GameDisplayMode _displayMode;
	SoundOptions _soundOptions;

    SDL_Window* _window;
    SDL_GLContext _glc;
    HGLRC _hndGLRC;
    FrameBuffer* _frameBuffer;

    ui32 _lastMS;
    GameTime _curTime, _lastTime;
    
    bool _isRunning;
    ScreenList* _screenList;
    IGameScreen* _screen;
};