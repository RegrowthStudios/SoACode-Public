#include "stdafx.h"
#include "GpuMemory.h"

#include "ImageLoader.h"
#include "utils.h"

#include <GL/glew.h>

#define RGBA_BYTES 4

ui32 vg::GpuMemory::_totalVramUsage = 0;
ui32 vg::GpuMemory::_textureVramUsage = 0;
ui32 vg::GpuMemory::_bufferVramUsage = 0;

std::unordered_map<ui32, ui32> vg::GpuMemory::_textures;
std::unordered_map<ui32, ui32> vg::GpuMemory::_buffers;

ui32 vg::GpuMemory::uploadTexture(const ui8* pixels,
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Setup Texture Sampling Parameters
    samplingParameters->set(GL_TEXTURE_2D);

    // Get the number of mipmaps for this image
    mipmapLevels = MIN(mipmapLevels, maxMipmapLevels);

    // Create Mipmaps If Necessary
    if (mipmapLevels > 0) {
        glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, mipmapLevels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmapLevels);
        glEnable(GL_TEXTURE_2D);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // Calculate memory usage
    ui32 vramUsage = width * height * RGBA_BYTES;
    // Store the texture and its memory usage
    _textures[textureID] = vramUsage;
    _totalVramUsage += vramUsage;
    _textureVramUsage += vramUsage;

    return textureID;
}

void vg::GpuMemory::freeTexture(ui32& textureID) {
    
    // See if the texture was uploaded through GpuMemory
    auto it = _textures.find(textureID);
    if (it != _textures.end()) {
        // Reduce total vram usage
        _totalVramUsage -= it->second;
        _textureVramUsage -= it->second;
        _textures.erase(it);
    }

    // Delete the texture
    glDeleteTextures(1, &textureID);
    textureID = 0;
}

void vg::GpuMemory::uploadBufferData(ui32 bufferID, BufferTarget target, ui32 bufferSize, const void* data, BufferUsageHint usage)
{
    // Orphan the buffer by default
    glBufferData(static_cast<GLenum>(target), bufferSize, nullptr, static_cast<GLenum>(usage));
    glBufferSubData(static_cast<GLenum>(target), 0, bufferSize, data);

    // Track the VRAM usage
    auto it = _buffers.find(bufferID);
    if (it != _buffers.end()) {
        ui32 memoryDiff = bufferSize - it->second;
        _totalVramUsage += memoryDiff;
        _bufferVramUsage += memoryDiff;
        it->second = bufferSize;
    } else {
        // Start tracking the buffer object if it is not tracked
        _buffers[bufferID] = bufferSize;
        _totalVramUsage += bufferSize;
        _bufferVramUsage += bufferSize;
    }
}

void vg::GpuMemory::freeBuffer(ui32& bufferID)
{
    // Reduce our memory counters
    auto it = _buffers.find(bufferID);
    if (it != _buffers.end()) {
        _totalVramUsage -= it->second;
        _bufferVramUsage -= it->second;
        _buffers.erase(it);
    }

    // Delete the buffer
    glDeleteBuffers(1, &bufferID);
    bufferID = 0;
}

void vg::GpuMemory::changeTextureMemory(ui32 s) {
    _textureVramUsage += s;
    _totalVramUsage += s;
}
void vg::GpuMemory::changeBufferMemory(ui32 s) {
    _bufferVramUsage += s;
    _totalVramUsage += s;
}

ui32 vg::GpuMemory::getFormatSize(ui32 format) {
    switch (format) {
    case GL_RGBA4:
    case GL_RGBA16:
        return 2;
    case GL_RGBA8:
        return 4;
    default:
        break;
    }
}
