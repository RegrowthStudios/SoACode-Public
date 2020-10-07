#include <Vorb/VorbPreDecl.inl>
#include <Vorb/mod/ModEnvironment.h>
#include <Vorb/ui/IGameScreen.h>

#include <iostream>

#include "CommonState.h"

DECL_VSCRIPT_LUA(class Environment)

class TestModScreen : public vui::IAppScreen<App> {
public:
    TestModScreen(const App* app, CommonState* state);

    i32 getNextScreen() const override;
    i32 getPreviousScreen() const override;

    void build() override;
    void destroy(const vui::GameTime&) override;

    void onEntry(const vui::GameTime&) override;
    void onExit(const vui::GameTime&) override;

    void update(const vui::GameTime&) override;
    void draw(const vui::GameTime&) override;
private:
    CommonState* m_commonState;

    vmod::ModEnvironment<vscript::lua::Environment>* m_modEnv;
};
