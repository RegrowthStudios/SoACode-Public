#pragma once
#include "MainGame.h"

class InitScreen;
class LoadScreen;

class App : public MainGame {
public:
    virtual void addScreens();
    virtual void onInit();
    virtual void onExit();

    // Accessible Pointers To The Screens
    InitScreen* scrInit;
    LoadScreen* scrLoad;
};