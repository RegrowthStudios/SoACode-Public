#include "stdafx.h"
#include "TerrainGenTextures.h"

#include <SamplerState.h>
#include <GpuMemory.h>

TerrainGenTextures::~TerrainGenTextures() {
    destroy();
}

void TerrainGenTextures::init(const ui32v2& dims) {
    m_dims = dims;
    
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create texture targets
    glGenTextures(3, m_textures);
    initTarget(m_dims, m_tex.height, TERRAINGEN_INTERNAL_FORMAT_HEIGHT, 0);
    initTarget(m_dims, m_tex.temp, TERRAINGEN_INTERNAL_FORMAT_TEMP, 1);
    initTarget(m_dims, m_tex.hum, TERRAINGEN_INTERNAL_FORMAT_HUM, 2);
   
    // Set the output location for pixels
    VGEnum bufs[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, bufs);

    // Unbind used resources
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // TODO: Change The Memory Usage Of The GPU
}

void TerrainGenTextures::use() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_dims.x, m_dims.y);
}

void TerrainGenTextures::unuse() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TerrainGenTextures::destroy() {
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    if (m_tex.height != 0) {
        glDeleteTextures(3, m_textures);
        m_tex = { 0, 0, 0 };
    }
}

void TerrainGenTextures::initTarget(const ui32v2& size, const ui32& texID, const vg::TextureInternalFormat& format, const ui32& attachment) {
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, (VGEnum)format, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    SamplerState::POINT_CLAMP.set(GL_TEXTURE_2D);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, texID, 0);
}

NormalGenTexture::~NormalGenTexture() {
    destroy();
}

void NormalGenTexture::init(const ui32v2& dims) {
    m_dims = dims;

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create texture target
    glGenTextures(1, &m_normal);
    glBindTexture(GL_TEXTURE_2D, m_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, (VGEnum)NORMALGEN_INTERNAL_FORMAT, m_dims.x, m_dims.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    SamplerState::POINT_CLAMP.set(GL_TEXTURE_2D);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_normal, 0);
    
    // Set the output location for pixels
    VGEnum att = GL_COLOR_ATTACHMENT0;

    glDrawBuffers(1, &att);

    // Unbind used resources
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // TODO: Change The Memory Usage Of The GPU
}

void NormalGenTexture::use() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void NormalGenTexture::unuse() {

}

void NormalGenTexture::destroy() {
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    if (m_normal != 0) {
        glDeleteTextures(1, &m_normal);
        m_normal = 0;
    }
}
