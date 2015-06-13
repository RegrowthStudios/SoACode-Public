///
/// TestBlockViewScreen.h
/// Seed of Andromeda
///
/// Created by Frank McCoy on 7 April 2015
/// Copyright 2014-2015 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Load and display a voxel model in .QB file format
///

#pragma once

#ifndef TestVoxelModelScreen_h__
#define TestVoxelModelScreen_h__

#include <vector>

#include <Vorb/Events.hpp>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>

#include "Camera.h"
#include "VoxelModel.h"
#include "VoxelModelRenderer.h"

class VoxelMatrix;
class VoxelModelVertex;

class TestVoxelModelScreen : public vui::IGameScreen {
public:
    /************************************************************************/
    /* IGameScreen functionality                                            */
    /************************************************************************/
    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;
    virtual void build() override;
    virtual void destroy(const vui::GameTime& gameTime) override;
    virtual void onEntry(const vui::GameTime& gameTime) override;
    virtual void onExit(const vui::GameTime& gameTime) override;
    virtual void update(const vui::GameTime& gameTime) override;
    virtual void draw(const vui::GameTime& gameTime) override;
private:
    Camera* m_camera;
    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    bool m_movingCamera;

    VoxelModel* m_model;
    VoxelModelRenderer m_renderer;

    bool m_movingForward;
    bool m_movingBack;
    bool m_movingLeft;
    bool m_movingRight;
    bool m_movingUp;
    bool m_movingDown;
};

#endif