#include "stdafx.h"
#include "LogLuminanceRenderStage.h"

#include "ShaderLoader.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/ShaderManager.h>
#include <Vorb/graphics/SamplerState.h>

LogLuminanceRenderStage::LogLuminanceRenderStage(vg::FullQuadVBO* quad, vg::GLRenderTarget* hdrFrameBuffer,
                                                 const ui32v4* viewPort, ui32 resolution) :
    m_quad(quad),
    m_hdrFrameBuffer(hdrFrameBuffer),
    m_restoreViewport(viewPort),
    m_resolution(resolution) {
    ui32 size = resolution;
    m_mipLevels = 1;
    while (size > 1) {
        m_mipLevels++;
        size >>= 1;
    }
    m_mipStep = 0;
}

void LogLuminanceRenderStage::reloadShader() {
    dispose();
}

void LogLuminanceRenderStage::dispose() {
    m_mipStep = 0;
    if (m_program) vg::ShaderManager::destroyProgram(&m_program);
    if (m_downsampleProgram) vg::ShaderManager::destroyProgram(&m_downsampleProgram);
    for (int i = 0; i < m_renderTargets.size(); i++) {
        m_renderTargets[i].dispose();
    }
    m_renderTargets.clear();
}

void LogLuminanceRenderStage::render() {
    if (m_renderTargets.empty()) {
        m_renderTargets.resize(m_mipLevels);
        for (int i = 0; i < m_mipLevels; i++) {
            ui32 res = m_resolution >> i;
            m_renderTargets[i].setSize(res, res);
            m_renderTargets[i].init(vg::TextureInternalFormat::RGBA16F);
        }    
    }
    // Lazy shader load
    if (!m_program) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                        "Shaders/PostProcessing/LogLuminance.frag");
        m_program->use();
        glUniform1i(m_program->getUniform("unTex"), 0);
        m_program->unuse();
    }
    if (!m_downsampleProgram) {
        m_downsampleProgram = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                        "Shaders/PostProcessing/LumDownsample.frag");
        m_downsampleProgram->use();
        glUniform1i(m_downsampleProgram->getUniform("unTex"), 0);
        m_downsampleProgram->unuse();
    }

    vg::GLProgram* prog = nullptr;
    if (m_mipStep == m_mipLevels-1) {
        // Final Step
        m_renderTargets[m_mipStep].bindTexture();
        m_mipStep = 1;
        f32v4 pixel;
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &pixel[0]);
        m_avgLuminance = glm::exp(pixel.r);
        printf("%f  %f\n", m_avgLuminance, glm::exp(pixel.g));
        prog = m_program;
        m_hdrFrameBuffer->bindTexture();
    } else if (m_mipStep > 0) {
        prog = m_downsampleProgram;
        m_renderTargets[m_mipStep].bindTexture();
        m_mipStep++;
    } else {
        prog = m_program;
        m_hdrFrameBuffer->bindTexture();
        m_mipStep++;
    }

    glActiveTexture(GL_TEXTURE0);
    // If we have rendered a frame before, generate mips and get lowest level

    m_renderTargets[m_mipStep].use();

    prog->use();
    prog->enableVertexAttribArrays();

    m_quad->draw();

    prog->disableVertexAttribArrays();
    prog->unuse();

    m_renderTargets[m_mipStep].unuse(m_restoreViewport->z, m_restoreViewport->w);
}
