/// 
///  GamePlayRenderPipeline.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the render pipeline for the
///  GamePlayScreen.
///
#pragma once

#ifndef GamePlayRenderPipeline_h__
#define GamePlayRenderPipeline_h__

#include "IRenderPipeline.h"
#include "GLProgramManager.h"
#include "FrameBuffer.h"
#include "GameRenderParams.h"

/// Forward declarations
class SkyboxRenderStage;
class PlanetRenderStage;
class OpaqueVoxelRenderStage;
class TransparentVoxelRenderStage;
class CutoutVoxelRenderStage;
class LiquidVoxelRenderStage;
class AwesomiumRenderStage;
class DevHudRenderStage;
class HdrRenderStage;
class Camera;
class IAwesomiumInterface;
class MeshManager;
class App;
class Player;

class GamePlayRenderPipeline : public vg::IRenderPipeline {
public:
    GamePlayRenderPipeline();
    ~GamePlayRenderPipeline();

    void init(const ui32v4& viewport, Camera* chunkCamera,
              const Camera* worldCamera, const App* app,
              const Player* player, const MeshManager* meshManager,
              const vg::GLProgramManager* glProgramManager);

    /// Renders the pipeline
    virtual void render() override;

    /// Frees all resources
    virtual void destroy() override;

    /// Cycles the dev hud
    /// @param offset: How much to offset the current mode
    void cycleDevHud(int offset = 1);
private: 
    SkyboxRenderStage* _skyboxRenderStage; ///< Renders the skybox
    PlanetRenderStage* _planetRenderStage; ///< Renders the planets
    OpaqueVoxelRenderStage* _opaqueVoxelRenderStage; ///< Renders opaque voxels
    CutoutVoxelRenderStage* _cutoutVoxelRenderStage; ///< Renders cutout voxels
    TransparentVoxelRenderStage* _transparentVoxelRenderStage; ///< Renders transparent voxels
    LiquidVoxelRenderStage* _liquidVoxelRenderStage; ///< Renders liquid voxels
    DevHudRenderStage* _devHudRenderStage; ///< renderes the dev/debug HUD
    AwesomiumRenderStage* _awesomiumRenderStage; ///< Renders the UI
    HdrRenderStage* _hdrRenderStage; ///< Renders HDR post-processing

    vg::FrameBuffer* _hdrFrameBuffer; ///< Framebuffer needed for the HDR rendering

    GameRenderParams _gameRenderParams; ///< Shared rendering parameters for voxels

    ui32v4 _viewport; ///< Viewport to draw to
    const Camera* _worldCamera; ///< handle to world camera
};

#endif // GamePlayRenderPipeline_h__