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

#include "Options.h"

class OptionsController {
public:
    OptionsController();
    /// Begins a context for changing options.
    /// Call this when beginning to change options.
    void beginContext(Options* options);

    void saveChanges();

    void restoreDefault();
    
    bool needsFboReload = false;
    bool needsTextureReload = false;
    bool needsShaderReload = false;
    bool needsWindowReload = false;
private:
    Options* m_options = nullptr;
    Options m_tempCopy;
    Options m_default;
};

#endif // OptionsController_h__

