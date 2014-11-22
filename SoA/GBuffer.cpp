#include "stdafx.h"
#include "GBuffer.h"

#include "SamplerState.h"

vg::GBuffer::GBuffer(ui32 w /*= 0*/, ui32 h /*= 0*/) :
_size(w, h) {
    // Empty
}

void initTarget(const VGEnum& textureTarget, const ui32v2& _size, const ui32& texID, const ui32& msaa, const vg::TextureInternalFormat& format, const ui32& attachment) {
    glBindTexture(textureTarget, texID);
    if (msaa > 0) {
        glTexImage2DMultisample(textureTarget, msaa, (VGEnum)format, _size.x, _size.y, GL_FALSE);
    } else {
        glTexImage2D(textureTarget, 0, (VGEnum)format, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    SamplerState::POINT_CLAMP.set(textureTarget);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, textureTarget, texID, 0);
}
vg::GBuffer& vg::GBuffer::init(ui32 msaa /*= 0*/) {
    // Make the framebuffer
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // Determine what type of texture we need to create
    _msaa = msaa;
    _textureTarget = isMSAA() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    // Create texture targets
    glGenTextures(3, _textures);
    initTarget(_textureTarget, _size, _tex.depth, msaa, GBUFFER_INTERNAL_FORMAT_DEPTH, 0);
    initTarget(_textureTarget, _size, _tex.normal, msaa, GBUFFER_INTERNAL_FORMAT_NORMAL, 1);
    initTarget(_textureTarget, _size, _tex.color, msaa, GBUFFER_INTERNAL_FORMAT_COLOR, 2);

    // Set the output location for pixels
    VGEnum bufs[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, bufs);

    // Unbind used resources
    glBindTexture(_textureTarget, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // TODO: Change The Memory Usage Of The GPU

    return *this;
}
vg::GBuffer& vg::GBuffer::initDepth(TextureInternalFormat depthFormat /*= TextureInternalFormat::DEPTH_COMPONENT32*/) {
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
void vg::GBuffer::dispose() {
    // TODO: Change The Memory Usage Of The GPU

    if (_fbo != 0) {
        glDeleteFramebuffers(1, &_fbo);
        _fbo = 0;
    }
    if (_tex.color != 0) {
        glDeleteTextures(3, _textures);
        _tex = { 0, 0, 0 };
    }
    if (_texDepth != 0) {
        glDeleteTextures(1, &_texDepth);
        _texDepth = 0;
    }
}

void vg::GBuffer::use() {
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glViewport(0, 0, _size.x, _size.y);
}

