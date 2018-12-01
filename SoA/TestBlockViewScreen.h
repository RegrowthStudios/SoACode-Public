///
/// TestBlockViewScreen.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 23 Dec 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Preview a block
///

#pragma once

#ifndef TestBlockViewScreen_h__
#define TestBlockViewScreen_h__

#include <Vorb/Event.hpp>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>

#include "BlockPack.h"

class TestBlockView : public vui::IGameScreen {
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
    /// Loads a file of block data
    /// @param file: File containing block data
    void loadBlocks(const cString file);

    void genBlockMesh();

    BlockPack m_blocks; ///< Block data
    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    VGVertexBuffer m_verts;
    VGVertexBuffer m_inds;
    ui32 m_indCount;
    vg::GLProgram m_program;
    f32m4 m_mRotation;
    bool m_movingCamera;
};

#endif // TestBlockViewScreen_h__