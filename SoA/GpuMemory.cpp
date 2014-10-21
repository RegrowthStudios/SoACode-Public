#include "stdafx.h"
#include "GpuMemory.h"

#include "ImageLoader.h"
#include "utils.h"

#include <GL/glew.h>

#define RGBA_BYTES 4

namespace vg {
   
ui32 GpuMemory::_totalVramUsage = 0;


ui32 GpuMemory::uploadTexture(const std::vector<ui8>& pixels,
                              ui32 width, 
                              ui32 height, 
                              SamplerState* samplingParameters, 
                              i32 mipmapLevels /* = INT_MAX */) {
    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Determine The Maximum Number Of Mipmap Levels Available
    i32 maxMipmapLevels = 0;
    i32 size = MIN(width, height);
    while (size > 1) {
        maxMipmapLevels++;
        size >>= 1;
    }

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Setup Texture Sampling Parameters
    samplingParameters->set(GL_TEXTURE_2D);

    // Get the number of mipmaps for this image
    mipmapLevels = MIN(mipmapLevels, maxMipmapLevels);

    // Create Mipmaps If Necessary
    if (mipmapLevels > 0) {
        glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, mipmapLevels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmapLevels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // Calculate memory usage
    ui32 vramUsage = width * height * RGBA_BYTES;
    // Store the texture and its memrory usage
    _textures[textureID] = vramUsage;
    _totalVramUsage += vramUsage;

    return textureID;
}

void GpuMemory::freeTexture(ui32& textureID) {
    // Delete the texture
    glDeleteTextures(1, &textureID);
    textureID = 0;

    // See if the texture was uploaded through GpuMemory
    auto it = _textures.find(textureID);
    if (it != _textures.end()) {
        // Reduce total vram usage
        _totalVramUsage -= it->second;
        _textures.erase(it);
    }
}


}