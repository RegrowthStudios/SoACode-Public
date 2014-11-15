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

vg::FrameBuffer::FrameBuffer(vg::TextureInternalFormat internalFormat, GLenum type, ui32 width, ui32 height, ui32 msaa) :
    _vbo(0),
    _ibo(0),
    _width(width),
    _height(height),
    fboSimple(width, height),
    fboMSAA(width, height),
    _msaa(msaa)
{
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    std::cout << "Creating framebuffer of width " << _width << " / " << maxTextureSize << std::endl;
    if (_width > maxTextureSize){
        pError("Framebuffer of width" + std::to_string(_width) + " exceeds maximum supported size of " + std::to_string(maxTextureSize));
    }

    fboSimple.init(internalFormat, vg::TextureInternalFormat::DEPTH_COMPONENT32, 0);
    if (_msaa > 0) {
        fboMSAA.init(internalFormat, vg::TextureInternalFormat::DEPTH_COMPONENT32, _msaa);
    }

    glGenVertexArrays(1, &quadVertexArrayID);
    glBindVertexArray(quadVertexArrayID);
    glGenBuffers(1, &quadVertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, quadVertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
    glBindVertexArray(0);
}
vg::FrameBuffer::~FrameBuffer()
{
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
void vg::FrameBuffer::bind() {
    if (_msaa > 0) {
        fboMSAA.use();
    } else {
        fboSimple.use();
    }
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

void vg::FrameBuffer::draw(const ui32v4& destViewport, i32 drawMode) {
    if (_vbo == 0) {
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(wholeScreenVertices), wholeScreenVertices, GL_STATIC_DRAW);
        glGenBuffers(1, &_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxDrawIndices), boxDrawIndices, GL_STATIC_DRAW);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Solid mode
    //need to blit the MSAA to a normal texture

    // Blit MSAA FBO To The Simple One
    if (_msaa > 0) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMSAA.getID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboSimple.getID());
        glBlitFramebuffer(0, 0, fboMSAA.getWidth(), fboMSAA.getHeight(), 0, 0, fboSimple.getWidth(), fboSimple.getHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    fboSimple.bindTexture();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Draw Command
    glViewport(destViewport.x, destViewport.y, destViewport.z, destViewport.w);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    // Get Rid Of State
    fboSimple.unbindTexture();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}