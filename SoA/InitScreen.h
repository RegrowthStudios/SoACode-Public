#pragma once
#include "IGameScreen.h"

class App;
class SpriteBatch;
class SpriteFont;

class InitScreen : public IAppScreen<App> {
public:
    CTOR_APP_SCREEN_INL(InitScreen, App) {
    }

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
    void buildSpriteResources();
    void destroySpriteResources();

    void checkRequirements();

    SpriteBatch* _sb;
    SpriteFont* _font;
    bool _canContinue;
};
