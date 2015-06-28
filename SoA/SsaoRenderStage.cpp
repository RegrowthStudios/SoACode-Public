#include "stdafx.h"
#include "SsaoRenderStage.h"

#include <Vorb/Random.h>

void SsaoRenderStage::hook(vg::FullQuadVBO* quad) {
    m_quad = quad;
    m_texNoise.width = SSAO_NOISE_TEXTURE_SIZE;
    m_texNoise.height = SSAO_NOISE_TEXTURE_SIZE;

    // Generate random data
    i32 pixCount = m_texNoise.width * m_texNoise.height;
    f32v2* data = new f32v2[pixCount];
    Random r(clock());
    for (i32 i = 0; i < pixCount; i++) {
        data[i].x = (f32)(r.genMT() * 2.0f - 1.0f);
        data[i].y = (f32)(r.genMT() * 2.0f - 1.0f);
        data[i] = glm::normalize(data[i]);
    }

    // Build noise texture
    glGenTextures(1, &m_texNoise.id);
    glBindTexture(GL_TEXTURE_2D, m_texNoise.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, m_texNoise.width, m_texNoise.height, 0, GL_RG32F, GL_UNSIGNED_BYTE, data);
    //vg::SamplerState::POINT_WRAP.set(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] data;
}

void SsaoRenderStage::dispose(StaticLoadContext& context)
{
    if (m_texNoise.id) {
        glDeleteTextures(1, &m_texNoise.id);
        m_texNoise.id = 0;
    }
}

void SsaoRenderStage::render(const Camera* camera)
{

}