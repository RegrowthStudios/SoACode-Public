#include "stdafx.h"
#include "NightVisionRenderStage.h"
#include "GLProgram.h"
#include "Options.h"


NightVisionRenderStage::NightVisionRenderStage(vg::GLProgram* glProgram, vg::FullQuadVBO* quad) :
    _glProgram(glProgram),
    _quad(quad) {
    // Empty
}

void NightVisionRenderStage::draw() {
    _glProgram->use();
    _glProgram->enableVertexAttribArrays();
    glUniform1i(_glProgram->getUniform("renderedTexture"), 0);

    glDisable(GL_DEPTH_TEST);
    _quad->draw();
    glEnable(GL_DEPTH_TEST);

    _glProgram->disableVertexAttribArrays();
    _glProgram->unuse();
}