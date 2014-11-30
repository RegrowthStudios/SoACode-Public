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
#include "gtypes.h"

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

                /// Destructor
                ~GLRenderTarget() { dispose(); }

                /// Initialize the framebuffer on the GPU
                /// @param format: Internal storage of the color texture target
                /// @param msaa: Number of MSAA samples
                /// @return Self
                GLRenderTarget& init(TextureInternalFormat format = TextureInternalFormat::RGBA8, ui32 msaa = 0, TextureFormat pFormat = TextureFormat::RGBA, TexturePixelType pType = TexturePixelType::UNSIGNED_BYTE);
                /// Append a depth texture into the framebuffer
                /// @param depthFormat: Internal storage found in depth texture
                /// @return Self
                GLRenderTarget& initDepth(TextureInternalFormat depthFormat = TextureInternalFormat::DEPTH_COMPONENT32F);
                /// Append a stencil texture into the framebuffer
                /// @param stencilFormat: Internal storage found in stencil texture
                /// @return Self
                GLRenderTarget& initDepthStencil(TextureInternalFormat stencilFormat = TextureInternalFormat::DEPTH24_STENCIL8);

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
                
                /// @return True if this is initialized with MSAA
                bool isMSAA() const {
                    return _msaa > 0;
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

                ui32 _fbo = 0; ///< Framebuffer ID
                ui32 _texColor = 0; ///< Color texture of framebuffer
                ui32 _texDepth = 0; ///< Depth texture of framebuffer
                ui32 _texStencil = 0; ///< Stencil texture of framebuffer
                ui32 _msaa = 0; ///< MSAA sample count in this framebuffer
                VGEnum _textureTarget; ///< The kinds of textures bound to this FBO
            };
        }
    }
}
namespace vg = vorb::core::graphics;

#endif // GLRenderTarget_h__