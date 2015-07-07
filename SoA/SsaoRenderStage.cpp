#include "stdafx.h"
#include "SSAORenderStage.h"

#include <Vorb/Random.h>
#include <Vorb/graphics/SamplerState.h>
#include <Vorb/utils.h>

#include <glm/gtc/type_ptr.hpp>
#include <random>

#include "Errors.h"
#include "Camera.h"
#include "ShaderLoader.h"

void SSAORenderStage::hook(vg::FullQuadVBO* quad, unsigned int width, unsigned int height) {
    std::mt19937 randGenerator;
    std::uniform_real_distribution<f32> range1(-1.0f, 1.0f);
    std::uniform_real_distribution<f32> range2(0.0f, 1.0f);
    
    m_quad = quad;
    m_texNoise.width = SSAO_NOISE_TEXTURE_SIZE;
    m_texNoise.height = SSAO_NOISE_TEXTURE_SIZE;

    // Generate random data
    i32 pixCount = m_texNoise.width * m_texNoise.height;
    f32v2* data = new f32v2[pixCount];
    Random r(clock());
    for (i32 i = 0; i < pixCount; i++) {
        // TODO(Ben): vec3?
        data[i].x = range1(randGenerator);
        data[i].y = range1(randGenerator);
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
    
    m_sampleKernel.resize(SSAO_SAMPLE_KERNEL_SIZE);
    for (unsigned int i = 0; i < SSAO_SAMPLE_KERNEL_SIZE; i++) {
        m_sampleKernel[i] = glm::normalize(f32v3(range1(randGenerator),
                                           range1(randGenerator),
                                           range2(randGenerator)));
        // Use accelerating interpolation
        f32 scale = (f32)i / (f32)SSAO_SAMPLE_KERNEL_SIZE;
        scale = lerp(0.1f, 1.0f, scale * scale);
        m_sampleKernel[i] *= scale;
    }
}

void SSAORenderStage::dispose(StaticLoadContext& context)
{
    if (m_texNoise.id) {
        glDeleteTextures(1, &m_texNoise.id);
        m_texNoise.id = 0;
    }
    m_ssaoShader.dispose();
    m_ssaoTarget.dispose();
    m_blurShader.dispose();
}

void SSAORenderStage::render(const Camera* camera)
{
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    const glm::mat4& projectionMatrix = camera->getProjectionMatrix();
    const glm::mat4& viewMatrix = camera->getViewMatrix();

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
            glUniform2f(m_ssaoShader.getUniform("unNoiseScale"),
                        (f32)m_ssaoTarget.getWidth() / SSAO_NOISE_TEXTURE_SIZE,
                        (f32)m_ssaoTarget.getHeight() / SSAO_NOISE_TEXTURE_SIZE);
            
        }
        else {
            m_ssaoShader.use();
        }

        glUniformMatrix4fv(m_ssaoShader.getUniform("unViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(m_ssaoShader.getUniform("unProjectionMatrix"), 1, false, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(m_ssaoShader.getUniform("unInvProjectionMatrix"), 1, false, glm::value_ptr(glm::inverse(projectionMatrix)));

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

void SSAORenderStage::reloadShaders()
{
    m_ssaoShader.dispose();
    m_blurShader.dispose();
}