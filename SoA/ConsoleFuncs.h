//
// ConsoleFuncs.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 30 Jun 2015
// Copyright 2014 Regrowth Studios
// MIT License
//
// Summary:
// 
//

#pragma once

#ifndef ConsoleFuncs_h__
#define ConsoleFuncs_h__

#include <Vorb/types.h>
#include <Vorb/script/IEnvironment.hpp>

#include "DLLAPI.h"
#include "SoAState.h"
#include "SoaController.h"
#include "SoaEngine.h"
#include "ConsoleTests.h"

#include <chrono>

template <typename ScriptImpl>
void runScript(vscript::IEnvironment<ScriptImpl>* env, const cString file) {
    env->run(nString(file));
}

#ifdef VORB_OS_WINDOWS
int loadDLL(const cString name) {
    HINSTANCE dll = LoadLibrary(name);
    int(*f)() = (int(*)())GetProcAddress(dll, "getCode");
    fflush(stderr);
    fflush(stdout);
    return f();
}
#endif//VORB_OS_WINDOWS

template<typename T>
T* create() {
    T* v = new T();
    return v;
}
template<typename T>
void free(T* v) {
    delete v;
}

void initState(SoaState* s, const cString spaceSystemDir) {
    SoaEngine::initState(s);
    SoaEngine::loadSpaceSystem(s, spaceSystemDir);
}

std::thread* startGame(SoaState* s, SoaController* c) {
    std::thread* t = new std::thread([=] () {
        c->startGame(s);
        while (true) {
//            Sleep(100);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return;
    });
    return t;
}
void stopGame(std::thread* t) {
    bool isRunning = true;
    std::thread printer([&] () {
        puts("");
        size_t i = 2;
        char buf[] = { '\r', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ' , '\0' };
        while (isRunning) {
            puts(buf);
            i++;
            buf[i] = ' ';
            i %= 8;
            buf[i + 1] = '.';
//            Sleep(250);
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    });
    t->join();
    isRunning = false;
    delete t;
}

void setStartingPlanet(SoaState* s, vecs::EntityID eID) {
    s->clientState.startingPlanet = eID;
}

template <typename ScriptImpl>
void registerFuncs(vscript::IEnvironment<ScriptImpl>* env) {
    env->setNamespaces("SC");

    /************************************************************************/
    /* Application methods                                                  */
    /************************************************************************/
    env->addValue("env", env);
    env->addCDelegate("run", makeDelegate(runScript<ScriptImpl>));
#ifdef VORB_OS_WINDOWS
    env->addCDelegate("loadDLL", makeRDelegate(loadDLL));
#endif//VORB_OS_WINDOWS

    /************************************************************************/
    /* Game-specific methods                                                */
    /************************************************************************/
    env->addCDelegate("createState",       makeDelegate(create<SoaState>));
    env->addCDelegate("freeState",         makeDelegate(free<SoaState>));
    env->addCDelegate("createController",  makeDelegate(create<SoaController>));
    env->addCDelegate("freeController",    makeDelegate(free<SoaController>));
    env->addCDelegate("initState",         makeDelegate(initState));
    env->addCDelegate("startGame",         makeDelegate(startGame));
    env->addCDelegate("stopGame",          makeDelegate(stopGame));
    env->addCDelegate("setStartingPlanet", makeDelegate(setStartingPlanet));

    /************************************************************************/
    /* Test methods                                                         */
    /************************************************************************/
    env->setNamespaces("CAS");
    env->addCDelegate("create", makeDelegate(createCASData));
    env->addCDelegate("run",    makeDelegate(runCAS));
    env->addCDelegate("free",   makeDelegate(freeCAS));

    env->setNamespaces("CHS");
    env->addCDelegate("run", makeDelegate(runCHS));

    env->setNamespaces();
}

#endif // ConsoleFuncs_h__
