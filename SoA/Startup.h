//
// Startup.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 29 Jun 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// 
//

#pragma once

#ifndef Startup_h__
#define Startup_h__

#include "Vorb/types.h"

/*! @brief Designates the startup format.
 */
enum class Startup {
    APP,
    CONSOLE,
    HELP,
    EXIT
};

/*! @brief Determines how to startup the application from its arguments.
 * 
 * @param argc: Number of process arguments.
 * @param argv: Values of process arguments.
 * @return The way to start the application.
 */
Startup startup(int argc, cString* argv);

#endif // Startup_h__
