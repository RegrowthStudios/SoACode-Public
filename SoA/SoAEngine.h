///
/// SoAEngine.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef SoAEngine_h__
#define SoAEngine_h__

class GameSystem;
class SpaceSystem;
class SoAState;

#pragma once
class SoAEngine {
public:
    struct SpaceSystemLoadData {
        SpaceSystem* spaceSystem;
        // More stuff here
    };
    struct GameSystemLoadData {
        GameSystem* spaceSystem;
        // More stuff here
    };

    static bool loadSpaceSystem(OUT SoAState* state, const SpaceSystemLoadData& loadData);

    static bool loadGameSystem(OUT SoAState* state, const GameSystemLoadData& loadData);

    static void destroyAll(OUT SoAState* state);

    static void destroyGameSystem(OUT SoAState* state);

private:
    static void destroySpaceSystem(OUT SoAState* state);
};

#endif // SoAEngine_h__