///
/// DevScreen.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 14 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Allows developers to bypass normal startup into other screens
///

#pragma once

#ifndef DevScreen_h__
#define DevScreen_h__

#include <Vorb/Events.hpp>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/ui/KeyboardEventDispatcher.h>
#include <Vorb/VorbPreDecl.inl>

DECL_VG(class SpriteBatch; class SpriteFont)

class DevScreen : public vui::IGameScreen {
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

    /// Registers a screen that developers can access
    /// @param vKey: Key code used to access the screen
    /// @param s: Screen
    /// @param name: Display name
    void addScreen(VirtualKey vKey, vui::IGameScreen* s, const nString& name);
private:
    std::map<VirtualKey, vui::IGameScreen*> m_screenMapping; ///< Stores available screen targets
    std::map<VirtualKey, nString> m_screenNames; ///< Stores display names of screens
    AutoDelegatePool m_delegatePool; ///< Input hooks reservoir
    vui::IGameScreen* m_nextScreen = nullptr; ///< The next screen
    int deferCounter = 1; //< When this hits 0 we change screen
    vg::SpriteBatch* m_sb = nullptr;
    vg::SpriteFont* m_font = nullptr;
};

#endif // DevScreen_h__