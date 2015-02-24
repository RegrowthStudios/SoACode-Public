///
/// MTRenderStateManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 22 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Manages the updating and access of triple buffered
/// render state on the render and update threads
///

#pragma once

#ifndef MTRenderStateManager_h__
#define MTRenderStateManager_h__

#include "MTRenderState.h"
#include <mutex>

class MTRenderStateManager {
public:
    /// Gets the state for updating. Only call once per frame.
    MTRenderState* getRenderStateForUpdate();
    /// Marks state as finished updating. Only call once per frame,
    /// must call for every call to getRenderStateForUpdate.
    void finishUpdating();
    /// Gets the state for rendering. Only call once per frame.
    const MTRenderState* getRenderStateForRender();
private:
    int m_updating = 0; ///< Currently updating state
    int m_lastUpdated = 0; ///< Most recently updated state
    int m_rendering = 0; ///< Currently rendering state
    MTRenderState m_renderState[3]; ///< Triple-buffered state
    std::mutex m_lock;
};

#endif // MTRenderStateManager_h__
