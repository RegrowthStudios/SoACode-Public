#include "stdafx.h"
#include "SsaoRenderStage.h"

#include <Vorb/Random.h>

#include "Errors.h"
#include "ShaderLoader.h"

void SsaoRenderStage::hook(vg::FullQuadVBO* quad, unsigned int width, unsigned int height) {
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, m_texNoise.width, m_texNoise.height, 0, GL_RG, GL_FLOAT, data);
    //vg::SamplerState::POINT_WRAP.set(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] data;

    m_ssaoTarget.setSize(width, height);
    m_ssaoTarget.init(vg::TextureInternalFormat::R8);
    
    for (unsigned int i = 0; i < SSAO_SAMPLE_KERNEL_SIZE; i++) {
        m_sampleKernel.emplace_back((f32)(r.genMT() * 2.0f - 1.0f), (f32)(r.genMT() * 2.0f - 1.0f), (f32)r.genMT());
        m_sampleKernel[i] = glm::normalize(m_sampleKernel[i]) * r.genMT();
    }
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
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    m_ssaoTarget.use();

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_texNoise.id);

    if (!m_ssaoShader.isCreated()) {
        m_ssaoShader = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/SSAO.frag");
        m_ssaoShader.use();
      //  glUniform1i(m_ssaoShader.getUniform("unTexDepth"), 0);
     //   glUniform1i(m_ssaoShader.getUniform("unTexNormal"), 1);
      //  glUniform1i(m_ssaoShader.getUniform("unTexNoise"), 2);
      //  glUniform3fv(m_ssaoShader.getUniform("unSampleKernel"), m_sampleKernel.size(), &m_sampleKernel[0].x);
    }
    else {
        m_ssaoShader.use();
    }

    m_ssaoShader.use();
    m_quad->draw();
    
    glActiveTexture(GL_TEXTURE0);
    m_swapChain->swap();
    m_swapChain->use(0, false);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_ssaoTarget.getTextureID());

    if (!m_blurShader.isCreated()) {
        m_blurShader = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/SSAOBlur.frag");
        m_blurShader.use();
        glUniform1i(m_blurShader.getUniform("unTexColor"), 0);
        glUniform1i(m_blurShader.getUniform("unTexSSAO"), 1);
        glUniform1f(m_blurShader.getUniform("unBlurAmount"), 1.0f);
    }
    else {
        m_blurShader.use();
    }

    m_ssaoShader.enableVertexAttribArrays();
    m_quad->draw();
    m_ssaoShader.disableVertexAttribArrays();

    vg::GLProgram::unuse();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
}