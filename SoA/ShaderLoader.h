///
/// ShaderLoader.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 31 Mar 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles easy loading and error checking of shaders for SoA
///

#pragma once

#ifndef ShaderLoader_h__
#define ShaderLoader_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/io/Path.h>
#include <Vorb/graphics/GLProgram.h>

DECL_VIO(class IOManager);

#pragma once
class ShaderLoader {
public:
    /// Creates a program using code loaded from files, and does error checking
    /// Does not register with global cache
    static CALLER_DELETE vg::GLProgram createProgramFromFile(const vio::Path& vertPath, const vio::Path& fragPath,
                                                             vio::IOManager* iom = nullptr, cString defines = nullptr);

    /// Creates a program using passed code, and does error checking
    /// Does not register with global cache
    static CALLER_DELETE vg::GLProgram createProgram(const cString displayName, const cString vertSrc, const cString fragSrc,
                                                     vio::IOManager* iom = nullptr, cString defines = nullptr);
};

#endif // ShaderLoader_h__

