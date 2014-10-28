// 
//  GpuMemory.h
//  Vorb Engine
//
//  Created by Ben Arnold on 20 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides a wrapper around VRAM and
//  openGL object uploads.
//

#pragma once

#ifndef GPUMEMORY_H_
#define GPUMEMORY_H_

#include <unordered_map>

class SamplerState;

namespace vorb {
namespace core {
namespace graphics {

// TODO(Ben): Flesh this out
class GpuMemory
{
public:
    /// Uploads a texture to the GPU.
    /// @param pixels: The image pixels
    /// @param width: Width of the texture in pixels
    /// @param height: Height of the texture in pixels
    /// @param samplingParameters: The texture sampler parameters
    /// @param mipmapLevels: The max number of mipmap levels
    /// @return The texture ID
    static ui32 uploadTexture(const ui8* pixels,
                              ui32 width,
                              ui32 height,
                              SamplerState* samplingParameters,
                              i32 mipmapLevels = INT_MAX);

    /// Frees a texture and sets its ID to 0
    /// @param textureID: The texture to free. Will be set to 0.
    static void freeTexture(ui32& textureID);

    /// Creates an OpenGL buffer object
    /// @param vbo: The result buffer ID
    static void createBuffer(ui32& bufferID) {
        glGenBuffers(1, &bufferID);
        _buffers[bufferID] = 0;
    }

    //static void uploadBufferData(ui32 bufferID, 

    /// Gets the amount of VRAM used in bytes
    static ui32 getTotalVramUsage() { return _totalVramUsage; }
private:

    static ui32 _totalVramUsage; ///< The total VRAM usage by all objects
    static ui32 _textureVramUsage; ///< The total VRAM usage by texture objects
    static ui32 _bufferVramUsage; ///< The total VRAM usage by buffer objects

    static std::unordered_map<ui32, ui32> _textures; ///< Store of texture objects

    static std::unordered_map<ui32, ui32> _buffers;
};

}
}
}

// Namespace Alias
namespace vg = vorb::core::graphics;

#endif // GPUMEMORY_H_