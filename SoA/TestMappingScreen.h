///
/// TestMappingScreen.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 14 Dec 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Render test of grid-mapped sphere
///

#pragma once

#ifndef TestMappingScreen_h__
#define TestMappingScreen_h__

#include <Vorb/ui/IGameScreen.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/gtypes.h>

class TestMappingScreen : public vui::IGameScreen {
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
    void buildGeometry();

    VGVertexBuffer m_verts; ///< Sphere's vertex buffer (of positions)
    VGIndexBuffer m_inds; ///< Sphere's index buffer
    ui32 m_indexCount; ///< Number of indices for sphere
    vg::GLProgram m_program; ///< Basic rendering program
};

#endif // TestMappingScreen_h__