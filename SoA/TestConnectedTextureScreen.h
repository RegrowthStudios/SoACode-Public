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
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>

#include "BlockPack.h"

class TestConnectedTextureScreen : public vui::IGameScreen {
public:
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

    BlockPack m_blocks; ///< Block data
};

#endif // TestConnectedTextureScreen_h__