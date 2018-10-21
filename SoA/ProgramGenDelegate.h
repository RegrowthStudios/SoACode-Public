///
/// ProgramGenDelegate.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 4 Feb 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Delegate for generating a shader program
///

#pragma once

#ifndef ProgramGenDelegate_h__
#define ProgramGenDelegate_h__

#include "ShaderLoader.h"

#include <Vorb/vorb_rpc.h>
#include <Vorb/graphics/GLProgram.h>

class ProgramGenDelegate {
public:
    virtual void invoke(Sender sender VORB_MAYBE_UNUSED, void* userData VORB_MAYBE_UNUSED) {
        printf("Building shader: %s\n", name);
        if (isFromFile) {
            program = ShaderLoader::createProgramFromFile(vs, fs, iom);
        } else {
            program = ShaderLoader::createProgram(name, vs, fs);
        }
    }

    ProgramGenDelegate() {
        del = makeDelegate(*this, &ProgramGenDelegate::invoke);
        rpc.data.f = &del;
    }

    void init(const cString name, const cString vs, const cString fs) {
        this->name = name;
        this->vs = vs;
        this->fs = fs;
        rpc.data.userData = nullptr;
        isFromFile = false;
    }

    void initFromFile(const cString name, const cString vertPath, const cString fragPath, vio::IOManager* iom) {
        this->name = name;
        this->vs = vertPath;
        this->fs = fragPath;
        this->iom = iom;
        rpc.data.userData = nullptr;
        isFromFile = true;
    }

    const cString name;
    const cString vs = nullptr;
    const cString fs = nullptr;
    bool isFromFile = false;

    vcore::RPC rpc;
    Delegate<Sender, void*> del;
    vio::IOManager* iom = nullptr;

    vg::GLProgram program;
    nString errorMessage;
};

#endif // ProgramGenDelegate_h__
