///
/// MainMenuScriptedUI.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 13 May 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Custom main menu UI
///

#pragma once

#ifndef MainMenuScriptedUI_h__
#define MainMenuScriptedUI_h__

#include <Vorb/ecs/Entity.h>
#include <Vorb/script/lua/Environment.h>
#include <Vorb/ui/ScriptedUI.h>
#include "InputMapper.h"

class MainMenuScreen;

class MainMenuScriptedUI : public vui::ScriptedUI<vscript::lua::Environment> {
public:
    MainMenuScriptedUI();
    ~MainMenuScriptedUI();

    virtual void init(vui::IGameScreen* ownerScreen, const vui::GameWindow* window,
                        vg::TextureCache* textureCache, vg::SpriteFont* defaultFont = nullptr,
                            vg::SpriteBatch* spriteBatch = nullptr) override;

protected:
    virtual void prepareScriptEnv(ViewEnv* env) override;

    // LUA funcs for controls
                  size_t getNumInputs();
    InputMapper::InputID getInput(int index);
              VirtualKey getKey(InputMapper::InputID id);
              VirtualKey getDefaultKey(InputMapper::InputID id);
                 nString getKeyString(InputMapper::InputID id);
                 nString getDefaultKeyString(InputMapper::InputID id);
                 nString getName(InputMapper::InputID id);
                    void onExit(int code);
                    void onTargetChange(Sender s, vecs::EntityID id);
                    void newGame();

    // Planet functions (temporary???)
    vecs::EntityID getTargetBody();
           nString getBodyName(vecs::EntityID entity);
           nString getBodyParentName(vecs::EntityID entity);
           nString getBodyTypeName(vecs::EntityID entity);
               f32 getBodyMass(vecs::EntityID entity);
               f32 getBodyDiameter(vecs::EntityID entity);
               f32 getBodyRotPeriod(vecs::EntityID entity);
               f32 getBodyOrbPeriod(vecs::EntityID entity);
               f32 getBodyAxialTilt(vecs::EntityID entity);
               f32 getBodyEccentricity(vecs::EntityID entity);
               f32 getBodyInclination(vecs::EntityID entity);
               f32 getBodySemiMajor(vecs::EntityID entity);
               f32 getGravityAccel(vecs::EntityID entity);
               f32 getVolume(vecs::EntityID entity);
               f32 getAverageDensity(vecs::EntityID entity);

    InputMapper* m_inputMapper = nullptr;
};

#endif // MainMenuScriptedUI_h__
