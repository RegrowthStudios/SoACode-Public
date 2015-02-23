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
    MTRenderState* getRenderStateForUpdate();
    void finishUpdating();

    MTRenderState* getRenderStateForRender();
private:
    int m_updating = 0;
    int m_lastUpdated = 0;
    int m_rendering = 0;
    MTRenderState m_renderState[3];
    std::mutex m_lock;
};

#endif // MTRenderStateManager_h__
