//
// ConsoleTests.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 1 Aug 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// 
//

#pragma once

#ifndef ConsoleTests_h__
#define ConsoleTests_h__

#include "Chunk.h"

/************************************************************************/
/* Chunk Access Speed                                                   */
/************************************************************************/
struct ChunkAccessSpeedData;
ChunkAccessSpeedData* createCASData(size_t numThreads, size_t requestCount, ChunkID maxID);
void runCAS(ChunkAccessSpeedData* data);
void freeCAS(ChunkAccessSpeedData* data);

#endif // !ConsoleTests_h__
