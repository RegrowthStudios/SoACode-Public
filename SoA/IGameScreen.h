#pragma once
#include "MainGame.h"

#define SCREEN_INDEX_NO_SCREEN -1
#define SCREEN_INDEX_NO_START_SELECTED -2

// A Screen Must Be In One Of These States
enum class ScreenState {
    // The Screen Is Doing Nothing At The Moment
    NONE,
    // The Screen Is Running (Current Active)
    RUNNING,
    // Exit Request To The Main Game
    EXIT_APPLICATION,
    // Request To Move To The Next Screen
    CHANGE_NEXT,
    // Request To Move To The Previous Screen
    CHANGE_PREVIOUS
};

// Common Interface For A Game Screen
class IGameScreen {
public:
    IGameScreen()
    : _state(ScreenState::NONE), _game(nullptr), _index(-1) {
        // empty
    }

    ~IGameScreen() {
        // empty
    }

    // All Screens Should Have A Parent
    void setParentGame(MainGame* game, i32 index) {
        _game = game;
        _index = index;
    }

    // The Screen's Location In The List
    i32 getIndex() const {
        return _index;
    }

    // Returns Screen Index When Called To Change Screens
    virtual i32 getNextScreen() const = 0;
    virtual i32 getPreviousScreen() const = 0;

    // Screen State Functions
    ScreenState getState() const {
        return _state;
    }
    void setRunning() {
        _state = ScreenState::RUNNING;
    }

    // Called At The Beginning And End Of The Application
    virtual void build() = 0;
    virtual void destroy(const GameTime& gameTime) = 0;

    // Called When A Screen Enters And Exits Focus
    virtual void onEntry(const GameTime& gameTime) = 0;
    virtual void onExit(const GameTime& gameTime) = 0;

    // Called In The Main Update Loop
    virtual void onEvent(const SDL_Event& e) = 0;
    virtual void update(const GameTime& gameTime) = 0;
    virtual void draw(const GameTime& gameTime) = 0;
protected:
    ScreenState _state;
    MainGame* _game;
private:
    // Location In The ScreenList
    i32 _index;
};

template<typename T>
class IAppScreen : public IGameScreen {
public:
    IAppScreen(const T* app)
        : _app(app) {
    }
protected:
    const T* const _app;
};

// Shorten Super-Constructors
#define CTOR_APP_SCREEN_INL(SCR_CLASS, APP_CLASS) SCR_CLASS(const APP_CLASS* app) : IAppScreen<APP_CLASS>(app)
#define CTOR_APP_SCREEN_DECL(SCR_CLASS, APP_CLASS) SCR_CLASS(const APP_CLASS* app)
#define CTOR_APP_SCREEN_DEF(SCR_CLASS, APP_CLASS) SCR_CLASS::SCR_CLASS(const APP_CLASS* app) : IAppScreen<APP_CLASS>(app)
