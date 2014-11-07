///
/// GLRenderTarget.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 6 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// An OpenGL FBO Object That Contains Minimum Functionality
///

#pragma once

#ifndef GLRenderTarget_h__
#define GLRenderTarget_h__

#include "GLEnums.h"

namespace vorb {
    namespace core {
        namespace graphics {
            /// An FBO Object That Contains No Hidden Functionality
            class GLRenderTarget {
            public:
                /// Specify A FrameBuffer With A Certain Size
                /// @param w: Width Of FrameBuffer In Pixels
                /// @param h: Width Of FrameBuffer In Pixels
                GLRenderTarget(ui32 w = 0, ui32 h = 0);
                /// Specify A FrameBuffer With A Certain Size
                /// @param s: Width And Height Of FrameBuffer In Pixels
                GLRenderTarget(ui32v2 s) : GLRenderTarget(s.x, s.y) {
                    // Empty
                }

                /// Initialize The FrameBuffer On The GPU
                /// 
                GLRenderTarget& init(
                    TextureInternalFormat format = TextureInternalFormat::RGBA8,
                    TextureInternalFormat depthStencilFormat = TextureInternalFormat::DEPTH24_STENCIL8,
                    ui32 msaa = 0);
                void dispose();

                const ui32& getID() const {
                    return _fbo;
                }
                const ui32& getTextureID() const {
                    return _texColor;
                }

                const ui32v2& getSize() const {
                    return _size;
                }
                const ui32& getWidth() const {
                    return _size.x;
                }
                const ui32& getHeight() const {
                    return _size.y;
                }

                void setSize(const ui32& w, const ui32& h);

                void use() const;
                static void unuse(ui32 w, ui32 h);

                void bindTexture() const;
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