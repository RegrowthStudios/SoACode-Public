///
/// GBuffer.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 21 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef GBuffer_h__
#define GBuffer_h__

#include "GLEnums.h"
#include "gtypes.h"

#define GBUFFER_INTERNAL_FORMAT_DEPTH vg::TextureInternalFormat::RGBA8
#define GBUFFER_INTERNAL_FORMAT_NORMAL vg::TextureInternalFormat::RGBA16F
#define GBUFFER_INTERNAL_FORMAT_COLOR vg::TextureInternalFormat::R32F

namespace vorb {
    namespace core {
		namespace graphics {
            class GBuffer {
            public:
                /// Name-defined textures stored in this GBuffer
                struct TextureIDs {
                    ui32 depth; ///< R-32f texture
                    ui32 normal; ///< RGB-16f normal texture
                    ui32 color; ///< RGBA-8f colorRGB-specularAlpha texture
                };

                GBuffer(ui32 w = 0, ui32 h = 0);
                GBuffer(ui32v2 s) : GBuffer(s.x, s.y) {
                    // Empty
                }

                GBuffer& init();
                GBuffer& initDepth(TextureInternalFormat depthFormat = TextureInternalFormat::DEPTH_COMPONENT32);
                void dispose();

                void use();

                /// @return OpenGL texture IDs
                const GBuffer::TextureIDs& getTextureIDs() const {
                    return _tex;
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

            private:
                ui32v2 _size; ///< The width and height of the GBuffer

                //union {
                //    struct {
                //        ui32 depth; ///< R-32f depth target
                //        ui32 normal; ///< RGB-16f normal target
                //        ui32 color; ///< RGBA-8f colorRGB-specularAlpha target
                //    } _fbo = { 0, 0, 0 };
                //    ui32 _fbos[3]; ///< All 3 rendering targets
                //};
                ui32 _fbo; ///< The rendering target
                union {
                    TextureIDs _tex;
                    ui32 _textures[3]; ///< All 3 textures
                };
                ui32 _texDepth = 0; ///< Depth texture of GBuffer
            };
		}
    }
}
namespace vg = vorb::core::graphics;

#endif // GBuffer_h__