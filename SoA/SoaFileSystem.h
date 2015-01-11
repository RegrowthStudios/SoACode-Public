///
/// SoaFilesystem.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Provides utilities for operating within the SoA filesystem
///

#pragma once

#ifndef SoaFilesystem_h__
#define SoaFilesystem_h__

#include <vorb/IOManager.h>

class SoaFileSystem {
public:
    void init();

    const vio::IOManager& get(const nString& name) const;
private:
    std::unordered_map<nString, vio::IOManager> m_roots; ///< IOManagers mapped by friendly file system names
};


#endif // SoaFilesystem_h__
