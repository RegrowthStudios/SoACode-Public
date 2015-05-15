///
/// OptionsController.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 13 May 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles changing of options.
///

#pragma once

#ifndef OptionsController_h__
#define OptionsController_h__

#include <Vorb/script/Environment.h>
#include "SoaOptions.h"

class OptionsController {
public:
    OptionsController();
    ~OptionsController();

    // Call right before loading options
    void setDefault();

    /// Begins a context for changing options.
    /// Call this when beginning to change options.
    void beginContext();

    void loadOptions();

    void saveOptions();

    void restoreDefault();

    void registerScripting(vscript::Environment* env);

    // These can be called from lua scripts
    void setOptionInt(nString optionName, int val);
    void setOptionFloat(nString optionName, f32 val);
    void setOptionBool(nString optionName, bool val);
    int getOptionInt(nString optionName);
    f32 getOptionFloat(nString optionName);
    bool getOptionBool(nString optionName);
    
    bool needsFboReload = false;
    bool needsTextureReload = false;
    bool needsShaderReload = false;
    bool needsWindowReload = false;
private:
    SoaOptions m_tempCopy;
    SoaOptions m_default;
};

#endif // OptionsController_h__

