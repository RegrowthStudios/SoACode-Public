#include "stdafx.h"
#include "MTRenderStateManager.h"

// Increments i in modulo 3 for triple buffering
void incrementMod3(OUT int& i) {
    i = (i + 1) % 3;
}

MTRenderState* MTRenderStateManager::getRenderStateForUpdate() {
    std::lock_guard<std::mutex> lock(m_lock);
    // Get the next free state
    incrementMod3(m_updating);
    if (m_updating == m_rendering) {
        incrementMod3(m_updating);
    }
    return &m_renderState[m_updating];
}

void MTRenderStateManager::finishUpdating() {
    std::lock_guard<std::mutex> lock(m_lock);
    // Mark the currently updating buffer as the last updated
    m_lastUpdated = m_updating;
}

const MTRenderState* MTRenderStateManager::getRenderStateForRender() {
    std::lock_guard<std::mutex> lock(m_lock);
    // If we haven't updated again, return the same one
    if (m_rendering == m_lastUpdated) return &m_renderState[m_rendering];
    // Render the last updated state
    m_rendering = m_lastUpdated;
    return &m_renderState[m_rendering];
}
