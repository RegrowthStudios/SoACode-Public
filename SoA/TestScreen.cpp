#include "stdafx.h"
#include "TestScreen.h"

#include "DevConsole.h"
#include "DevConsoleView.h"
#include "Keg.h"

enum class KEnum {
    One,
    Two
};

struct KegTest {
public:
    KEnum value1;
    ui32 value2;
    Array< Array<i32> > value3;
    cString value4;
    void* value5;
};

KEG_TYPE_DECL(KegTest);
KEG_TYPE_INIT_BEGIN(KegTest, KegTest, t)
t->addValue("value1", Keg::Value::custom("KEnum", offsetof(KegTest, value1), true));
t->addValue("value2", Keg::Value::basic(Keg::BasicType::UI32, offsetof(KegTest, value2)));
t->addValue("value3", Keg::Value::array(offsetof(KegTest, value3), 
    Keg::Value::array(0,
        Keg::Value::basic(Keg::BasicType::I32, 0)
        )
    ));
t->addValue("value4", Keg::Value::basic(Keg::BasicType::C_STRING, offsetof(KegTest, value4)));
t->addValue("value5", Keg::Value::ptr(offsetof(KegTest, value5), 
    Keg::Value::basic(Keg::BasicType::I8_V4, 0)
    ));
KEG_TYPE_INIT_END

KEG_ENUM_DECL(KEnum);
KEG_ENUM_INIT_BEGIN(KEnum, KEnum, e)
e->addValue("One", KEnum::One);
e->addValue("Two", KEnum::Two);
KEG_ENUM_INIT_END

i32 TestScreen::getNextScreen() const {
    return -1;
}
i32 TestScreen::getPreviousScreen() const {
    return -1;
}

void TestScreen::build() {
    cString data = R"(
    value1: Two
    value2: 42452

    value3: 
        -   -   1
            -   2
            -   3
        -   -   4
            -   5
            -   6
        -   -   7
            -   8
            -   9

    value4: "Hello There\0 Evil"
    value5:
        value: [1, 2, 3, 4]
        # Alternatively, xyzw, rgba, stpq, 0123 labels could have been used :)
)";
    KegTest kt = {};
    Keg::parse(&kt, data, "KegTest");
    for (i32 i = 0; i < kt.value3.length(); i++) {
        Array<i32> oa = kt.value3[i];
        for (i32 i2 = 0; i2 < kt.value3[i].length(); i2++) {
            printf("%d\n", kt.value3[i][i2]);
        }
    }
    i8v4* kte = (i8v4*)kt.value5;
    printf("%f\n", kte->x);
}
void TestScreen::destroy(const GameTime& gameTime) {}

void TestScreen::onEntry(const GameTime& gameTime) {
    _console = new DevConsole(20);
    _consoleView = new DevConsoleView();
    _consoleView->init(_console, 10);

    _console->write("Test 1\nI'm^^^^^ a crazy monkey\n Kern iiiiIIIMLMmlm");
    _console->write("Test 2");
}
void TestScreen::onExit(const GameTime& gameTime) {
    _consoleView->dispose();
    delete _consoleView;
    delete _console;
}

void TestScreen::onEvent(const SDL_Event& e) {}
void TestScreen::update(const GameTime& gameTime) {
    _consoleView->update((f32)gameTime.elapsed);
}
void TestScreen::draw(const GameTime& gameTime) {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    GameDisplayMode gdm;
    _game->getDisplayMode(&gdm);
    _consoleView->render(f32v2(0, 0), f32v2(gdm.screenWidth, gdm.screenHeight));
}
