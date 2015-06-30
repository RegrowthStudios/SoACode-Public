#pragma once

#include "IRenderStage.h"

#define SSAO_NOISE_TEXTURE_SIZE 32
#define SSAO_SAMPLE_KERNEL_SIZE 16
#define SSAO_BLUR_AMOUNT 0

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/RTSwapChain.hpp>
#include <Vorb/graphics/Texture.h>

#include <vector>

class SsaoRenderStage : public IRenderStage
{
public:
    void hook(vg::FullQuadVBO* quad, unsigned int width, unsigned int height);

    virtual void dispose(StaticLoadContext& context) override;

    /// Draws the render stage
    virtual void render(const Camera* camera = nullptr) override;

    inline void set(VGTexture depthTexture, VGTexture normalTexture, VGTexture colorTexture, VGFramebuffer hdrFrameBuffer, const f32m4& projectionMatrix) { 
        m_depthTexture = depthTexture;
        m_normalTexture = normalTexture;
        m_colorTexture = colorTexture;
        m_hdrFrameBuffer = hdrFrameBuffer;
        m_projectionMatrix = projectionMatrix;
    }
    void reloadShaders();
private:
    vg::GLProgram m_ssaoShader; ///< SSAO effect
    vg::GLRenderTarget m_ssaoTarget; ///< SSAO render target
    vg::GLProgram m_blurShader; ///< Blurring to reduce noise
    vg::FullQuadVBO* m_quad; ///< For use in processing through data
    vg::Texture m_texNoise; ///< A noise texture to make low sample amounts less obvious
    VGTexture m_depthTexture;
    VGTexture m_normalTexture;
    VGTexture m_colorTexture;
    VGFramebuffer m_hdrFrameBuffer;
    std::vector<f32v3> m_sampleKernel;
    f32m4 m_projectionMatrix;
};