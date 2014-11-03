#pragma once
#include <SDL/SDL.h>

#if defined(WIN32) || defined(WIN64)
#include <SDL/SDL_syswm.h>
#endif

#include "Keg.h"

#define DEFAULT_TITLE "Seed Of Andromeda"
#define DEFAULT_WINDOW_WIDTH 1600
#define DEFAULT_WINDOW_HEIGHT 900
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
    USE_VALUE_CAP = 4
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

class GameWindow {
public:
    GameWindow();

    // Attempts To Load Application Settings Creates The Window And OpenGL Context
    bool init();
    // Saves Application Settings (If Changed) And Destroys The Window And OpenGL Context
    void dispose();

    operator SDL_Window*() const {
        return _window;
    }

    // Access Display Settings
    const i32& getWidth() const {
        return _displayMode.screenWidth;
    }
    const i32& getHeight() const {
        return _displayMode.screenHeight;
    }
    ui32v2 getViewportDims() const {
        return ui32v2(_displayMode.screenWidth, _displayMode.screenHeight);
    }
    float getAspectRatio() const {
        return (float)_displayMode.screenWidth / (float)_displayMode.screenHeight;
    }
    const bool& isFullscreen() const {
        return _displayMode.isFullscreen;
    }
    const bool& isBorderless() const {
        return _displayMode.isFullscreen;
    }
    const GameSwapInterval& getSwapInterval() const {
        return _displayMode.swapInterval;
    }
    const f32& getMaxFPS() const {
        return _displayMode.maxFPS;
    }
    const SDL_GLContext& getGLContext() const {
        return _glc;
    }


    // Change Display Settings
    void setScreenSize(const i32& w, const i32& h, const bool& overrideCheck = false);
    void setFullscreen(const bool& useFullscreen, const bool& overrideCheck = false);
    void setBorderless(const bool& useBorderless, const bool& overrideCheck = false);
    void setSwapInterval(const GameSwapInterval& mode, const bool& overrideCheck = false);
    void setMaxFPS(const f32& fpsLimit);
    void setTitle(const cString title) const;

    void sync(ui32 frameTime);
private:
    // Application Setting Management
    static void setDefaultSettings(GameDisplayMode* mode);
    void readSettings();
    void saveSettings() const;

    // Window Handle
    SDL_Window* _window;

    // OpenGL Context
    SDL_GLContext _glc;

    // Display Settings
    GameDisplayMode _displayMode;

};