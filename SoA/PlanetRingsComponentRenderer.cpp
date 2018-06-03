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

PlanetRingsComponentRenderer::~PlanetRingsComponentRenderer() {
    dispose();
}

void PlanetRingsComponentRenderer::initGL() {
    if (!m_program.isCreated()) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/PlanetRings/Rings.vert",
                                                        "Shaders/PlanetRings/Rings.frag");
    }
    if (!m_isInitialized) {
        m_isInitialized = true;
        m_quad.init();
    }
}

void PlanetRingsComponentRenderer::draw(const PlanetRingsComponent& prCmp,
                                        vecs::EntityID eid,
                                        const f32m4& VP,
                                        const f32v3& relCamPos,
                                        const f32v3& lightPos,
                                        const f32 planetRadius,
                                        const f32 zCoef,
                                        const SpaceLightComponent* spComponent) {
    // Get renderables
    // TODO(Ben): Use a renderable component instead
    std::vector<RenderableRing>* rings;
    auto& it = m_renderableRings.find(eid);
    if (it == m_renderableRings.end()) {
        // Look how ugly this line is.
        rings = &m_renderableRings.insert(std::make_pair(eid, std::vector<RenderableRing>(prCmp.rings.size()))).first->second;
        for (size_t i = 0; i < prCmp.rings.size(); i++) {
            auto& rr = rings->operator[](i);
            rr.ring = prCmp.rings[i];
            // Load the texture
            vg::ScopedBitmapResource b = vg::ImageIO().load(rr.ring.texturePath);
            if (b.data) {
                rr.texture = vg::GpuMemory::uploadTexture(&b, vg::TexturePixelType::UNSIGNED_BYTE,
                                                         vg::TextureTarget::TEXTURE_2D,
                                                         &vg::SamplerState::LINEAR_CLAMP);
            } else {
                fprintf(stderr, "Failed to load %s\n", rr.ring.texturePath.getCString());
            }
        }
    } else {
        rings = &it->second;
    }


    m_program.use();
    // For logarithmic Z buffer
    glUniform1f(m_program.getUniform("unZCoef"), zCoef);

    glDisable(GL_CULL_FACE);
    // Set up matrix
    for (auto& r : (*rings)) {

        f64q invOrientation = vmath::inverse(r.ring.orientation);

        // Convert f64q to f32q
        f32q orientationF32;
        orientationF32.x = (f32)r.ring.orientation.x;
        orientationF32.y = (f32)r.ring.orientation.y;
        orientationF32.z = (f32)r.ring.orientation.z;
        orientationF32.w = (f32)r.ring.orientation.w;
        // Convert to matrix
        f32m4 rotationMatrix = vmath::toMat4(orientationF32);

        f32m4 W(1.0);
        setMatrixScale(W, f32v3(r.ring.outerRadius));
        setMatrixTranslation(W, -relCamPos);
        W *= rotationMatrix;
        f32m4 WVP = VP * W;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r.texture);

        glUniformMatrix4fv(m_program.getUniform("unM"), 1, GL_FALSE, &W[0][0]);
        glUniformMatrix4fv(m_program.getUniform("unMVP"), 1, GL_FALSE, &WVP[0][0]);
        glUniform1f(m_program.getUniform("unInnerRadius"), r.ring.innerRadius);
        glUniform1f(m_program.getUniform("unOuterRadius"), r.ring.outerRadius);
        glUniform1i(m_program.getUniform("unColorLookup"), 0);
        glUniform3fv(m_program.getUniform("unLightPos"), 1, &lightPos[0]);
        f32v3 planetPos = -relCamPos;
        glUniform3fv(m_program.getUniform("unPlanetPos"), 1, &planetPos[0]);
        glUniform1f(m_program.getUniform("unPlanetRadius"), planetRadius);

        m_quad.draw();
    }
    glEnable(GL_CULL_FACE);
    m_program.unuse();
}

void PlanetRingsComponentRenderer::dispose() {
    if (m_program.isCreated()) m_program.dispose();
    if (m_isInitialized) {
        m_quad.dispose();
        m_isInitialized = false;
    }
}
