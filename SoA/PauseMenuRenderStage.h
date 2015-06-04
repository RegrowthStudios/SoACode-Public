///
/// PauseMenuRenderStage.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Render stage for drawing the pause menu
///

#pragma once

#ifndef PauseMenuRenderStage_h__
#define PauseMenuRenderStage_h__

#include <Vorb/graphics/IRenderStage.h>

class PauseMenu;

class PauseMenuRenderStage : public vg::IRenderStage {
public:
    PauseMenuRenderStage();

    void init(const PauseMenu* pauseMenu);

    // Draws the render stage
    virtual void render() override;

private:
    const PauseMenu* _pauseMenu = nullptr; ///< Handle to pause menu for rendering
};

#endif // PauseMenuRenderStage_h__