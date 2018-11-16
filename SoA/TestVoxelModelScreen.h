///
/// TestBlockViewScreen.h
/// Seed of Andromeda
///
/// Created by Frank McCoy on 7 April 2015
/// Copyright 2014-2015 Regrowth Studios
/// MIT License
///
/// Summary:
/// Load and display a voxel model in .QB file format
///

#pragma once

#ifndef TestVoxelModelScreen_h__
#define TestVoxelModelScreen_h__

#include <vector>

#include <Vorb/Event.hpp>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>

#include "Camera.h"
#include "VoxelModel.h"
#include "VoxelModelRenderer.h"

class App;
class VoxelMatrix;
class VoxelModelVertex;

enum class VoxelMeshType { BASIC, MARCHING_CUBES, DUAL_CONTOURING };

struct MeshDebugInfo {
    nString name;
    VoxelMeshType meshType;
    VoxelModel* model;
    ui32 numPolygons;
    f64 buildTime;
    f32 size;
};

class TestVoxelModelScreen : public vui::IAppScreen<App> {
public:
    TestVoxelModelScreen(const App* app);
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
    void addMesh(const nString& name, VoxelMeshType meshType, VoxelModel* model);
    Camera m_camera;
    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    bool m_mouseButtons[3];

    ui32 m_currentMesh = 0;
    std::vector<VoxelModelMesh> m_meshes;
    std::vector<MeshDebugInfo> m_meshInfos;
    VoxelModelRenderer m_renderer;
    bool m_wireFrame = false;

    vg::SpriteBatch m_sb;
    vg::SpriteFont m_sf;

    bool m_movingForward;
    bool m_movingBack;
    bool m_movingLeft;
    bool m_movingRight;
    bool m_movingUp;
    bool m_movingDown;
    bool m_movingFast;
};

#endif