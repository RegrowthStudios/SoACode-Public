#define SOA_GAME
#include "stdafx.h"

#include "IOManager.h"
#include "utils.h"
#include "Errors.h"
#include <boost/filesystem/operations.hpp>

#if defined(WIN32) || defined(WIN64)
#include <SDL/SDL_syswm.h>
#endif

#include "App.h"
#include "ECS.h"
#include "ComponentTable.hpp"
#include "MultipleComponentSet.h"

// Just a precaution to make sure our types have the right size
void checkTypes();

// Creates The Environment For IO Managers
void initIOEnvironment(char** argv);

// Entry
int main(int argc, char **argv) {
    // Check that the types are right
    checkTypes();

    // Set up the IO environment
    initIOEnvironment(argv);

#if defined(WIN32) || defined(WIN64)
    // Tell windows that our priority class should be real time
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif
    {
        // Make ECS
        vcore::ECS ecs;

        // Add component tables
        vcore::ComponentTable<ui64> t1(0);
        ecs.addComponent("Health", &t1);
        vcore::ComponentTable<f32v3> t2(f32v3(0, 0, 0));
        ecs.addComponent("Position", &t2);

        // Add multi-component listeners
        vcore::MultipleComponentSet mt;
        mt.addRequirement(&t1);
        mt.addRequirement(&t2);

        // Use ECS
        auto e1 = ecs.addEntity();
        auto e2 = ecs.addEntity();
        auto e3 = ecs.addEntity();
        t1.add(e1);
        t1.add(e2);
        t1.add(e3);
        // mt has ()
        t2.add(e3);
        t2.add(e2);
        // mt has (3, 2)
        t2.remove(e3);
        t2.remove(e2);
        // mt has ()
        t2.add(e1);
        // mt has (1)
    }

    // Run the game
    MainGame* mg = new App;
    mg->run();
    delete mg;
    mg = nullptr;

#if defined(WIN32) || defined(WIN64)
    // Need to free the console on windows
    FreeConsole();
#endif

    return 0;
}

void initIOEnvironment(char** argv) {
    // Make Sure The Initial Path Is Set Correctly
    boost::filesystem::path cwP =  boost::filesystem::initial_path();

    // Set The Executable Directory
    nString execDir;
    IOManager::getDirectory(argv[0], execDir);
    IOManager::setExecutableDirectory(execDir);

    // Set The Current Working Directory
    nString cwPath, cwDir;
    convertWToMBString((cwString)boost::filesystem::system_complete(cwP).c_str(), cwPath);
    IOManager::getDirectory(cwPath.c_str(), cwDir);
    IOManager::setCurrentWorkingDirectory(cwDir);

#ifdef DEBUG
    printf("Executable Directory:\n    %s\n", IOManager::getExecutableDirectory());
    printf("Current Working Directory:\n    %s\n\n\n", IOManager::getCurrentWorkingDirectory()
        ? IOManager::getCurrentWorkingDirectory()
        : "None Specified");
#endif // DEBUG
}

void checkTypes() {
    if (sizeof(float) != 4) {
        pError("Size of float is not 4. It is " + std::to_string(sizeof(float)));
        exit(33);
    }
    if (sizeof(double) != 8) {
        pError("Size of double is not 8. It is " + std::to_string(sizeof(double)));
        exit(33);
    }
    if (sizeof(f32) != 4) {
        pError("Size of f32 is not 4. It is " + std::to_string(sizeof(f32)));
        exit(33);
    }
    if (sizeof(f64) != 8) {
        pError("Size of f64 is not 8. It is " + std::to_string(sizeof(f64)));
        exit(33);
    }
    if (sizeof(i32) != 4) {
        pError("Size of i32 is not 4. It is " + std::to_string(sizeof(i32)));
        exit(33);
    }
    if (sizeof(i64) != 8) {
        pError("Size of i64 is not 8. It is " + std::to_string(sizeof(i64)));
        exit(33);
    }
    if (sizeof(f32) != 4) {
        pError("Size of f32 is not 4. It is " + std::to_string(sizeof(f32)));
        exit(33);
    }
    if (sizeof(i32v3) != 12) {
        pError("Size of i32v3 is not 12. It is " + std::to_string(sizeof(i32v3)));
        exit(33);
    }
    if (sizeof(f32v3) != 12) {
        pError("Size of f32v3 is not 12. It is " + std::to_string(sizeof(f32v3)));
        exit(33);
    }
    if (sizeof(i64v3) != 24) {
        pError("Size of i64v3 is not 24. It is " + std::to_string(sizeof(i64v3)));
        exit(33);
    }
    if (sizeof(f64v3) != 24) {
        pError("Size of f64v3 is not 24. It is " + std::to_string(sizeof(f64v3)));
        exit(33);
    }
    if (sizeof(i16v3) != 6) {
        pError("Size of i16v3 is not 6. It is " + std::to_string(sizeof(i16v3)));
        exit(33);
    }
    if (sizeof(i8v3) != 3) {
        pError("Size of i8v3 is not 3. It is " + std::to_string(sizeof(i8v3)));
        exit(33);
    }
}