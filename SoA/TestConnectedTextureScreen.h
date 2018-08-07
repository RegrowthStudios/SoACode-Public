///
/// TestConnectedTextureScreen.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 2 Jul 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Test screen to help artists with connected textures
///

#pragma once

#ifndef TestConnectedTextureScreen_h__
#define TestConnectedTextureScreen_h__

#include <Vorb/Events.hpp>
#include <Vorb/graphics/GBuffer.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/RTSwapChain.hpp>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/ui/IGameScreen.h>

#include "BlockPack.h"
#include "Camera.h"
#include "Chunk.h"
#include "CommonState.h"
#include "ChunkMesher.h"
#include "ChunkRenderer.h"
#include "SsaoRenderStage.h"
#include "HdrRenderStage.h"

class TestConnectedTextureScreen : public vui::IAppScreen<App> {
public:
    TestConnectedTextureScreen(const App* app, CommonState* state);
    /************************************************************************/
    /* IGameScreen functionality                                            */
    /************************************************************************/
    i32 getNextScreen() const override;
    i32 getPreviousScreen() const override;
    void build() override;
    void destroy(const vui::GameTime& gameTime) override;
    void onEntry(const vui::GameTime& gameTime) override;
    void onExit(const vui::GameTime& gameTime) override;
    void update(const vui::GameTime& gameTime) override;
    void draw(const vui::GameTime& gameTime) override;
private:
    void initChunks();
    void initInput();
    Chunk* addChunk(const nString& name);

    struct ViewableChunk {
        Chunk* chunk;
        ChunkMesh* chunkMesh;
        nString name;
    };

    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    Camera m_camera;
    BlockPack m_blocks; ///< Block data
    CommonState* m_commonState;
    SoaState* m_soaState;
    ChunkRenderer m_renderer;
    ChunkMesher m_mesher;

    vg::SpriteBatch m_sb;
    vg::SpriteFont m_font;

    std::vector <ViewableChunk> m_chunks;

    vg::GBuffer m_hdrTarget; ///< Framebuffer needed for the HDR rendering
    vg::RTSwapChain<2> m_swapChain; ///< Swap chain of framebuffers used for post-processing

    SSAORenderStage m_ssaoStage;
    HdrRenderStage m_hdrStage;

    bool m_wireFrame = false;
    bool m_mouseButtons[2];
    bool m_movingForward = false;
    bool m_movingBack = false;
    bool m_movingLeft = false;
    bool m_movingRight = false;
    bool m_movingUp = false;
    bool m_movingDown = false;
    bool m_movingFast = false;

    int m_activeChunk = 0;
};

#endif // TestConnectedTextureScreen_h__