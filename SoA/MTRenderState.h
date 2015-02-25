///
/// MTRenderState.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 22 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Render state that needs to be cloned for multi-threading
/// to prevent inaccuracies.
///

#pragma once

#ifndef MTRenderState_h__
#define MTRenderState_h__

#include <Vorb/ecs/ECS.h>
#include <map>

/// Not every bit of render state needs to be in MTRenderState. Only things
/// that are sensitive, such as fast moving planets and the camera.
struct MTRenderState {
    f64q spaceCameraOrientation; ///< Orientation in space
    f64v3 spaceCameraPos; ///< Position in space, relative to parent body
    std::map<vcore::EntityID, f64v3> spaceBodyPositions; ///< Space entity positions
};

#endif // MTRenderState_h__
