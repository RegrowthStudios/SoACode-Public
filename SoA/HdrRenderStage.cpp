#include "stdafx.h"
#include "HdrRenderStage.h"

#include <Vorb/graphics/ShaderManager.h>
#include "Camera.h"
#include "Chunk.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "MeshManager.h"
#include "SoaOptions.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"

void HdrRenderStage::hook(vg::FullQuadVBO* quad) {
    m_quad = quad;
}

void HdrRenderStage::dispose(LoadContext& context) {
    if (m_glProgramBlur.isCreated()) m_glProgramBlur.dispose();
    if (m_glProgramDoFBlur.isCreated()) m_glProgramDoFBlur.dispose();
}

void HdrRenderStage::render(const Camera* camera /*= nullptr*/) {
    f32m4 oldVP = m_oldVP;
    f32m4 vp = camera->getProjectionMatrix() * camera->getViewMatrix();
    m_oldVP = vp;

    vg::GLProgram* program;
    
    if (soaOptions.get(OPT_MOTION_BLUR).value.i > 0) {
        if (!m_glProgramBlur.isCreated()) {
            m_glProgramBlur = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                                       "Shaders/PostProcessing/MotionBlur.frag",
                                                                       nullptr, "#define MOTION_BLUR\n");
        }
        program = &m_glProgramBlur;
    } else {
        if (!m_program.isCreated()) {
            m_program = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                                 "Shaders/PostProcessing/MotionBlur.frag");
        }
        program = &m_program;
    }

    // TODO(Ben): DOF shader?
    //program = /*graphicsOptions.depthOfField == 1 ? _glProgramDoFBlur :*/ _glProgram;

    program->use();
    program->enableVertexAttribArrays();

    glUniform1i(program->getUniform("unTex"), 0);
    glUniform1f(program->getUniform("unGamma"), 1.0f / soaOptions.get(OPT_GAMMA).value.f);
    glUniform1f(program->getUniform("unExposure"), soaOptions.get(OPT_HDR_EXPOSURE).value.f);
    if (soaOptions.get(OPT_MOTION_BLUR).value.i > 0) {
        f32m4 newInverseVP = glm::inverse(vp);
        glUniform1i(program->getUniform("unTexDepth"), 1);
        glUniformMatrix4fv(program->getUniform("unVPPrev"), 1, GL_FALSE, &oldVP[0][0]);
        glUniformMatrix4fv(program->getUniform("unVPInv"), 1, GL_FALSE, &newInverseVP[0][0]);
        glUniform1i(program->getUniform("unNumSamples"), soaOptions.get(OPT_MOTION_BLUR).value.i);
        glUniform1f(program->getUniform("unBlurIntensity"), 0.5f);
    }
    //if (graphicsOptions.depthOfField > 0) {
    //    glUniform1f(_glprogram->getUniform("unFocalLen"), 70.0f);
    //    glUniform1f(_glprogram->getUniform("unZfocus"), 0.96f); // [0, 1]
    //}

    glDisable(GL_DEPTH_TEST);
    m_quad->draw();
    glEnable(GL_DEPTH_TEST);

    program->disableVertexAttribArrays();
    program->unuse();
}
