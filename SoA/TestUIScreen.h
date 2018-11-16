#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/ui/Button.h>
#include <Vorb/ui/CheckBox.h>
#include <Vorb/ui/ComboBox.h>
#include <Vorb/ui/Label.h>
#include <Vorb/ui/Panel.h>
#include <Vorb/ui/Viewport.h>
#include <Vorb/ui/WidgetList.h>

#include "CommonState.h"

class InputMapper;

class TestUIScreen : public vui::IAppScreen<App> {
public:
    TestUIScreen(const App* app, CommonState* state);

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
    vui::Panel      m_panels[5];
    vui::CheckBox   m_checkBox;
    vui::Label      m_label;
    vui::Button     m_button;
    vui::ComboBox   m_comboBox;
    vui::WidgetList m_widgetList;
    vui::Button     m_listButtons[6];
};
