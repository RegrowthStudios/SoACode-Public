#include "stdafx.h"
#include "GLRenderTarget.h"

#include "SamplerState.h"

vg::GLRenderTarget::GLRenderTarget(ui32 w /*= 0*/, ui32 h /*= 0*/) :
_size(w, h) {
    // Empty
}

vg::GLRenderTarget& vg::GLRenderTarget::init(vg::TextureInternalFormat format /*= TextureInternalFormat::RGBA8*/, ui32 msaa /*= 0*/, vg::TextureFormat pFormat /*= TextureFormat::RGBA*/, vg::TexturePixelType pType /*= TexturePixelType::UNSIGNED_BYTE*/) {
    // Make the framebuffer
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // Determine what type of texture we need to create
    _msaa = msaa;
    _textureTarget = isMSAA() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    // Create color target
    glGenTextures(1, &_texColor);
    glBindTexture(_textureTarget, _texColor);
    if (msaa > 0) {
        glTexImage2DMultisample(_textureTarget, msaa, (VGEnum)format, _size.x, _size.y, GL_FALSE);
    } else {
        glTexImage2D(_textureTarget, 0, (VGEnum)format, _size.x, _size.y, 0, (VGEnum)pFormat, (VGEnum)pType, nullptr);
    }
    SamplerState::POINT_CLAMP.set(_textureTarget);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _textureTarget, _texColor, 0);

    // Set the output location for pixels
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // Unbind used resources
    glBindTexture(_textureTarget, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // TODO: Change The Memory Usage Of The GPU

    return *this;
}
vg::GLRenderTarget& vg::GLRenderTarget::initDepth(vg::TextureInternalFormat depthFormat /*= TextureInternalFormat::DEPTH_COMPONENT32F*/) {
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glGenTextures(1, &_texDepth);
    glBindTexture(_textureTarget, _texDepth);
    if (isMSAA()) {
        glTexImage2DMultisample(_textureTarget, _msaa, (VGEnum)depthFormat, _size.x, _size.y, GL_FALSE);
    } else {
        glTexImage2D(_textureTarget, 0, (VGEnum)depthFormat, _size.x, _size.y, 0, (VGEnum)vg::TextureFormat::DEPTH_COMPONENT, (VGEnum)vg::TexturePixelType::FLOAT, nullptr);
    }
    SamplerState::POINT_CLAMP.set(_textureTarget);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _textureTarget, _texDepth, 0);

    // Unbind used resources
    glBindTexture(_textureTarget, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // TODO: Change The Memory Usage Of The GPU

    return *this;
}
vg::GLRenderTarget& vg::GLRenderTarget::initDepthStencil(TextureInternalFormat stencilFormat /*= TextureInternalFormat::DEPTH24_STENCIL8*/) {
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glGenTextures(1, &_texDepth);
    glBindTexture(_textureTarget, _texDepth);
    if (isMSAA()) {
        glTexImage2DMultisample(_textureTarget, _msaa, (VGEnum)stencilFormat, _size.x, _size.y, GL_FALSE);
    } else {
        glTexImage2D(_textureTarget, 0, (VGEnum)stencilFormat, _size.x, _size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    }
    SamplerState::POINT_CLAMP.set(_textureTarget);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, _textureTarget, _texDepth, 0);

    // Unbind used resources
    glBindTexture(_textureTarget, 0);
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
    if (_texDepth != 0) {
        glDeleteTextures(1, &_texDepth);
        _texDepth = 0;
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
    glBindTexture(_textureTarget, _texColor);
}
void vg::GLRenderTarget::unbindTexture() const {
    glBindTexture(_textureTarget, 0);
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

