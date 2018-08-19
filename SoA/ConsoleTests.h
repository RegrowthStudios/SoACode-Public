//
// ConsoleTests.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 1 Aug 2015
// Copyright 2014 Regrowth Studios
// MIT License
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
ChunkAccessSpeedData* createCASData(size_t numThreads, size_t requestCount, ui64 maxID);
void runCAS(ChunkAccessSpeedData* data);
void freeCAS(ChunkAccessSpeedData* data);

void runCHS();

#endif // !ConsoleTests_h__
