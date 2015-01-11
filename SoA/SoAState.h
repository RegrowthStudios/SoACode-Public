///
/// SoaState.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// The main game state for SoA
///

#pragma once

#ifndef SoAState_h__
#define SoAState_h__

#include "SpaceSystem.h"
#include "GameSystem.h"

#include <Vorb/IOManager.h>
#include <Vorb/Entity.h>

class DebugRenderer;
class MeshManager;

class SoaState {
public:
    ~SoaState() {};

    SpaceSystem spaceSystem;
    GameSystem gameSystem;

    vcore::Entity playerEntity;

    std::unique_ptr<vg::GLProgramManager> glProgramManager = nullptr;
    std::unique_ptr<DebugRenderer> debugRenderer = nullptr;
    std::unique_ptr<MeshManager> meshManager = nullptr;

    vio::IOManager saveFileIom;
    bool isNewGame = true;
    f32v3 startGridPos = f32v3(0.0f);
    int startFace = 0;
};

#endif // SoAState_h__
