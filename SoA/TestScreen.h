#pragma once
#include "IGameScreen.h"

class DevConsole;
class DevConsoleView;

class TestScreen : public IGameScreen {
public:
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
    DevConsole* _console;
    DevConsoleView* _consoleView;
};