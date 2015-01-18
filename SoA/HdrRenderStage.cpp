#include "stdafx.h"
#include "HdrRenderStage.h"

#include <Vorb/graphics/GLProgram.h>

#include "Camera.h"
#include "Options.h"

HdrRenderStage::HdrRenderStage(const vg::GLProgramManager* glPM, vg::FullQuadVBO* quad, const Camera* camera) :
    _glProgramDefault(glPM->getProgram("HDR")),
    _glProgramBlur(glPM->getProgram("MotionBlur")),
    _glProgramDoFBlur(glPM->getProgram("DoFMotionBlur")),
    _quad(quad),
    _oldVP(1.0f),
    _camera(camera) {
    // Empty
}

void HdrRenderStage::draw() {
    f32m4 oldVP = _oldVP;
    f32m4 vp = _camera->getProjectionMatrix() * _camera->getViewMatrix();
    _oldVP = vp;

    vg::GLProgram* _glProgram = graphicsOptions.motionBlur > 0 ? _glProgramBlur : _glProgramDefault;
    _glProgram = /*graphicsOptions.depthOfField == 1 ? _glProgramDoFBlur :*/ _glProgram;

    _glProgram->use();
    _glProgram->enableVertexAttribArrays();

    glUniform1i(_glProgram->getUniform("unTex"), 0);
    glUniform1f(_glProgram->getUniform("unGamma"), 1.0f / graphicsOptions.gamma);
    glUniform1f(_glProgram->getUniform("unExposure"), graphicsOptions.hdrExposure);
    if (graphicsOptions.motionBlur > 0) {
        f32m4 newInverseVP = glm::inverse(vp);
        glUniform1i(_glProgram->getUniform("unTexDepth"), 1);
        glUniformMatrix4fv(_glProgram->getUniform("unVPPrev"), 1, GL_FALSE, &oldVP[0][0]);
        glUniformMatrix4fv(_glProgram->getUniform("unVPInv"), 1, GL_FALSE, &newInverseVP[0][0]);
        glUniform1i(_glProgram->getUniform("unNumSamples"), (int)graphicsOptions.motionBlur);
        glUniform1f(_glProgram->getUniform("unBlurIntensity"), 0.5f);
    }
    //if (graphicsOptions.depthOfField > 0) {
    //    glUniform1f(_glProgram->getUniform("unFocalLen"), 70.0f);
    //    glUniform1f(_glProgram->getUniform("unZfocus"), 0.96f); // [0, 1]
    //}

    glDisable(GL_DEPTH_TEST);
    _quad->draw();
    glEnable(GL_DEPTH_TEST);

    _glProgram->disableVertexAttribArrays();
    _glProgram->unuse();
}