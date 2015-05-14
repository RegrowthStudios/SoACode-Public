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

#include "SoaOptions.h"

class OptionsController {
public:
    OptionsController();
    ~OptionsController();
    /// Begins a context for changing options.
    /// Call this when beginning to change options.
    void beginContext(SoaOptions* options);

    void loadOptions(SoaOptions* options);

    void saveOptions(SoaOptions* options);

    void restoreDefault();
    
    bool needsFboReload = false;
    bool needsTextureReload = false;
    bool needsShaderReload = false;
    bool needsWindowReload = false;
private:
    SoaOptions* m_options = nullptr;
    SoaOptions m_tempCopy;
    SoaOptions m_default;
};

#endif // OptionsController_h__

