#pragma once
// Make Sure We Use Correct Debug Variables
#ifdef DEBUG
#ifndef _DEBUG
#define _DEBUG
#endif
#endif // DEBUG
#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif // _DEBUG

#define MACRO_PARAN_L {
#define MACRO_PARAN_R }

#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <queue>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <GL\glew.h>

#include "types.h"

//Uncommment to use flat arrays instead of interval trees
//define USEARRAYS