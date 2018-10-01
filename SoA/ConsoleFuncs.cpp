#include "stdafx.h"
#include "ConsoleFuncs.h"

#include <Vorb/script/Environment.h>

#include "DLLAPI.h"
#include "SoAState.h"
#include "SoaController.h"
#include "SoaEngine.h"
#include "ConsoleTests.h"

#include <chrono>

void runScript(vscript::Environment* env, const cString file) {
    env->load(file);
}

#ifdef _WINDOWS
int loadDLL(const cString name) {
    HINSTANCE dll = LoadLibrary(name);
    int(*f)() = (int(*)())GetProcAddress(dll, "getCode");
    fflush(stderr);
    fflush(stdout);
    return f();
}
#endif//_WINDOWS

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

void registerFuncs(vscript::Environment& env) {
    env.setNamespaces("SC");

    /************************************************************************/
    /* Application methods                                                  */
    /************************************************************************/
    env.addValue("env", &env);
    env.addCDelegate("run", makeDelegate(runScript));
#ifdef _WINDOWS
    env.addCRDelegate("loadDLL", makeRDelegate(loadDLL));
#endif//_WINDOWS

    /************************************************************************/
    /* Game-specific methods                                                */
    /************************************************************************/
    env.addCRDelegate("createState", makeRDelegate(create<SoaState>));
    env.addCDelegate("freeState", makeDelegate(free<SoaState>));
    env.addCRDelegate("createController", makeRDelegate(create<SoaController>));
    env.addCDelegate("freeController", makeDelegate(free<SoaController>));
    env.addCDelegate("initState", makeDelegate(initState));
    env.addCRDelegate("startGame", makeRDelegate(startGame));
    env.addCDelegate("stopGame", makeDelegate(stopGame));
    env.addCDelegate("setStartingPlanet", makeDelegate(setStartingPlanet));

    /************************************************************************/
    /* Test methods                                                         */
    /************************************************************************/
    env.setNamespaces("CAS");
    env.addCRDelegate("create", makeRDelegate(createCASData));
    env.addCDelegate("run", makeDelegate(runCAS));
    env.addCDelegate("free", makeDelegate(freeCAS));

    env.setNamespaces("CHS");
    env.addCDelegate("run", makeDelegate(runCHS));

    env.setNamespaces();
}
