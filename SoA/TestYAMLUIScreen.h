#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/graphics/TextureCache.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/ui/widgets/Viewport.h>

#include "CommonState.h"

class InputMapper;

class TestYAMLUIScreen : public vui::IAppScreen<App> {
public:
    TestYAMLUIScreen(const App* app, CommonState* state);

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

    vg::SpriteBatch m_sb;
    vg::SpriteFont  m_font;

    vui::Viewport   m_viewport;
    
    vio::IOManager m_iom;

    vg::TextureCache m_textureCache;
};
