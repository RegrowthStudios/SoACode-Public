///
/// LoadContext.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 4 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// A common context space where assets can
/// be loaded.
///

#pragma once

#ifndef LoadContext_h__
#define LoadContext_h__

#include "GameMeshLoader.h"
#include "ImageAssetLoader.h"
#include "ShaderAssetLoader.h"

class LoadContext {
public:
    void resetWork() {
        m_totalWork = 0;
        m_workFinished = 0;
    }
    void addAnticipatedWork(ui32 w) {
        m_totalWork += w;
    }
    void addWorkCompleted(ui32 w) {
        m_workFinished += w;
    }

    f32 getPercentComplete() const {
        return (f32)m_workFinished / (f32)m_totalWork;
    }
    bool isWorkComplete() const {
        return m_workFinished >= m_totalWork;
    }

    struct {
        ImageAssetLoader image;
        GameMeshLoader mesh;
        ShaderAssetLoader shader;
    } loaders;

    nString environment;
private:
    ui32 m_totalWork = 0;
    ui32 m_workFinished = 0;
};

#endif // LoadContext_h__