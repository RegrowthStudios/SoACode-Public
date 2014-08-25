#pragma once

class FrameBuffer
{
public:
    FrameBuffer(i32 internalFormat, ui32 type, i32 width, i32 height, i32 msaa = 0);
    ~FrameBuffer();

    void bind(int msaa = 0);
    void unBind();
    void checkErrors(nString name = "Frame Buffer");
#define FB_SHADER_MOTIONBLUR 0x1

    void draw(int shaderMode);

#define FB_DRAW 0
#define FB_MSAA 1

    ui32 frameBufferIDs[2]; //color
    ui32 depthTextureIDs[2];
    ui32 quadVertexArrayID;
    ui32 quadVertexBufferID;
    ui32 renderedTextureIDs[2];
    ui32 vertexBufferID;
    ui32 elementBufferID;
    i32 fbWidth, fbHeight;
};