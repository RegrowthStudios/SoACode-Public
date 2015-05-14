#include "stdafx.h"
#include "HdrRenderStage.h"

#include <Vorb/graphics/GLProgram.h>

#include <Vorb/graphics/GLProgram.h>
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

HdrRenderStage::HdrRenderStage(vg::FullQuadVBO* quad, const Camera* camera) : IRenderStage("HDR", camera),
    m_quad(quad),
    m_oldVP(1.0f) {
    // Empty
}

void HdrRenderStage::reloadShader() {
    IRenderStage::reloadShader();
    if (m_glProgramBlur) {
        vg::ShaderManager::destroyProgram(&m_glProgramBlur);
    }
    if (m_glProgramDoFBlur) {
        vg::ShaderManager::destroyProgram(&m_glProgramDoFBlur);
    }
}

void HdrRenderStage::dispose() {
    IRenderStage::dispose();
    if (m_glProgramBlur) {
        vg::ShaderManager::destroyProgram(&m_glProgramBlur);
    }
}

void HdrRenderStage::render() {
    f32m4 oldVP = m_oldVP;
    f32m4 vp = m_camera->getProjectionMatrix() * m_camera->getViewMatrix();
    m_oldVP = vp;

    vg::GLProgram* program;
    
    if (graphicsOptions.motionBlur > 0) {
        if (!m_glProgramBlur) {
            m_glProgramBlur = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                                       "Shaders/PostProcessing/MotionBlur.frag",
                                                                       nullptr, "#define MOTION_BLUR\n");
        }
        program = m_glProgramBlur;
    } else {
        if (!m_program) {
            m_program = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                                 "Shaders/PostProcessing/MotionBlur.frag");
        }
        program = m_program;
    }

    // TODO(Ben): DOF shader?
    //program = /*graphicsOptions.depthOfField == 1 ? _glProgramDoFBlur :*/ _glProgram;

    program->use();
    program->enableVertexAttribArrays();

    glUniform1i(program->getUniform("unTex"), 0);
    glUniform1f(program->getUniform("unGamma"), 1.0f / graphicsOptions.gamma);
    glUniform1f(program->getUniform("unExposure"), graphicsOptions.hdrExposure);
    if (graphicsOptions.motionBlur > 0) {
        f32m4 newInverseVP = glm::inverse(vp);
        glUniform1i(program->getUniform("unTexDepth"), 1);
        glUniformMatrix4fv(program->getUniform("unVPPrev"), 1, GL_FALSE, &oldVP[0][0]);
        glUniformMatrix4fv(program->getUniform("unVPInv"), 1, GL_FALSE, &newInverseVP[0][0]);
        glUniform1i(program->getUniform("unNumSamples"), (int)graphicsOptions.motionBlur);
        glUniform1f(program->getUniform("unBlurIntensity"), 0.5f);
    }
    //if (graphicsOptions.depthOfField > 0) {
    //    glUniform1f(_glProgram->getUniform("unFocalLen"), 70.0f);
    //    glUniform1f(_glProgram->getUniform("unZfocus"), 0.96f); // [0, 1]
    //}

    glDisable(GL_DEPTH_TEST);
    m_quad->draw();
    glEnable(GL_DEPTH_TEST);

    program->disableVertexAttribArrays();
    program->unuse();
}
