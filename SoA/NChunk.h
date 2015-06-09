//
// NChunk.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 25 May 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// 
//

#pragma once

#ifndef NChunk_h__
#define NChunk_h__

#include "SmartVoxelContainer.hpp"

class NChunk;
typedef NChunk* NChunkPtr;

struct NChunkLayer {

};
struct NChunkLine {
public:
    
};

class NChunk {
public:



private:
    vvox::SmartVoxelContainer<ui16> m_blocks;
};

#endif // NChunk_h__
