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

#include <Vorb/Events.hpp>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>

#include "BlockPack.h"

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

    void genBlockMesh();

    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    VGVertexBuffer m_verts;
    VGVertexBuffer m_inds;
    ui32 m_indCount;
    vg::GLProgram m_program;
    f32m4 m_mRotation;
    bool m_movingCamera;
};

#endif