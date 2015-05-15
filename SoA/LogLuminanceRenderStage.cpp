#include "stdafx.h"
#include "LogLuminanceRenderStage.h"

#include "ShaderLoader.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/ShaderManager.h>

LogLuminanceRenderStage::LogLuminanceRenderStage(vg::FullQuadVBO* quad, vg::GLRenderTarget* hdrFrameBuffer,
                                                 const ui32v4* viewPort, ui32 resolution) :
    m_quad(quad),
    m_hdrFrameBuffer(hdrFrameBuffer),
    m_restoreViewport(viewPort),
    m_viewportDims(resolution, resolution) {
    ui32 size = resolution;
    while (size > 1) {
        m_mipLevels++;
        size >>= 1;
    }
}

void LogLuminanceRenderStage::reloadShader() {
    dispose();
}

void LogLuminanceRenderStage::dispose() {
    m_hasPrevFrame = false;
    if (m_program) {
        vg::ShaderManager::destroyProgram(&m_program);
    }
}

void LogLuminanceRenderStage::render() {
    if (!m_renderTarget) {
        m_renderTarget = new vg::GLRenderTarget(m_viewportDims.x, m_viewportDims.y);
        m_renderTarget->init(vg::TextureInternalFormat::RGBA16F);// .initDepth();
    }
    if (!m_program) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                        "Shaders/PostProcessing/LogLuminance.frag");
        m_program->use();
        glUniform1i(m_program->getUniform("unTex"), 0);
        m_program->unuse();
    }

    glActiveTexture(GL_TEXTURE0);
    // If we have rendered a frame before, generate mips and get lowest level
    if (m_hasPrevFrame) {
        m_renderTarget->bindTexture();
        glGenerateMipmap(GL_TEXTURE_2D);
        f32v4 pixel;
        glGetTexImage(GL_TEXTURE_2D, m_mipLevels, GL_RGBA, GL_FLOAT, &pixel[0]);
        float logLuminance = pixel.r;
        std::cout << "LOG LUM: " << logLuminance << std::endl;
    }

    m_hdrFrameBuffer->bindTexture();

    m_renderTarget->use();

    m_program->use();
    m_program->enableVertexAttribArrays();

    m_quad->draw();

    m_program->disableVertexAttribArrays();
    m_program->unuse();

    m_renderTarget->unuse(m_restoreViewport->z, m_restoreViewport->w);

    m_hasPrevFrame = true;
}
