#include "stdafx.h"
#include "HdrRenderStage.h"
#include "GLProgram.h"
#include "Options.h"


HdrRenderStage::HdrRenderStage(vg::GLProgram* glProgram, vg::FullQuadVBO* quad) :
    _glProgram(glProgram),
    _quad(quad) {
    // Empty
}

void HdrRenderStage::draw() {
    /// Used for screen space motion blur
   // static bool start = 1;

    /*if (graphicsOptions.motionBlur > 0) {
        program = GameManager::glProgramManager->getProgram("MotionBlur");
        program->use();
        program->enableVertexAttribArrays();

        static f32m4 currMat;
        static f32m4 oldVP;
        static f32m4 newInverseVP;

        if (start) {
            oldVP = VP;
            start = 0;
        } else {
            oldVP = currMat;
        }
        currMat = VP;
        newInverseVP = glm::inverse(currMat);

        glUniform1i(program->getUniform("renderedTexture"), 0);
        glUniform1i(program->getUniform("depthTexture"), 1);
        glUniform1f(program->getUniform("gamma"), 1.0f / graphicsOptions.gamma);
        glUniform1f(program->getUniform("fExposure"), graphicsOptions.hdrExposure);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, frameBuffer->depthTextureIDs[FB_DRAW]);

        glUniformMatrix4fv(program->getUniform("prevVP"), 1, GL_FALSE, &oldVP[0][0]);
        glUniformMatrix4fv(program->getUniform("inverseVP"), 1, GL_FALSE, &newInverseVP[0][0]);
        glUniform1i(program->getUniform("numSamples"), (int)graphicsOptions.motionBlur);
    } else {
        start = 0;*/

    _glProgram->use();
    _glProgram->enableVertexAttribArrays();
    glUniform1i(_glProgram->getUniform("renderedTexture"), 0);
    glUniform1f(_glProgram->getUniform("gamma"), 1.0f / graphicsOptions.gamma);
    glUniform1f(_glProgram->getUniform("fExposure"), graphicsOptions.hdrExposure);
  //  }

    glDisable(GL_DEPTH_TEST);
    _quad->draw();
    glEnable(GL_DEPTH_TEST);

    _glProgram->disableVertexAttribArrays();
    _glProgram->unuse();
}