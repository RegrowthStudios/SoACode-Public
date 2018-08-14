#include "stdafx.h"
#include "ExposureCalcRenderStage.h"

#include "ShaderLoader.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/ShaderManager.h>
#include <Vorb/graphics/SamplerState.h>

#define EXPOSURE_FUNCTION_FILE "Shaders/PostProcessing/exposure.lua"
#define EXPOSURE_FUNCTION_NAME "calculateExposure"

ExposureCalcRenderStage::ExposureCalcRenderStage() {
    
}

ExposureCalcRenderStage::~ExposureCalcRenderStage() {
    delete m_scripts;
}

void ExposureCalcRenderStage::hook(vg::FullQuadVBO* quad, vg::GBuffer* hdrFrameBuffer,
                                   const ui32v4* viewPort, ui32 resolution) {
    if (!m_scripts) m_scripts = new vscript::Environment;
    m_quad = quad;
    m_hdrFrameBuffer = hdrFrameBuffer;
    m_restoreViewport = viewPort;
    m_resolution = resolution;
    ui32 size = resolution;
    m_mipLevels = 1;
    while (size > 1) {
        m_mipLevels++;
        size >>= 1;
    }
    m_mipStep = 0;
}

void ExposureCalcRenderStage::dispose(StaticLoadContext& context VORB_UNUSED) {
    m_mipStep = 0;
    if (m_program.isCreated()) m_program.dispose();
    if (m_downsampleProgram.isCreated()) m_downsampleProgram.dispose();
    for (size_t i = 0; i < m_renderTargets.size(); i++) {
        m_renderTargets[i].dispose();
    }
    m_renderTargets.clear();
    m_needsScriptLoad = true;
}

void ExposureCalcRenderStage::render(const Camera* camera VORB_UNUSED /*= nullptr*/) {
    if (m_renderTargets.empty()) {
        m_renderTargets.resize(m_mipLevels);
        for (size_t i = 0; i < m_mipLevels; i++) {
            ui32 res = m_resolution >> i;
            m_renderTargets[i].setSize(res, res);
            m_renderTargets[i].init(vg::TextureInternalFormat::RGBA16F);
        }    
    }
    // Lazy shader load
    if (!m_program.isCreated()) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                        "Shaders/PostProcessing/LogLuminance.frag");
        m_program.use();
        glUniform1i(m_program.getUniform("unTex"), 0);
        m_program.unuse();
    }
    if (!m_downsampleProgram.isCreated()) {
        m_downsampleProgram = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                        "Shaders/PostProcessing/LumDownsample.frag");
        m_downsampleProgram.use();
        glUniform1i(m_downsampleProgram.getUniform("unTex"), 0);
        m_downsampleProgram.unuse();
    }
    // Lazy script load
    if (m_needsScriptLoad) {
        m_scripts->load(EXPOSURE_FUNCTION_FILE);
        m_calculateExposure = (*m_scripts)[EXPOSURE_FUNCTION_NAME].as<f32>();
        m_needsScriptLoad = false;
    }

    vg::GLProgram* prog = nullptr;
    if (m_mipStep == (int)m_mipLevels-1) {
        // Final Step
        m_renderTargets[m_mipStep].bindTexture();
        m_mipStep = 1;
        f32v4 pixel = f32v4(0.0f);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &pixel[0]);

        // LUA SCRIPT
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        m_exposure = m_calculateExposure(pixel.r, pixel.g, pixel.b, pixel.a);
#pragma GCC diagnostic pop
#else
        m_exposure = m_calculateExposure(pixel.r, pixel.g, pixel.b, pixel.a);
#endif

        prog = &m_program;
        m_hdrFrameBuffer->bindGeometryTexture(0, 0);
    } else if (m_mipStep > 0) {
        prog = &m_downsampleProgram;
        m_renderTargets[m_mipStep].bindTexture();
        m_mipStep++;
    } else {
        prog = &m_program;
        m_hdrFrameBuffer->bindGeometryTexture(0, 0);
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
