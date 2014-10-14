#pragma once
#include <SDL/SDL.h>

#include "GraphicsDevice.h"
#include "Keg.h"

class FrameBuffer;
class IGameScreen;
class ScreenList;

#define DEFAULT_TITLE "SDL PROGRAM"
#define DEFAULT_WINDOW_WIDTH 600
#define DEFAULT_WINDOW_HEIGHT 480
#define DEFAULT_WINDOW_FLAGS (SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)
#define DEFAULT_SWAP_INTERVAL GameSwapInterval::V_SYNC
#define DEFAULT_MAX_FPS 60.0f
#define DEFAULT_APP_CONFIG_FILE "app.config"

// Different Kinds Of Swap Intervals Available
enum class GameSwapInterval : i32 {
    UNLIMITED_FPS = 0,
    V_SYNC = 1,
    LOW_SYNC = 2,
    POWER_SAVER = 3,
    USE_VALUE_CAP = -1
};
KEG_ENUM_DECL(GameSwapInterval);

// The Current Displaying Mode
struct GameDisplayMode {
public:
    // Screen Buffer Parameters
    i32 screenWidth;
    i32 screenHeight;

    // Window Settings
    bool isFullscreen;
    bool isBorderless;

    // Frame Rate Options
    GameSwapInterval swapInterval;
    f32 maxFPS;
};
KEG_TYPE_DECL(GameDisplayMode);

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

    SDL_Window* getWindowHandle() const {
        return _window;
    }
    SDL_GLContext getGLContext() const {
        return _glc;
    }
    #if defined(WIN32) || defined(WIN64)
    HGLRC getGLRHandle() const {
        return _hndGLRC;
    }
    #endif
    FrameBuffer* getFrameBuffer() const {
        return _frameBuffer;
    }

    // This Will Poll SDL For A Newer State
    // @param displayMode: Pointer To Where Result Is Stored
    void getDisplayMode(GameDisplayMode* displayMode);
    // Sets A New Display Mode And Attempts To Make Minimal Changes
    void setDisplayMode(const GameDisplayMode& displayMode);

    void setWindowTitle(const cString title);

    void run();
    void exitGame();

    // The Method Where IGameScreens Must Be Added To _screenList
    virtual void addScreens() = 0;
    // Initialization Logic When Application Starts Up
    virtual void onInit() = 0;
    // Called When The Application Is Going To Close
    virtual void onExit() = 0;
protected:
    // Initializes Necessary Children Systems (OpenGL, TTF, etc.)
    void init();
    void initSystems();

    void refreshElapsedTime();
    void checkInput();
    void onUpdateFrame();
    void onRenderFrame();

    // Application Setting Management
    static void setDefaultSettings(GameDisplayMode* mode);
    void readSettings();
    void saveSettings();

    GraphicsDevice* _gDevice;
    GameDisplayMode _displayMode;

    SDL_Window* _window;
    SDL_GLContext _glc;
    #if defined(WIN32) || defined(WIN64)
    HGLRC _hndGLRC;
    #endif
    FrameBuffer* _frameBuffer;

    ui32 _lastMS;
    GameTime _curTime, _lastTime;

    bool _isRunning;
    ScreenList* _screenList;
    IGameScreen* _screen;
};