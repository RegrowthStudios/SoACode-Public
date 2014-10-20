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

struct TextureInfo {
    TextureInfo() : ID(0), width(1), height(1) {}
    GLuint ID;
    int width;
    int height;
    void freeTexture() {
        if (ID != 0) {
            glDeleteTextures(1, &ID);
            ID = 0;
        }
    }
};

struct Animation {
    Animation() : fadeOutBegin(INT_MAX) {}
    int duration;
    int fadeOutBegin;
    int repetitions;
    int xFrames;
    int yFrames;
    int frames;
    TextureInfo textureInfo;
};

extern SamplerState textureSamplers[6];

// Load a .DDS file using GLFW's own loader
GLuint loadDDS(const cString imagepath, i32 smoothType);

struct PNGLoadInfo {
public:
    PNGLoadInfo(SamplerState* ss, i32 mipmaps) :
        samplingParameters(ss),
        mipmapLevels(mipmaps) {}

    SamplerState* samplingParameters;
    i32 mipmapLevels;
};

ui32v2 readImageSize(IOManager* iom, const cString imagePath);
ui8* loadPNG(const cString imagepath, ui32& rWidth, ui32& rHeight);
ui8* loadPNG(TextureInfo& texInfo, const cString imagepath, PNGLoadInfo texParams, bool makeTexture = true);
ui8* loadPNG(TextureInfo& texInfo, ui8* pngdata, size_t fileSize, PNGLoadInfo texParams, bool makeTexture);
void savePNG(nString fileName, ui32 width, ui32 height, std::vector<ui8> imgData);