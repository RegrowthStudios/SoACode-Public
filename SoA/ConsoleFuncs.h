//
// ConsoleFuncs.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 30 Jun 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// 
//

#pragma once

#ifndef ConsoleFuncs_h__
#define ConsoleFuncs_h__

#include <Vorb/VorbPreDecl.inl>

DECL_VSCRIPT(class Environment);

void registerFuncs(vscript::Environment& env);

int loadDLL(const cString name);

#endif // ConsoleFuncs_h__
