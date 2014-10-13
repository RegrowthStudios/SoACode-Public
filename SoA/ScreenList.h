#pragma once
#include "MainGame.h"

class IGameScreen;

class ScreenList {
public:
    ScreenList(MainGame* g);

    IGameScreen* getCurrent() const {
        const i32 screen_size = static_cast<const i32>(_screens.size());
        if (_current < 0 || _current >= screen_size) {
            return nullptr;
        }
        else return _screens[_current];
    }
    IGameScreen* moveNext();
    IGameScreen* movePrevious();

    void setScreen(i32 s);
    ScreenList* addScreens(IGameScreen** s, i32 c = 0);
    ScreenList* addScreen(IGameScreen* s);

    void destroy(GameTime gameTime);

protected:
    MainGame* _game;

    std::vector<IGameScreen*> _screens;
    i32 _current;
};