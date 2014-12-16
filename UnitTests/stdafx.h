#pragma once

#ifndef STDAFX_H_
#define STDAFX_H_

#include <compat.h>

// Make Sure We Use Correct Debug Variables
#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG
#endif
#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif

#define MACRO_PARAN_L {
#define MACRO_PARAN_R }

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <types.h>

#endif // STDAFX_H_
