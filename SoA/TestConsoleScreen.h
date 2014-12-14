///
/// TestConsoleScreen.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 14 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Tests out the lua dev console
///

#pragma once

#ifndef TestConsoleScreen_h__
#define TestConsoleScreen_h__

#include <Events.hpp>
#include <IGameScreen.h>
#include <LuaDevConsole.h>

class TestConsoleScreen : public IGameScreen {
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
    vui::LuaDevConsole m_console; ///< Console used for testing
    AutoDelegatePool m_delegatePool; ///< Input hooks reservoir
    nString m_command; ///< Current command string
};

#endif // TestConsoleScreen_h__