///
/// CommonState.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 4 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Container for states and controllers passed between screens.
///

#pragma once

#ifndef CommonState_h__
#define CommonState_h__

#include "LoadContext.h"
#include <Vorb/VorbPreDecl.inl>

struct SoaState;
DECL_VSOUND(class Engine)
DECL_VUI(class GameWindow)

struct CommonState {
public:
    SoaState* state = nullptr;
    vsound::Engine* soundEngine = nullptr;
    vui::GameWindow* window = nullptr;
    LoadContext loadContext;
};

#endif // CommonState_h__
