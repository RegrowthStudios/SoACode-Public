#pragma once
#include "IGameScreen.h"

#include "Random.h"
#include "Thread.h"

class FrameBuffer;
class LoadBar;
class SpriteBatch;
class SpriteFont;

class LoadScreen : public IGameScreen {
public:
    LoadScreen();

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
    void checkSystemRequirements();
    void initializeOld();

    void createFrameBuffer();
    void loadShaders();
    void loadTextures();
    void loadInputs();
    void loadBlockData();

    void changeFont();

    LoadBar* _loadBars;
    SpriteBatch* _sb;
    SpriteFont* _sf;
    ui32 _texID;

    FrameBuffer* _frameBuffer;

    Thread<LoadScreen*> _loadThreads[4];

    Random rand;
    f32 rTimeRefresh;
    std::map<nString, nString> _ffm;
};