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

#include <list>

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
    void addAnticipatedWork(ui32 work) {
        m_totalWork += work;
    }
    void addWorkCompleted(ui32 work) {
        m_workFinished += work;
    }

    f32 getPercentComplete() const {
        return (f32)m_workFinished / (f32)m_totalWork;
    }
    bool isWorkComplete() const {
        return m_workFinished >= m_totalWork;
    }

    struct {
        ImageAssetLoader image;
        ShaderAssetLoader shader;
    } loaders;

    nString environment;
    vcore::RPCManager rpcManager;
protected:
    ui32 m_totalWork = 0;
    volatile ui32 m_workFinished = 0;
};

// For when the number of tasks is known
class StaticLoadContext : public LoadContext {
public:
    // Call before begin() to indicate how much work is
    // anticipated.
    // numTasks is the number of tasks that will be added
    // via addTask.
    void addAnticipatedWork(ui32 work) = delete;
    void addAnticipatedWork(ui32 work, ui32 numTasks) {
#ifdef DEBUG
        if (m_glRPCs) throw 380;
#endif
        m_totalWork += work;
        m_numTasks += numTasks;
    }

    // If using any GL RPCs, be sure to call on GL thread
    size_t processRequests(size_t maxRequests) {
        return rpcManager.processRequests(maxRequests);
    }

    // Call this after all anticipated work is added, and before
    // any tasks are added.
    void begin() {
        m_glRPCs = new vcore::GLRPC[m_numTasks];
    }

    // It is important that tasks added have been anticipated with
    // addAnticipatedWork
    template<typename F>
    vcore::GLRPC& addTask(F f, bool blockUntilFinished) {
#ifdef DEBUG
        if (m_freeTask > m_numTasks) throw 381;
#endif
        vcore::GLRPC& rpc = m_glRPCs[m_freeTask++];
        rpc.set(f);
        rpcManager.invoke(&rpc, blockUntilFinished);
        return rpc;
    }

    f32 getPercentComplete() const {
        return (f32)m_workFinished / (f32)m_totalWork;
    }
    bool isWorkComplete() const {
        return m_workFinished >= m_totalWork;
    }

    void blockUntilFinished() {
        if (m_freeTask) m_glRPCs[m_freeTask - 1].block();
    }

    // Call this when work is completed to free memory used by GLRPCs.
    void end() {
        delete[] m_glRPCs;
        m_glRPCs = nullptr;
        m_numTasks = 0;
        m_freeTask = 0;
        resetWork();
    }
protected:
    vcore::GLRPC* m_glRPCs = nullptr;
    ui32 m_numTasks = 0;
    volatile ui32 m_freeTask = 0;
};

// For when the number of tasks might change
class DynamicLoadContext : public LoadContext {
public:
    void resetWork() {
        m_totalWork = 0;
        m_workFinished = 0;
    }

    // If using any GL RPCs, be sure to call on GL thread
    size_t processRequests(size_t maxRequests) {
        return rpcManager.processRequests(maxRequests);
    }

    // It is important that tasks added have been anticipated with
    // addAnticipatedWork
    template<typename F>
    vcore::GLRPC& addTask(F f, bool blockUntilFinished) {
        m_glRPCs.emplace_back();
        vcore::GLRPC& rpc = m_glRPCs.back();
        rpc.set(f);
        rpcManager.invoke(&rpc, blockUntilFinished);
        return rpc;
    }

    f32 getPercentComplete() const {
        return (f32)m_workFinished / (f32)m_totalWork;
    }
    bool isWorkComplete() const {
        return m_workFinished >= m_totalWork;
    }

    void blockUntilFinished() {
        m_glRPCs.back().block();
    }

    // Call this when work is completed to free memory used by GLRPCs.
    void clearTasks() {
        std::list<vcore::GLRPC>().swap(m_glRPCs);
    }

protected:
    std::list<vcore::GLRPC> m_glRPCs;
};

#endif // LoadContext_h__
