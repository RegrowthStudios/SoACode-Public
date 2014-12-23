///
/// TestBlockViewScreen.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 23 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Preview a block
///

#pragma once

#ifndef TestBlockViewScreen_h__
#define TestBlockViewScreen_h__

#include <Events.hpp>
#include <IGameScreen.h>

class TestBlockView : public IGameScreen {
public:
    /************************************************************************/
    /* IGameScreen functionality                                            */
    /************************************************************************/
    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;
    virtual void build() override;
    virtual void destroy(const GameTime& gameTime) override;
    virtual void onEntry(const GameTime& gameTime) override;
    virtual void onExit(const GameTime& gameTime) override;
    virtual void onEvent(const SDL_Event& e) override;
    virtual void update(const GameTime& gameTime) override;
    virtual void draw(const GameTime& gameTime) override;
private:
    /// Loads a file of block data
    /// @param file: File containing block data
    void loadBlocks(const cString file);
};

#endif // TestBlockViewScreen_h__