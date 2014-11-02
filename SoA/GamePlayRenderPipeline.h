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

#include "FrameBuffer.h"
#include "GLProgramManager.h"
#include "GameRenderParams.h"
#include "IRenderPipeline.h"

/// Forward declarations
class App;
class Camera;
class CutoutVoxelRenderStage;
class DevHudRenderStage;
class HdrRenderStage;
class IAwesomiumInterface;
class LiquidVoxelRenderStage;
class MeshManager;
class OpaqueVoxelRenderStage;
class PDA;
class PdaRenderStage;
class PlanetRenderStage;
class Player;
class SkyboxRenderStage;
class TransparentVoxelRenderStage;

class GamePlayRenderPipeline : public vg::IRenderPipeline {
public:
    GamePlayRenderPipeline();
    ~GamePlayRenderPipeline();

    /// Initializes the pipeline and passes dependencies
    /// @param viewport: The viewport to draw to.
    /// @param chunkCamera: The camera used for voxel rendering.
    /// @param worldCamera: The camera used for planet rendering.
    /// @param app: Handle to the App
    /// @param player: Handle to the Player
    /// @param meshManager: Stores all needed meshes
    /// @param pda: The PDA to render
    /// @param glProgramManager: Contains all the needed GLPrograms
    void init(const ui32v4& viewport, Camera* chunkCamera,
              const Camera* worldCamera, const App* app,
              const Player* player, const MeshManager* meshManager,
              PDA* pda, const vg::GLProgramManager* glProgramManager);

    /// Renders the pipeline
    virtual void render() override;

    /// Frees all resources
    virtual void destroy() override;

    /// Cycles the dev hud
    /// @param offset: How much to offset the current mode
    void cycleDevHud(int offset = 1);
private: 
    SkyboxRenderStage* _skyboxRenderStage = nullptr; ///< Renders the skybox
    PlanetRenderStage* _planetRenderStage = nullptr; ///< Renders the planets
    OpaqueVoxelRenderStage* _opaqueVoxelRenderStage = nullptr; ///< Renders opaque voxels
    CutoutVoxelRenderStage* _cutoutVoxelRenderStage = nullptr; ///< Renders cutout voxels
    TransparentVoxelRenderStage* _transparentVoxelRenderStage = nullptr; ///< Renders transparent voxels
    LiquidVoxelRenderStage* _liquidVoxelRenderStage = nullptr; ///< Renders liquid voxels
    DevHudRenderStage* _devHudRenderStage = nullptr; ///< renders the dev/debug HUD
    PdaRenderStage* _pdaRenderStage = nullptr;
    HdrRenderStage* _hdrRenderStage = nullptr; ///< Renders HDR post-processing

    vg::FrameBuffer* _hdrFrameBuffer = nullptr; ///< Framebuffer needed for the HDR rendering

    GameRenderParams _gameRenderParams; ///< Shared rendering parameters for voxels

    ui32v4 _viewport; ///< Viewport to draw to
    const Camera* _worldCamera = nullptr; ///< handle to world camera
};

#endif // GamePlayRenderPipeline_h__