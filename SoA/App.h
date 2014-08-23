#pragma once
#include "MainGame.h"

class App : public MainGame {
    virtual void addScreens();
    virtual void onInit();
    virtual void onExit();
};