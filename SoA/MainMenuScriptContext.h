///
/// MainMenuScriptContext.h
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

#ifndef MainMenuScriptContext_h__
#define MainMenuScriptContext_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/ecs/Entity.h>
#include <Vorb/ui/Keys.inl>
#include <Vorb/ui/UI.h>
#include "InputMapper.h"

DECL_VG(class TextureCache)
DECL_VUI(class GameWindow)
DECL_VSCRIPT_LUA(class Environment)

class MainMenuScreen;

namespace MainMenuScriptContext {
    void injectInto(vscript::lua::Environment* env, const vui::GameWindow* window, vg::TextureCache* textureCache, InputMapper* inputMapper, MainMenuScreen* ownerScreen, vui::IWidgets& widgets);

    namespace impl {
        // LUA funcs for controls
                      size_t getNumInputs(InputMapper* inputMapper);
        InputMapper::InputID getInput(InputMapper* inputMapper, int index);
                  VirtualKey getKey(InputMapper* inputMapper, InputMapper::InputID id);
                  VirtualKey getDefaultKey(InputMapper* inputMapper, InputMapper::InputID id);
                     nString getKeyString(InputMapper* inputMapper, InputMapper::InputID id);
                     nString getDefaultKeyString(InputMapper* inputMapper, InputMapper::InputID id);
                     nString getName(InputMapper* inputMapper, InputMapper::InputID id);
                        void onExit(MainMenuScreen* ownerScreen, int code);
                        void onTargetChange(Sender s, vecs::EntityID id);
                        void newGame(MainMenuScreen* ownerScreen);

        // Planet functions (temporary???)
        vecs::EntityID getTargetBody(MainMenuScreen* ownerScreen);
               nString getBodyName(MainMenuScreen* ownerScreen, vecs::EntityID entity);
               nString getBodyParentName(MainMenuScreen* ownerScreen, vecs::EntityID entity);
               nString getBodyTypeName(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getBodyMass(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getBodyDiameter(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getBodyRotPeriod(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getBodyOrbPeriod(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getBodyAxialTilt(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getBodyEccentricity(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getBodyInclination(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getBodySemiMajor(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getGravityAccel(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getVolume(MainMenuScreen* ownerScreen, vecs::EntityID entity);
                   f32 getAverageDensity(MainMenuScreen* ownerScreen, vecs::EntityID entity);
    }

}

#endif // MainMenuScriptContext_h__
