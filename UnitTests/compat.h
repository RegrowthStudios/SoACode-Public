#pragma once

// Windows
#if !defined(WIN32) && defined(_WIN32)
#define WIN32 _WIN32
#endif

#if !defined(WIN64) && defined(_WIN64)
#define WIN64 _WIN64
#endif

#if defined(WIN32) || defined(WIN64)
#define OS_WINDOWS
#include "Windows.h"
#endif

// register keyword creates a warning and is deprecated
#if !defined(REGISTER)
#if defined(WIN32) || defined(WIN64)
#define REGISTER register
#else
#define REGISTER
#endif
#endif
