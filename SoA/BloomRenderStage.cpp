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
    m_fbo1.init(vorb::graphics::TextureInternalFormat::RGBA32F, 0, vorb::graphics::TextureFormat::RGBA, vorb::graphics::TexturePixelType::FLOAT);
    m_fbo2.init(vorb::graphics::TextureInternalFormat::RGBA32F, 0, vorb::graphics::TextureFormat::RGBA, vorb::graphics::TexturePixelType::FLOAT);

}

void BloomRenderStage::setParams(ui32 gaussianN /* = 20 */, float gaussianVariance /* = 36.0f */, float lumaThreshold /* = 0.75f */) {
    if (gaussianN > 50)
        // if a bigger radius than 50 is desired, change the size of weights array in this file and the blur shaders
        throw "Gaussian Radius for BloomRenderStage::setParams has to be less than 50.";
    m_gaussianN = gaussianN;
    m_gaussianVariance = gaussianVariance;
    m_lumaThreshold = lumaThreshold;
}

void BloomRenderStage::load(StaticLoadContext& context) {

    // luma
    context.addTask([&](Sender, void*) {
        m_programLuma = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/BloomLuma.frag");
        m_programLuma.use();
        glUniform1i(m_programLuma.getUniform("unTexColor"), BLOOM_TEXTURE_SLOT_COLOR);
        glUniform1f(m_programLuma.getUniform("unLumaThresh"), m_lumaThreshold);
        m_programLuma.unuse();

        context.addWorkCompleted(TOTAL_TASK);
    }, false);

    // gaussian first pass
    context.addTask([&](Sender, void*) {
        m_programGaussianFirst = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/BloomGaussianFirst.frag");
        m_programGaussianFirst.use();
        glUniform1i(m_programGaussianFirst.getUniform("unTexLuma"), BLOOM_TEXTURE_SLOT_LUMA);
        glUniform1i(m_programGaussianFirst.getUniform("unHeight"), m_window->getHeight());
        glUniform1i(m_programGaussianFirst.getUniform("unGaussianN"), m_gaussianN);
        m_programGaussianFirst.unuse();
        context.addWorkCompleted(TOTAL_TASK);
    }, true);

    // gaussian second pass
    context.addTask([&](Sender, void*) {
        m_programGaussianSecond = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/BloomGaussianSecond.frag");
        m_programGaussianSecond.use();
        glUniform1i(m_programGaussianSecond.getUniform("unTexColor"), BLOOM_TEXTURE_SLOT_COLOR);
        glUniform1i(m_programGaussianSecond.getUniform("unTexBlur"), BLOOM_TEXTURE_SLOT_BLUR);
        glUniform1i(m_programGaussianSecond.getUniform("unWidth"), m_window->getWidth());
        glUniform1i(m_programGaussianSecond.getUniform("unGaussianN"), m_gaussianN);
        m_programGaussianSecond.unuse();
        context.addWorkCompleted(TOTAL_TASK);
    }, true);

    // calculate gaussian weights

    context.addTask([&](Sender, void*) {
        float weights[50], sum;
        weights[0] = gauss(0, m_gaussianVariance);
        sum = weights[0];
        for (ui32 i = 1; i < m_gaussianN; i++) {
            weights[i] = gauss(i, m_gaussianVariance);
            sum += 2 * weights[i];
        }
        for (ui32 i = 0; i < m_gaussianN; i++) {
            weights[i] = weights[i] / sum;
        }
        m_programGaussianFirst.use();
        glUniform1fv(m_programGaussianFirst.getUniform("unWeight[0]"), m_gaussianN, weights);
        m_programGaussianFirst.unuse();
        m_programGaussianSecond.use();
        glUniform1fv(m_programGaussianSecond.getUniform("unWeight[0]"), m_gaussianN, weights);
        m_programGaussianSecond.unuse();

        context.addWorkCompleted(TOTAL_TASK);
    }, false);
}

void BloomRenderStage::hook(vg::FullQuadVBO* quad) {
    m_quad = quad;
}

void BloomRenderStage::dispose(StaticLoadContext& context) {
    m_programLuma.dispose();
    m_programGaussianFirst.dispose();
    m_programGaussianSecond.dispose();
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
        m_programLuma.use();
        m_programLuma.enableVertexAttribArrays();

        glDisable(GL_DEPTH_TEST);
        m_quad->draw();
        glEnable(GL_DEPTH_TEST);

        m_programLuma.disableVertexAttribArrays();
        m_programLuma.unuse();
        break;

        // first gaussian pass
    case BLOOM_RENDER_STAGE_GAUSSIAN_FIRST:
        m_programGaussianFirst.use();
        m_programGaussianFirst.enableVertexAttribArrays();

        glDisable(GL_DEPTH_TEST);
        m_quad->draw();
        glEnable(GL_DEPTH_TEST);

        m_programGaussianFirst.disableVertexAttribArrays();
        m_programGaussianFirst.unuse();
        break;

        // second gaussian pass
    case BLOOM_RENDER_STAGE_GAUSSIAN_SECOND:
        m_programGaussianSecond.use();
        m_programGaussianSecond.enableVertexAttribArrays();

        glDisable(GL_DEPTH_TEST);
        m_quad->draw();
        glEnable(GL_DEPTH_TEST);

        m_programGaussianSecond.disableVertexAttribArrays();
        m_programGaussianSecond.unuse();
        break;
    }

}

