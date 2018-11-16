///
/// OptionsController.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 13 May 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Handles changing of options.
///

#pragma once

#ifndef OptionsController_h__
#define OptionsController_h__

#include "SoaOptions.h"
#include <Vorb/Event.hpp>
// #include <Vorb/script/Environment.h>

class OptionsController {
public:
    OptionsController(const nString& filePath = "Data/options.ini");
    ~OptionsController();

    // Call right before loading options
    void setDefault();

    /// Begins a context for changing options.
    /// Call this when beginning to change options.
    void beginContext();

    bool loadOptions();

    void saveOptions();

    void restoreDefault();

    // void registerScripting(vscript::Environment* env);

    // These can be called from lua scripts
    void setInt(nString optionName, int val);
    void setFloat(nString optionName, f32 val);
    void setBool(nString optionName, bool val);
    int getInt(nString optionName);
    f32 getFloat(nString optionName);
    bool getBool(nString optionName);
    
    bool needsFboReload = false;
    bool needsTextureReload = false;
    bool needsShaderReload = false;
    bool needsWindowReload = false;

    Event<> OptionsChange;

private:
    nString m_filePath = "";
    SoaOptions m_tempCopy;
    SoaOptions m_default;
};

#endif // OptionsController_h__

