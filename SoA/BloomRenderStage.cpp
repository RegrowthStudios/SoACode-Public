#include "stdafx.h"

#include "BloomRenderStage.h"

#include <sstream>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/GLProgram.h>
#include "ShaderLoader.h"
#include "LoadContext.h"
#include "Errors.h"
#include "Vorb/ui/GameWindow.h"

#define TASK_WORK  4                     // (arbitrary) weight of task
#define TOTAL_TASK 4                    // number of tasks
#define TOTAL_WORK TOTAL_TASK * TASK_WORK

#define BLOOM_TEXTURE_SLOT_COLOR  0      // texture slot to bind color texture which luma info will be extracted
#define BLOOM_TEXTURE_SLOT_LUMA   0       // texture slot to bind luma texture
#define BLOOM_TEXTURE_SLOT_BLUR   1       // texture slot to bind blur texture

float BloomRenderStage::gauss(int i, float sigma2) {
    return 1.0 / std::sqrt(2 * 3.14159265 * sigma2) * std::exp(-(i*i) / (2 * sigma2));
}

void BloomRenderStage::init(vui::GameWindow* window, StaticLoadContext& context) {
    
    IRenderStage::init(window, context);
    context.addAnticipatedWork(TOTAL_WORK, TOTAL_TASK);

    // initialize FBOs
    m_fbo1.setSize(m_window->getWidth(), m_window->getHeight());
    m_fbo2.setSize(m_window->getWidth(), m_window->getHeight());
    m_fbo1.init(vorb::graphics::TextureInternalFormat::RGBA16, 0, vorb::graphics::TextureFormat::RGBA, vorb::graphics::TexturePixelType::UNSIGNED_BYTE);
    m_fbo2.init(vorb::graphics::TextureInternalFormat::RGBA16, 0, vorb::graphics::TextureFormat::RGBA, vorb::graphics::TexturePixelType::UNSIGNED_BYTE);

}

void BloomRenderStage::setParams(ui32 gaussianN /* = 20 */, float gaussian_variance /* = 36.0f */, float luma_threshold /* = 0.75f */) {
    if (gaussianN > 50)
        // if a bigger radius than 50 is desired, change the size of weights array in this file and the blur shaders
        throw "Gaussian Radius for BloomRenderStage::setParams has to be less than 50.";
    m_gaussianN = gaussianN;
    m_gaussian_variance = gaussian_variance;
    m_luma_threshold = luma_threshold;
}

void BloomRenderStage::load(StaticLoadContext& context) {

    // luma
    context.addTask([&](Sender, void*) {
        m_program_luma = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/BloomLuma.frag");
        m_program_luma.use();
        glUniform1i(m_program_luma.getUniform("unTexColor"), BLOOM_TEXTURE_SLOT_COLOR);
        glUniform1f(m_program_luma.getUniform("unLumaThresh"), m_luma_threshold);
        m_program_luma.unuse();

        context.addWorkCompleted(TOTAL_TASK);
    }, false);

    // gaussian first pass
    context.addTask([&](Sender, void*) {
        m_program_gaussian_first = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/BloomGaussianFirst.frag");
        m_program_gaussian_first.use();
        glUniform1i(m_program_gaussian_first.getUniform("unTexLuma"), BLOOM_TEXTURE_SLOT_LUMA);
        glUniform1i(m_program_gaussian_first.getUniform("unHeight"), m_window->getHeight());
        glUniform1i(m_program_gaussian_first.getUniform("unGaussianN"), m_gaussianN);
        m_program_gaussian_first.unuse();
        context.addWorkCompleted(TOTAL_TASK);
    }, true);

    // gaussian second pass
    context.addTask([&](Sender, void*) {
        m_program_gaussian_second = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/BloomGaussianSecond.frag");
        m_program_gaussian_second.use();
        glUniform1i(m_program_gaussian_second.getUniform("unTexColor"), BLOOM_TEXTURE_SLOT_COLOR);
        glUniform1i(m_program_gaussian_second.getUniform("unTexBlur"), BLOOM_TEXTURE_SLOT_BLUR);
        glUniform1i(m_program_gaussian_second.getUniform("unWidth"), m_window->getWidth());
        glUniform1i(m_program_gaussian_second.getUniform("unGaussianN"), m_gaussianN);
        glUniform1f(m_program_gaussian_second.getUniform("unFinalWeight"), 1.0);
        m_program_gaussian_second.unuse();
        context.addWorkCompleted(TOTAL_TASK);
    }, true);

    // calculate gaussian weights
    
    context.addTask([&](Sender, void*) {
        float weights[50], sum;
        weights[0] = gauss(0, m_gaussian_variance);
        sum = weights[0];
        for (int i = 0; i < m_gaussianN; i++) {
            weights[i] = gauss(i, m_gaussian_variance);
            sum += 2 * weights[i];
        }
        for (int i = 0; i < m_gaussianN; i++) {
            weights[i] = weights[i] / sum;
        }
        m_program_gaussian_first.use();
        glUniform1fv(m_program_gaussian_first.getUniform("unWeight[0]"), m_gaussianN, weights);
        m_program_gaussian_first.unuse();
        m_program_gaussian_second.use();
        glUniform1fv(m_program_gaussian_second.getUniform("unWeight[0]"), m_gaussianN, weights);
        m_program_gaussian_second.unuse();

        context.addWorkCompleted(TOTAL_TASK);
    }, false);

}

void BloomRenderStage::hook(vg::FullQuadVBO* quad) {
    m_quad = quad;
}

void BloomRenderStage::dispose(StaticLoadContext& context) {
    m_program_luma.dispose();
    m_program_gaussian_first.dispose();
    m_program_gaussian_second.dispose();
}

void BloomRenderStage::setIntensity(float intensity /* = 1.0f */) {
    m_program_gaussian_second.use();
    glUniform1f(m_program_gaussian_second.getUniform("unFinalWeight"), intensity);
    m_program_gaussian_second.unuse();
}

void BloomRenderStage::render(const Camera* camera) {
    // get initial bound FBO and bound color texture to use it on final pass
    GLint initial_fbo, initial_texture;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &initial_fbo);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &initial_texture);

    // color texture should be bound on GL_TEXTURE0 slot

    // luma pass rendering on temporary FBO 1
    m_fbo1.use();
    glActiveTexture(GL_TEXTURE0 + BLOOM_TEXTURE_SLOT_COLOR);
    glBindTexture(GL_TEXTURE_2D, initial_texture);
    render(BLOOM_RENDER_STAGE_LUMA);
    m_fbo1.unuse(m_window->getWidth(), m_window->getHeight());
    
    // first gaussian blur pass rendering on temporary FBO 2
    m_fbo2.use();
    glActiveTexture(GL_TEXTURE0 + BLOOM_TEXTURE_SLOT_LUMA);
    m_fbo1.bindTexture();
    render(BLOOM_RENDER_STAGE_GAUSSIAN_FIRST);
    m_fbo2.unuse(m_window->getWidth(), m_window->getHeight());

    // second gaussian blur pass rendering on initially FBO
    glActiveTexture(GL_TEXTURE0 + BLOOM_TEXTURE_SLOT_COLOR);
    glBindTexture(GL_TEXTURE_2D, initial_texture);
    glActiveTexture(GL_TEXTURE0 + BLOOM_TEXTURE_SLOT_BLUR);
    m_fbo2.bindTexture();
    glBindFramebuffer(GL_FRAMEBUFFER, initial_fbo);
    render(BLOOM_RENDER_STAGE_GAUSSIAN_SECOND);
    
}

void BloomRenderStage::render(BloomRenderStagePass stage) {
    switch (stage) {
        // luma
    case BLOOM_RENDER_STAGE_LUMA:
        m_program_luma.use();
        m_program_luma.enableVertexAttribArrays();

        glDisable(GL_DEPTH_TEST);
        m_quad->draw();
        glEnable(GL_DEPTH_TEST);

        m_program_luma.disableVertexAttribArrays();
        m_program_luma.unuse();
        break;

        // first gaussian pass
    case BLOOM_RENDER_STAGE_GAUSSIAN_FIRST:
        m_program_gaussian_first.use();
        m_program_gaussian_first.enableVertexAttribArrays();

        glDisable(GL_DEPTH_TEST);
        m_quad->draw();
        glEnable(GL_DEPTH_TEST);

        m_program_gaussian_first.disableVertexAttribArrays();
        m_program_gaussian_first.unuse();
        break;

        // second gaussian pass
    case BLOOM_RENDER_STAGE_GAUSSIAN_SECOND:
        m_program_gaussian_second.use();
        m_program_gaussian_second.enableVertexAttribArrays();

        glDisable(GL_DEPTH_TEST);
        m_quad->draw();
        glEnable(GL_DEPTH_TEST);

        m_program_gaussian_second.disableVertexAttribArrays();
        m_program_gaussian_second.unuse();
        break;
    }

}

