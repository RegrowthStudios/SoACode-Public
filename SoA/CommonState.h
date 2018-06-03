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
#include <Vorb/graphics/FullQuadVBO.h>

#include "SpaceSystemRenderStage.h"
#include "SkyboxRenderStage.h"
#include "HdrRenderStage.h"

struct SoaState;
DECL_VSOUND(class Engine)
DECL_VUI(class GameWindow)

struct CommonState {
public:
    SoaState* state = nullptr;
    vsound::Engine* soundEngine = nullptr;
    vui::GameWindow* window = nullptr;
    StaticLoadContext loadContext;

    struct {
        SkyboxRenderStage skybox;
        SpaceSystemRenderStage spaceSystem;
        HdrRenderStage hdr;
    } stages; // Shared render stages
    vg::FullQuadVBO quad; ///< Quad used for post-processing
};

#endif // CommonState_h__
