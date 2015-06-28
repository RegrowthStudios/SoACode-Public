#pragma once

#include "IRenderStage.h"

#define SSAO_NOISE_TEXTURE_SIZE 32

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/Texture.h>


class SsaoRenderStage : public IRenderStage
{
public:
    void hook(vg::FullQuadVBO* quad);

    virtual void dispose(StaticLoadContext& context) override;

    /// Draws the render stage
    virtual void render(const Camera* camera = nullptr) override;
private:
    vg::GLProgram m_ssaoShader; ///< SSAO effect
    vg::GLProgram m_blurShader; ///< Blurring to reduce noise
    vg::FullQuadVBO* m_quad; ///< For use in processing through data
    vg::Texture m_texNoise; ///< A noise texture to make low sample amounts less obvious
};