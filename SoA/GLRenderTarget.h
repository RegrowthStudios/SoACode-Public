///
/// GLRenderTarget.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 6 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// An OpenGL FBO object that contains minimum functionality
///

#pragma once

#ifndef GLRenderTarget_h__
#define GLRenderTarget_h__

#include "GLEnums.h"

namespace vorb {
    namespace core {
        namespace graphics {
            /// An FBO object that contains no hidden functionality
            class GLRenderTarget {
            public:
                /// Specify a frameBuffer with a certain size
                /// @param w: Width of framebuffer in pixels
                /// @param h: Height of framebuffer in pixels
                GLRenderTarget(ui32 w = 0, ui32 h = 0);
                /// Specify a framebuffer with a certain size
                /// @param s: Width and height of framebuffer in pixels
                GLRenderTarget(ui32v2 s) : GLRenderTarget(s.x, s.y) {
                    // Empty
                }

                /// Initialize the framebuffer on the GPU
                /// @param format: Internal storage of the color texture target
                /// @param depthStencilFormat: Internal storage found in depth/stencil texture (NONE to halt creation of texture)
                /// @param msaa: Number of MSAA samples
                /// @return Self
                GLRenderTarget& init(
                    TextureInternalFormat format = TextureInternalFormat::RGBA8,
                    TextureInternalFormat depthStencilFormat = TextureInternalFormat::DEPTH24_STENCIL8,
                    ui32 msaa = 0);

                /// Dispose all GPU resources used by this FBO
                void dispose();

                /// @return OpenGL FBO ID
                const ui32& getID() const {
                    return _fbo;
                }
                /// @return OpenGL color texture ID
                const ui32& getTextureID() const {
                    return _texColor;
                }

                /// @return Size of the FBO in pixels (W,H)
                const ui32v2& getSize() const {
                    return _size;
                }
                /// @return Width of the FBO in pixels
                const ui32& getWidth() const {
                    return _size.x;
                }
                /// @return Height of the FBO in pixels
                const ui32& getHeight() const {
                    return _size.y;
                }

                /// Set the size of this FBO before its initialization
                /// @param w: Width in pixels
                /// @param h: Height in pixels
                void setSize(const ui32& w, const ui32& h);

                /// Bind this as the current target for pixel output and change the viewport to fill it
                void use() const;
                /// Use the internal backbuffer for pixel output and reset the viewport
                /// @param w: Width of backbuffer
                /// @param h: Height of backbuffer
                static void unuse(ui32 w, ui32 h);

                /// Bind the color texture
                void bindTexture() const;
                /// Unbind the color texture
                void unbindTexture() const;
            private:
                ui32v2 _size; ///< The Width And Height Of The FrameBuffer

                ui32 _fbo = 0; ///< FrameBuffer ID
                ui32 _texColor = 0; ///< Color Texture Of FrameBuffer
                ui32 _texDS = 0; ///< Depth/Stencil Texture Of FrameBuffer
                VGEnum textureTarget; ///< The Kinds Of Textures Bound To This FBO
            };
        }
    }
}
namespace vg = vorb::core::graphics;

#endif // GLRenderTarget_h__