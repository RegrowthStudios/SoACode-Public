#include "stdafx.h"
#include "PlanetRingsComponentRenderer.h"

#include "SpaceSystem.h"
#include "RenderUtils.h"
#include "soaUtils.h"
#include "ShaderLoader.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/RasterizerState.h>
#include <Vorb/graphics/ShaderManager.h>

#include <glm/gtx/quaternion.hpp>

PlanetRingsComponentRenderer::PlanetRingsComponentRenderer() {
    // Empty
}

PlanetRingsComponentRenderer::~PlanetRingsComponentRenderer() {
    disposeShader();
    m_quad.dispose();
}

void PlanetRingsComponentRenderer::draw(const PlanetRingsComponent& prCmp,
                                        const f32m4& VP,
                                        const f32v3& relCamPos,
                                        const f32v3& lightPos,
                                        const f32 planetRadius,
                                        const SpaceLightComponent* spComponent) {
    // Lazily construct buffer and shaders
    if (!m_program) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/PlanetRings/Rings.vert",
                                                        "Shaders/PlanetRings/Rings.frag");
    }
    if (!m_isInitialized) {
        m_isInitialized = true;
        m_quad.init();
    }

    m_program->use();
    glDisable(GL_CULL_FACE);
    // Set up matrix
    for (auto& r : prCmp.rings) {

        f64q invOrientation = glm::inverse(r.orientation);

        // Convert f64q to f32q
        f32q orientationF32;
        orientationF32.x = (f32)r.orientation.x;
        orientationF32.y = (f32)r.orientation.y;
        orientationF32.z = (f32)r.orientation.z;
        orientationF32.w = (f32)r.orientation.w;
        // Convert to matrix
        f32m4 rotationMatrix = glm::toMat4(orientationF32);

        f32m4 W(1.0);
        setMatrixScale(W, f32v3(r.outerRadius));
        setMatrixTranslation(W, -relCamPos);
        W *= rotationMatrix;
        f32m4 WVP = VP * W;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r.colorLookup);

        glUniformMatrix4fv(m_program->getUniform("unM"), 1, GL_FALSE, &W[0][0]);
        glUniformMatrix4fv(m_program->getUniform("unMVP"), 1, GL_FALSE, &WVP[0][0]);
        glUniform1f(m_program->getUniform("unInnerRadius"), r.innerRadius);
        glUniform1f(m_program->getUniform("unOuterRadius"), r.outerRadius);
        glUniform1i(m_program->getUniform("unColorLookup"), 0);
        glUniform3fv(m_program->getUniform("unLightPos"), 1, &lightPos[0]);
        f32v3 planetPos = -relCamPos;
        glUniform3fv(m_program->getUniform("unPlanetPos"), 1, &planetPos[0]);
        glUniform1f(m_program->getUniform("unPlanetRadius"), planetRadius);

        m_quad.draw();
    }
    glEnable(GL_CULL_FACE);
    m_program->unuse();
}

void PlanetRingsComponentRenderer::disposeShader() {
    if (m_program) vg::ShaderManager::destroyProgram(&m_program);
}
