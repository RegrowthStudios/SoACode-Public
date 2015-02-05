///
/// SoAEngine.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles initialization and destruction of SoAState
///

#pragma once

#ifndef SoAEngine_h__
#define SoAEngine_h__

#include <Vorb/ecs/Entity.h>
#include <Vorb/VorbPreDecl.inl>

class GameSystem;
class SoaState;
class SpaceSystem;
struct SpaceSystemLoadParams;
struct SystemBody;
struct SystemBodyKegProperties;

DECL_VCORE(class RPCManager)

#pragma once
class SoaEngine {
public:
    struct SpaceSystemLoadData {
        nString filePath;
    };
    struct GameSystemLoadData {
        // More stuff here
    };
    static bool initState(OUT SoaState* state);

    static bool loadSpaceSystem(OUT SoaState* state, const SpaceSystemLoadData& loadData, vcore::RPCManager* glrpc = nullptr);

    static bool loadGameSystem(OUT SoaState* state, const GameSystemLoadData& loadData);

    static void destroyAll(OUT SoaState* state);

    static void destroyGameSystem(OUT SoaState* state);

    static void destroySpaceSystem(OUT SoaState* state);

private:
    static void addSolarSystem(SpaceSystemLoadParams& pr);

    static bool loadSystemProperties(SpaceSystemLoadParams& pr);

    static bool loadBodyProperties(SpaceSystemLoadParams& pr, const nString& filePath,
                                   const SystemBodyKegProperties* sysProps, SystemBody* body);

    static void calculateOrbit(SpaceSystem* spaceSystem, vcore::EntityID entity, f64 parentMass, bool isBinary);

};

#endif // SoAEngine_h__
