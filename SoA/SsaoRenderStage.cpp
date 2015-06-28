#include "stdafx.h"
#include "SsaoRenderStage.h"

#include <Vorb/Random.h>

#include "ShaderLoader.h"

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

    m_ssaoTarget.init(vg::TextureInternalFormat::R8);
}

void SsaoRenderStage::dispose(StaticLoadContext& context)
{
    if (m_texNoise.id) {
        glDeleteTextures(1, &m_texNoise.id);
        m_texNoise.id = 0;
    }
    m_ssaoShader.dispose();
    m_ssaoTarget.dispose();
    m_blurShader.dispose();
}

void SsaoRenderStage::render(const Camera* camera)
{
    glDisable(GL_DEPTH_TEST);
    m_ssaoTarget.use();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texNoise.id);

    if (!m_ssaoShader.isCreated()) {
        m_ssaoShader = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/SSAO.frag");
    }

    m_ssaoShader.use();
    m_ssaoShader.enableVertexAttribArrays();
    m_quad->draw();
    m_ssaoShader.disableVertexAttribArrays();
    
    glActiveTexture(GL_TEXTURE0);
    m_swapChain->swap();
    m_swapChain->use(0, false);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_ssaoTarget.getTextureID());

    if (!m_blurShader.isCreated()) {
        m_blurShader = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/SSAOBlur.frag");
        glUniform1i(m_blurShader.getUniform("unTexColor"), 0);
        glUniform1i(m_blurShader.getUniform("unTexSSAO"), 1);
        glUniform1f(m_blurShader.getUniform("unBlurAmount"), 1.0f);
    }

    m_blurShader.use();
    m_ssaoShader.enableVertexAttribArrays();
    m_quad->draw();
    m_ssaoShader.disableVertexAttribArrays();

    vg::GLProgram::unuse();
    glEnable(GL_DEPTH_TEST);
}