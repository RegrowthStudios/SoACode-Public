///
/// MainMenuForm.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 12 May 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Main menu form for UI.
///

#pragma once

#ifndef MainMenuForm_h__
#define MainMenuForm_h__

#include <Vorb/ui/Form.h>

class MainMenuScreen;

class MainMenuForm : public vui::Form {
public:
    MainMenuForm();
    ~MainMenuForm();

    void init(MainMenuScreen* ownerScreen, ui32v4 destRect, vg::SpriteFont* defaultFont = nullptr, vg::SpriteBatch* spriteBatch = nullptr);

    virtual bool registerCallback(vui::Widget* w, nString callback) override;

private:
    MainMenuScreen* m_ownerScreen = nullptr;
};

#endif // MainMenuForm_h__
