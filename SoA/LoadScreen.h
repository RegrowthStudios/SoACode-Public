#pragma once

#include <RPC.h>
#include <IGameScreen.h>
#include <Random.h>

#include "LoadMonitor.h"
#include "LoadBar.h"

class App;
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
    void addLoadTask(const nString& name, const cString loadText, ILoadTask* task);

    // Visualization Of Loading Tasks
    std::vector<LoadBar> _loadBars;
    SpriteBatch* _sb;
    SpriteFont* _sf;

    // Loading Tasks
    LoadMonitor _monitor;
    std::vector<ILoadTask*> _loadTasks;

    vcore::RPCManager m_glrpc; ///< Handles cross-thread OpenGL calls
};