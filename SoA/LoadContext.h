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

#include "ImageAssetLoader.h"
#include "ShaderAssetLoader.h"

class LoadContext {
public:
    void resetWork() {
        m_totalWork = 0;
        m_workFinished = 0;
    }
    // Call before begin() to indicate how much work is
    // anticipated.
    // numTasks is the number of tasks that will be added
    // via addTask.
    void addAnticipatedWork(ui32 work, ui32 numTasks) {
#ifdef DEBUG
        if (m_glRPCs) throw 380;
#endif
        m_totalWork += work;
        m_numTasks += numTasks;
    }
    void addWorkCompleted(ui32 work) {
        m_workFinished += work;
    }

    // If using any GL RPCs, be sure to call on GL thread
    size_t processRequests(size_t maxRequests) {
        return m_rpcManager.processRequests(maxRequests);
    }

    // Call this after all anticipated work is added, and before
    // any tasks are added.
    void begin() {
        m_glRPCs = new vcore::GLRPC[m_numTasks];
    }
    
    // It is important that tasks added have been anticipated with
    // addAnticipatedWork
    template<typename F>
    void addTask(F f, bool blockUntilFinished) {
#ifdef DEBUG
        if (m_freeTask > m_numTasks) throw 381;
#endif
        vcore::GLRPC& rpc = m_glRPCs[m_freeTask++];
        rpc.set(f);
        m_rpcManager.invoke(&rpc, blockUntilFinished)
    }

    f32 getPercentComplete() const {
        return (f32)m_workFinished / (f32)m_totalWork;
    }
    bool isWorkComplete() const {
        return m_workFinished >= m_totalWork;
    }
    
    void blockUntilFinished() {
        m_glRPCs[m_numTasks - 1].block();
    }

    // Call this when work is completed to free memory used by GLRPCs.
    void end() {
        delete[] m_glRPCs;
        m_glRPCs = nullptr;
        m_numTasks = 0;
        m_totalWork = 0;
        m_freeTask = 0;
        m_workFinished = 0;
    }

    struct {
        ImageAssetLoader image;
        ShaderAssetLoader shader;
    } loaders;

    nString environment;
private:
    vcore::RPCManager m_rpcManager;
    vcore::GLRPC* m_glRPCs = nullptr;
    ui32 m_totalWork = 0;
    ui32 m_numTasks = 0;
    ui32 m_freeTask = 0;
    volatile ui32 m_workFinished = 0;
};

#endif // LoadContext_h__
