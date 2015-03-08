#pragma once
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/VorbPreDecl.inl>

class App;
DECL_VG(class SpriteBatch;  class SpriteFont);

class InitScreen : public vui::IAppScreen<App> {
public:
    CTOR_APP_SCREEN_INL(InitScreen, App) {
    }

    virtual i32 getNextScreen() const;
    virtual i32 getPreviousScreen() const;

    virtual void build();
    virtual void destroy(const vui::GameTime& gameTime);

    virtual void onEntry(const vui::GameTime& gameTime);
    virtual void onExit(const vui::GameTime& gameTime);

    virtual void update(const vui::GameTime& gameTime);
    virtual void draw(const vui::GameTime& gameTime);

private:
    void buildSpriteResources();
    void destroySpriteResources();

    // Check Requirements And Draws Results
    void checkRequirements();

    vg::SpriteBatch* _sb;
    vg::SpriteFont* _font;
    bool _canContinue;
};
