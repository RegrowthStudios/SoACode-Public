#pragma once
#include <MainGame.h>

class TestApp : public MainGame {
    virtual void onInit();
    virtual void addScreens();
    virtual void onExit();
};