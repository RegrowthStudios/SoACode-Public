#include "stdafx.h"
#include "SsaoRenderStage.h"

#include <Vorb/Random.h>
#include <Vorb/graphics/SamplerState.h>
#include <glm/gtc/type_ptr.hpp>

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, m_texNoise.width, m_texNoise.height, 0, GL_RG, GL_FLOAT, data);
    vg::SamplerState::POINT_WRAP.set(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] data;

    m_ssaoTarget.setSize(width, height);
    m_ssaoTarget.init(vg::TextureInternalFormat::R32F);
    
    for (unsigned int i = 0; i < SSAO_SAMPLE_KERNEL_SIZE; i++) {
		m_sampleKernel.emplace_back((f32)(r.genMT() * 2.0f - 1.0f), (f32)(r.genMT() * 2.0f - 1.0f), (f32)(r.genMT() * 2.0f - 1.0f));
        float random = r.genMT();
        m_sampleKernel[i] = glm::normalize(m_sampleKernel[i]) * (random * random + 0.1f);
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

    { // SSAO pass      
        m_ssaoTarget.use();
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_normalTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_texNoise.id);

        // Lazy shader init
        // TODO(Ben): Make it not lazy
        if (!m_ssaoShader.isCreated()) {
            m_ssaoShader = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/SSAO.frag");
            m_ssaoShader.use();
            glUniform1i(m_ssaoShader.getUniform("unTexDepth"), 0);
            glUniform1i(m_ssaoShader.getUniform("unTexNormal"), 1);
            glUniform1i(m_ssaoShader.getUniform("unTexNoise"), 2);
            glUniform3fv(glGetUniformLocation(m_ssaoShader.getID(), "unSampleKernel"), m_sampleKernel.size(), &m_sampleKernel.data()->x);
            glUniform2f(m_ssaoShader.getUniform("unSSAOTextureSize"), (float)m_ssaoTarget.getWidth(), (float)m_ssaoTarget.getHeight());
			glUniformMatrix4fv(m_ssaoShader.getUniform("unProjectionMatrix"), 1, false, glm::value_ptr(m_projectionMatrix));
            glUniformMatrix4fv(m_ssaoShader.getUniform("unInvProjectionMatrix"), 1, false, glm::value_ptr(glm::inverse(m_projectionMatrix)));
        }
        else {
            m_ssaoShader.use();
        }

        m_quad->draw();
    }

    { // Blur pass
        // Bind hdr framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFrameBuffer);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_colorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_ssaoTarget.getTextureID());

        // Lazy shader init
        // TODO(Ben): Make it not lazy
        if (!m_blurShader.isCreated()) {
            m_blurShader = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/SSAOBlur.frag");
            m_blurShader.use();
            glUniform1i(m_blurShader.getUniform("unTexColor"), 0);
            glUniform1i(m_blurShader.getUniform("unTexSSAO"), 1);
            glUniform1f(m_blurShader.getUniform("unBlurAmount"), (float)SSAO_BLUR_AMOUNT);
        } else {
            m_blurShader.use();
        }

        m_quad->draw();

        vg::GLProgram::unuse();
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
}

void SsaoRenderStage::reloadShaders()
{
    m_ssaoShader.dispose();
    m_blurShader.dispose();
}