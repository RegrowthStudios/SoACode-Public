//
// TestNewBlockAPIScreen.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 26 May 2015
// Copyright 2014 Regrowth Studios
// MIT License
//
// Summary:
// 
//

#pragma once

#ifndef TestNewBlockAPIScreen_h__
#define TestNewBlockAPIScreen_h__

#include <Vorb/ui/IGameScreen.h>

class TestNewBlockAPIScreen : public vui::IGameScreen {
public:
    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;
    virtual void build() override;
    virtual void destroy(const vui::GameTime& gameTime) override;
    virtual void onEntry(const vui::GameTime& gameTime) override;
    virtual void onExit(const vui::GameTime& gameTime) override;
    virtual void update(const vui::GameTime& gameTime) override;
    virtual void draw(const vui::GameTime& gameTime) override;
};

#endif // TestNewBlockAPIScreen_h__
