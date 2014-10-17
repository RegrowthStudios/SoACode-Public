#pragma once

class FrameBuffer
{
public:
    FrameBuffer(i32 internalFormat, GLenum type, ui32 width, ui32 height, i32 msaa = 0);
    ~FrameBuffer();

    void bind();
    void unBind(const ui32v2& viewportDimensions);
    void checkErrors(nString name = "Frame Buffer");

    void draw(const ui32v2& destDimensions, i32 drawMode = 0);

#define FB_DRAW 0
#define FB_MSAA 1

    // Getters
    ui32 getWidth() const { return _width; }
    ui32 getHeight() const { return _height; }

    ui32 frameBufferIDs[2]; 
    ui32 depthTextureIDs[2];
    ui32 quadVertexArrayID;
    ui32 quadVertexBufferID;
    ui32 renderedTextureIDs[2];
private:
    ui32 _vbo;
    ui32 _ibo;
    ui32 _width, _height;
    i32 _msaa;
};