#pragma once

// Windows
#if !defined(WIN32) && defined(_WIN32)
#define WIN32 _WIN32
#endif

#if !defined(WIN64) && defined(_WIN64)
#define WIN64 _WIN64
#endif

#if defined(WIN32) || defined(WIN64)
#include "Windows.h"
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

// register keyword creates a warning and is deprecated
#if !defined(REGISTER)
#if defined(WIN32) || defined(WIN64)
#define REGISTER register
#else
#define REGISTER
#endif
#endif
