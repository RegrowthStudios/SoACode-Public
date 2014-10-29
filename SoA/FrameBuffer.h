

#pragma once

#ifndef FrameBuffer_h__
#define FrameBuffer_h__

namespace vorb {
    namespace core {
        namespace graphics {

            class FrameBuffer
            {
            public:
                FrameBuffer(i32 internalFormat, GLenum type, ui32 width, ui32 height, ui32 msaa = 0);
                ~FrameBuffer();
                void bind();
                void unBind(const ui32v2& viewportDimensions);
                void checkErrors(nString name = "Frame Buffer");

                void draw(const ui32v2& destViewportDimensions, i32 drawMode);

#define FB_DRAW 0
#define FB_MSAA 1

                ui32 frameBufferIDs[2]; //color
                ui32 depthTextureIDs[2];
                ui32 quadVertexArrayID;
                ui32 quadVertexBufferID;
                ui32 renderedTextureIDs[2];

                // Getters
                ui32 getWidth() const { return _width; }
                ui32 getHeight() const { return _height; }

            private:
                ui32 _vbo;
                ui32 _ibo;
                ui32 _width, _height;
                ui32 _msaa;
            };
        }
    }
}

namespace vg = vorb::core::graphics;

#endif // FrameBuffer_h__