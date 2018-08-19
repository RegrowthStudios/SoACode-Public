///
/// TestConsoleScreen.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 14 Dec 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Tests out the Lua dev console
///

#pragma once

#ifndef TestConsoleScreen_h__
#define TestConsoleScreen_h__

#include <Vorb/Events.hpp>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/ui/LuaDevConsole.h>
#include <Vorb/ui/TextInputListener.hpp>

class TestConsoleScreen : public vui::IGameScreen {
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
#ifdef VORB_LUA
    vui::LuaDevConsole m_console; ///< Console used for testing
#endif//VORB_LUA

    vui::TextInputListener<char> m_text; ///< Text input
    AutoDelegatePool m_delegatePool; ///< Input hooks reservoir
};

#endif // TestConsoleScreen_h__