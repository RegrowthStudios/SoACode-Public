///
/// ModPathResolver.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 19 Apr 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Resolves file paths for mod files by first looking in 
/// a mod folder, then a default if it fails.
///

#pragma once

#ifndef ModPathResolver_h__
#define ModPathResolver_h__

#include <Vorb/io/IOManager.h>

class ModPathResolver {
public:
    /// Initialization
    void init(const vio::Path& defaultPath, const vio::Path& modPath);
    /// Sets directory for defaults
    void setDefaultDir(const vio::Path& path);
    /// Sets directory for mods
    void setModDir(const vio::Path& path);
    /// Gets the absolute path. If not in Mod, checks in default.
    /// @return false on failure
    bool resolvePath(const vio::Path& path, vio::Path& resultAbsolutePath, bool printModNotFound = false) const;

    vio::IOManager defaultIom;
    vio::IOManager modIom;
};

#endif // ModPathResolver_h__
