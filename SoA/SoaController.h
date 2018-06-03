///
/// SoaController.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Main game controller for SoA
///

#pragma once

#ifndef SoaController_h__
#define SoaController_h__

class App;
struct SoaState;

#include <Vorb/ecs/Entity.h>

class SoaController {
public:
    virtual ~SoaController();
    void startGame(SoaState* state);

    f64v3 getEntityEyeVoxelPosition(SoaState* state, vecs::EntityID eid);
    f64v3 getEntityViewVoxelDirection(SoaState* state, vecs::EntityID eid);
};

#endif // SoaController_h__
