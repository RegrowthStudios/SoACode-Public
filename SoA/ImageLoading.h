#pragma once
#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>

#include <SDL/SDL.h>

#include "Constants.h"
#include "lodepng.h"
#include "SamplerState.h"

class IOManager;

// TODO: Remove This
using namespace std;

struct Animation {
    Animation() : fadeOutBegin(INT_MAX) {}
    int duration;
    int fadeOutBegin;
    int repetitions;
    int xFrames;
    int yFrames;
    int frames;
    Texture textureInfo;
};

extern SamplerState textureSamplers[6];

// Load a .DDS file using GLFW's own loader
GLuint loadDDS(const cString imagepath, i32 smoothType);

ui32v2 readImageSize(IOManager* iom, const cString imagePath);