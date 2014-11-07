#include "stdafx.h"
#include "GLRenderTarget.h"

#include "SamplerState.h"

vg::GLRenderTarget::GLRenderTarget(ui32 w /*= 0*/, ui32 h /*= 0*/) :
_size(w, h) {
    // Empty
}

vg::GLRenderTarget& vg::GLRenderTarget::init(
    vg::TextureInternalFormat format /*= TextureInternalFormat::RGBA8*/, 
    vg::TextureInternalFormat depthStencilFormat /*= TextureInternalFormat::DEPTH24_STENCIL8*/,
    ui32 msaa /*= 0*/) {
    // Make the framebuffer
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // Determine what type of texture we need to create
    textureTarget = msaa > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    // Create color target
    glGenTextures(1, &_texColor);
    glBindTexture(textureTarget, _texColor);
    if (msaa > 0) {
        glTexImage2DMultisample(textureTarget, msaa, (VGEnum)format, _size.x, _size.y, GL_FALSE);
    } else {
        glTexImage2D(textureTarget, 0, (VGEnum)format, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    SamplerState::POINT_CLAMP.set(textureTarget);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureTarget, _texColor, 0);

    // Create depth and stencil target
    if (depthStencilFormat != vg::TextureInternalFormat::NONE) {
        glGenTextures(1, &_texDS);
        glBindTexture(textureTarget, _texDS);
        if (msaa > 0) {
            glTexImage2DMultisample(textureTarget, msaa, (VGEnum)depthStencilFormat, _size.x, _size.y, GL_FALSE);
        } else {
            glTexImage2D(textureTarget, 0, (VGEnum)depthStencilFormat, _size.x, _size.y, 0, (VGEnum)vg::TextureFormat::DEPTH_COMPONENT, (VGEnum)vg::TexturePixelType::FLOAT, nullptr);
        }
        SamplerState::POINT_CLAMP.set(textureTarget);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureTarget, _texDS, 0);
    }

    // Set the output location for pixels
    ui32 drawTarget = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawTarget);

    // Unbind used resources
    glBindTexture(textureTarget, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

void vg::GLRenderTarget::bindTexture() const {
    glBindTexture(textureTarget, _texColor);
}
void vg::GLRenderTarget::unbindTexture() const {
    glBindTexture(textureTarget, 0);
}

void vg::GLRenderTarget::setSize(const ui32& w, const ui32& h) {
    // Don't change size if initialized already
    if (_fbo != 0) {
        // "Silent failure"
#ifdef DEBUG
        std::cerr << "Cannot Change Size Of An FBO That Is Already Initialized" << std::endl;
#endif // DEBUG
    } else {
        _size.x = w;
        _size.y = h;
    }
}
