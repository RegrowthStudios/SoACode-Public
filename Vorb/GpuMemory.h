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
            enum class BufferTarget;
            enum class BufferUsageHint;

            // TODO(Ben): Flesh this out
            class GpuMemory {
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
                    BufferUsageHint usage);

                /// Changes The Total Texture Memory Usage By A Specified Amount
                /// @param s: Amount Of Memory Change In Bytes
                static void changeTextureMemory(ui32 s);
                /// Changes The Total Buffer Memory Usage By A Specified Amount
                /// @param s: Amount Of Memory Change In Bytes
                static void changeBufferMemory(ui32 s);

                /// Gets the amount of VRAM used in bytes
                static ui32 getTotalVramUsage() {
                    return _totalVramUsage;
                }

                /// Gets the texture VRAM used in bytes
                static ui32 getTextureVramUsage() {
                    return _textureVramUsage;
                }

                /// Gets the buffer VRAM used in bytes
                static ui32 getBufferVramUsage() {
                    return _bufferVramUsage;
                }

                static ui32 getFormatSize(ui32 format);
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