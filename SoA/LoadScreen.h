#pragma once
#include "IGameScreen.h"

#include "Random.h"
#include "LoadMonitor.h"

class App;
class LoadBar;
class SpriteBatch;
class SpriteFont;

class LoadScreen : public IAppScreen<App> {
public:
    CTOR_APP_SCREEN_DECL(LoadScreen, App);

    virtual i32 getNextScreen() const;
    virtual i32 getPreviousScreen() const;

    virtual void build();
    virtual void destroy(const GameTime& gameTime);

    virtual void onEntry(const GameTime& gameTime);
    virtual void onExit(const GameTime& gameTime);

    virtual void onEvent(const SDL_Event& e);
    virtual void update(const GameTime& gameTime);
    virtual void draw(const GameTime& gameTime);
private:
    // Visualization Of Loading Tasks
    LoadBar* _loadBars;
    SpriteBatch* _sb;
    SpriteFont* _sf;

    // Loading Tasks
    LoadMonitor _monitor;
    std::vector<ILoadTask*> _loadTasks;
};