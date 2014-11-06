#include "stdafx.h"
#include "GLRenderTarget.h"

vg::GLRenderTarget::GLRenderTarget(ui32 w, ui32 h) :
_size(w, h) {
    // Empty
}

vg::GLRenderTarget& vg::GLRenderTarget::init(ui32 format /*= GL_RGBA8*/, ui32 depthStencilFormat /*= GL_DEPTH24_STENCIL8*/, ui32 msaa /*= 0*/) {
    // Make The Frame Buffer
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // Determine What Type Of Texture We Need To Create
    ui32 texTarget = msaa > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    // Create Color Target
    glGenTextures(1, &_texColor);
    glBindTexture(texTarget, _texColor);
    if (msaa > 0) {
        glTexImage2DMultisample(texTarget, msaa, format, _size.x, _size.y, GL_FALSE);
    } else {
        glTexImage2D(texTarget, 0, format, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texTarget, _texColor, 0);

    // Create Depth And Stencil Target
    if (depthStencilFormat != 0) {
        glGenTextures(1, &_texDS);
        glBindTexture(texTarget, _texDS);
        if (msaa > 0) {
            glTexImage2DMultisample(texTarget, msaa, depthStencilFormat, _size.x, _size.y, GL_FALSE);
        } else {
            glTexImage2D(texTarget, 0, depthStencilFormat, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texTarget, _texDS, 0);
    }

    // Set The Output Location For Pixels
    ui32 drawTarget = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawTarget);

    // Unbind Used Resources
    glBindTexture(texTarget, _texColor);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // TODO: Change The Memory Usage Of The GPU

    return *this;
}
void vg::GLRenderTarget::dispose() {
    // TODO: Change The Memory Usage Of The GPU

    if (_fbo != 0) {
        glDeleteFramebuffers(1, &_fbo);
        _fbo = 0;
    }
    if (_texColor != 0) {
        glDeleteTextures(1, &_texColor);
        _texColor = 0;
    }
    if (_texDS != 0) {
        glDeleteTextures(1, &_texDS);
        _texDS = 0;
    }
}

void vg::GLRenderTarget::use() const {
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glViewport(0, 0, _size.x, _size.y);
}
void vg::GLRenderTarget::unuse(ui32 w, ui32 h) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, w, h);
}
