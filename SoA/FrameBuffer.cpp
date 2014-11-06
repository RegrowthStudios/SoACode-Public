#include "stdafx.h"
#include "FrameBuffer.h"

#include "Errors.h"

static const f32 g_quad_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
};

static const f32 wholeScreenVertices[] = { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };

static const ui16 boxDrawIndices[6] = { 0, 1, 2, 2, 3, 0 };

vg::FrameBuffer::FrameBuffer(i32 internalFormat, GLenum type, ui32 width, ui32 height, ui32 msaa) :
    _vbo(0),
    _ibo(0),
    _width(width),
    _height(height),
    _msaa(msaa)
{
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    std::cout << "Creating framebuffer of width " << _width << " / " << maxTextureSize << std::endl;
    if (_width > maxTextureSize){
        pError("Framebuffer of width" + std::to_string(_width) + " exceeds maximum supported size of " + std::to_string(maxTextureSize));
    }

    frameBufferIDs[FB_DRAW] = 0;
    frameBufferIDs[FB_MSAA] = 0;
    renderedTextureIDs[FB_DRAW] = 0;
    renderedTextureIDs[FB_MSAA] = 0;
    depthTextureIDs[FB_DRAW] = 0;
    depthTextureIDs[FB_MSAA] = 0;

    //Initialize frameBuffer
    glGenFramebuffers(1, &(frameBufferIDs[FB_DRAW]));
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIDs[FB_DRAW]);
    // The texture we're going to render to
    glGenTextures(1, &renderedTextureIDs[FB_DRAW]);
    glBindTexture(GL_TEXTURE_2D, renderedTextureIDs[FB_DRAW]);
    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Give an empty image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, GL_BGRA, type, NULL);
    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTextureIDs[FB_DRAW], 0);
    // The depth buffer
    glGenTextures(1, &depthTextureIDs[FB_DRAW]);
    glBindTexture(GL_TEXTURE_2D, depthTextureIDs[FB_DRAW]);
    glBindTexture(GL_TEXTURE_2D, depthTextureIDs[FB_DRAW]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureIDs[FB_DRAW], 0);
    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    checkErrors();
    if (_msaa > 0){
        //Initialize frameBuffer
        glGenFramebuffers(1, &(frameBufferIDs[FB_MSAA]));
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIDs[FB_MSAA]);
        // The texture we're going to render to
        glGenTextures(1, &renderedTextureIDs[FB_MSAA]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderedTextureIDs[FB_MSAA]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _msaa, internalFormat, _width, _height, 0);
        GLuint err = glGetError();
        if (err != GL_NO_ERROR){
            showMessage(std::to_string((int)err));
        }
        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, renderedTextureIDs[FB_MSAA], 0);
        // The depth buffer
        glGenTextures(1, &depthTextureIDs[FB_MSAA]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureIDs[FB_MSAA]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureIDs[FB_MSAA]);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _msaa, GL_DEPTH_COMPONENT32, _width, _height, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTextureIDs[FB_MSAA], 0);
        glDrawBuffers(1, DrawBuffers);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        checkErrors("MSAA Frame Buffer");
    }
    glGenVertexArrays(1, &quadVertexArrayID);
    glBindVertexArray(quadVertexArrayID);
    glGenBuffers(1, &quadVertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, quadVertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
   
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
vg::FrameBuffer::~FrameBuffer()
{
    if (renderedTextureIDs[FB_DRAW] != 0){
        glDeleteTextures(1, &renderedTextureIDs[FB_DRAW]);
        renderedTextureIDs[FB_DRAW] = 0;
    }
    if (renderedTextureIDs[FB_MSAA] != 0){
        glDeleteTextures(1, &renderedTextureIDs[FB_MSAA]);
        renderedTextureIDs[FB_MSAA] = 0;
    }
    if (depthTextureIDs[FB_DRAW] != 0){
        glDeleteTextures(1, &depthTextureIDs[FB_DRAW]);
        depthTextureIDs[FB_DRAW] = 0;
    }
    if (depthTextureIDs[FB_MSAA] != 0){
        glDeleteTextures(1, &depthTextureIDs[FB_MSAA]);
        depthTextureIDs[FB_MSAA] = 0;
    }
    if (frameBufferIDs[FB_DRAW] != 0){
        glDeleteFramebuffers(1, &(frameBufferIDs[FB_DRAW]));
        frameBufferIDs[FB_DRAW] = 0;
    }
    if (frameBufferIDs[FB_MSAA] != 0){
        glDeleteFramebuffers(1, &(frameBufferIDs[FB_MSAA]));
        frameBufferIDs[FB_MSAA] = 0;
    }
    if (quadVertexArrayID != 0){
        glDeleteVertexArrays(1, &quadVertexArrayID);
        quadVertexArrayID = 0;
    }
    if (_vbo != 0){
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }
    if (_ibo != 0){
        glDeleteBuffers(1, &_ibo);
        _ibo = 0;
    }
   
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void vg::FrameBuffer::bind()
{
    if (_msaa > 0){
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIDs[FB_MSAA]);
    } else{
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIDs[FB_DRAW]);
    }
    glViewport(0, 0, _width, _height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
}

void vg::FrameBuffer::unBind(const ui32v2& viewportDimensions)
{
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewportDimensions.x, viewportDimensions.y);
}

void vg::FrameBuffer::checkErrors(nString name)
{
    checkGlError(name);
    // Always check that our framebuffer is ok
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE){
        nString err = "Unknown Error " + std::to_string((int)status);
        if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT){
            err = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        } else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT){
            err = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT";
        } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT){
            err = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        } else if (status == GL_FRAMEBUFFER_UNSUPPORTED){
            err = "GL_FRAMEBUFFER_UNSUPPORTED";
        } else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER){
            err = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE){
            err = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        }
        pError(name + " did not properly initialize: " + err);
        exit(414);
    }
}

void vg::FrameBuffer::draw(const ui32v4& destViewport, i32 drawMode)
{
    if (_vbo == 0){
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(wholeScreenVertices), wholeScreenVertices, GL_STATIC_DRAW);
        glGenBuffers(1, &_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxDrawIndices), boxDrawIndices, GL_STATIC_DRAW);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Solid mode
    //need to blit the MSAA to a normal texture
    if (_msaa > 0){
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferIDs[FB_MSAA]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferIDs[FB_DRAW]);
        glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderedTextureIDs[FB_DRAW]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(destViewport.x, destViewport.y, destViewport.z, destViewport.w); // Render on the whole screen, complete from the lower left corner to the upper right

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // Draw the triangles !
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}
