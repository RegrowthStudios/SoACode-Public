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

    enum class BufferTarget {
        ARRAY = GL_ARRAY_BUFFER,
        COPY_READ = GL_COPY_READ_BUFFER,
        COPY_WRITE = GL_COPY_WRITE_BUFFER,
        ELEMENT_ARRAY = GL_ELEMENT_ARRAY_BUFFER,
        PIXEL_PACK = GL_PIXEL_PACK_BUFFER,
        PIXEL_UNPACK = GL_PIXEL_UNPACK_BUFFER,
        TEXTURE = GL_TEXTURE_BUFFER,
        TRANSFORM_FEEDBACK = GL_TRANSFORM_FEEDBACK_BUFFER,
        UNIFORM = GL_UNIFORM_BUFFER
    };

    enum class BufferUsage {
        STREAM_DRAW = GL_STREAM_DRAW, 
        STREAM_READ = GL_STREAM_READ, 
        STREAM_COPY = GL_STREAM_COPY,
        STATIC_DRAW = GL_STATIC_DRAW,
        STATIC_READ = GL_STATIC_READ,
        STATIC_COPY = GL_STATIC_COPY,
        DYNAMIC_DRAW = GL_DYNAMIC_DRAW, 
        DYNAMIC_READ = GL_DYNAMIC_READ, 
        DYNAMIC_COPY = GL_DYNAMIC_COPY
    };

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

        /// Frees an OpenGL buffer object and sets the
        /// ID to 0.
        /// @param bufferID: The ID of the buffer
        static void freeBuffer(ui32& bufferID);

        /// Binds a buffer
        /// @param bufferID: The ID of the buffer
        /// @param target: The desired buffer target
        static void bindBuffer(const ui32& bufferID, BufferTarget target) {
            glBindBuffer(static_cast<GLenum>(target), bufferID);
        }
    
        /// Uploads data to a buffer. Make sure the buffer is bound using
        /// bindBuffer before uploading.
        /// @param bufferID: The ID of the buffer
        /// @param target: The desired buffer target
        /// @param bufferSize: The size of data
        /// @param data: Pointer to the buffer data
        /// @usage: The desired buffer usage
        static void uploadBufferData(ui32 bufferID,
                                     BufferTarget target,
                                     ui32 bufferSize,
                                     const void* data,
                                     BufferUsage usage);


        /// Gets the amount of VRAM used in bytes
        static ui32 getTotalVramUsage() { return _totalVramUsage; }

        /// Gets the texture VRAM used in bytes
        static ui32 getTextureVramUsage() { return _textureVramUsage; }

        /// Gets the buffer VRAM used in bytes
        static ui32 getBufferVramUsage() { return _bufferVramUsage; }
    private:

        static ui32 _totalVramUsage; ///< The total VRAM usage by all objects
        static ui32 _textureVramUsage; ///< The total VRAM usage by texture objects
        static ui32 _bufferVramUsage; ///< The total VRAM usage by buffer objects

        static std::unordered_map<ui32, ui32> _textures; ///< Store of texture objects

        static std::unordered_map<ui32, ui32> _buffers; ///< Store of buffer objects
    };

}
}
}

// Namespace Alias
namespace vg = vorb::core::graphics;

#endif // GPUMEMORY_H_