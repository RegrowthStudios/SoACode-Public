#pragma once
#include "IGameScreen.h"

#include "AwesomiumInterface.h"
#include "MainMenuAPI.h"
#include "Random.h"
#include "LoadMonitor.h"

class App;
struct TerrainMeshMessage;

class MainMenuScreen : public IAppScreen<App>
{
public:
    CTOR_APP_SCREEN_DECL(MainMenuScreen, App);

    virtual i32 getNextScreen() const;
    virtual i32 getPreviousScreen() const;

    virtual void build();
    virtual void destroy(const GameTime& gameTime);

    virtual void onEntry(const GameTime& gameTime);
    virtual void onExit(const GameTime& gameTime);

    virtual void onEvent(const SDL_Event& e);
    virtual void update(const GameTime& gameTime);
    virtual void draw(const GameTime& gameTime);

    // Getters
    CinematicCamera& getCamera() { return _camera; }
    IOManager& getIOManager() { return _ioManager; }

private:

    void updateThreadFunc();
    void UpdateTerrainMesh(TerrainMeshMessage *tmm);

    vui::AwesomiumInterface<MainMenuAPI> _awesomiumInterface;
    
    MainMenuAPI _api;

    IOManager _ioManager;

    CinematicCamera _camera;

    std::thread* _updateThread;
    bool _threadRunning;
};

