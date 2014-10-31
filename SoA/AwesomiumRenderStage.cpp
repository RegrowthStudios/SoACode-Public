#include "stdafx.h"
#include "AwesomiumRenderStage.h"
#include "IAwesomiumInterface.h"


AwesomiumRenderStage::AwesomiumRenderStage(IAwesomiumInterface* awesomiumInterface, vg::GLProgram* glProgram) :
    _awesomiumInterface(awesomiumInterface),
    _glProgram(glProgram) {
}


AwesomiumRenderStage::~AwesomiumRenderStage() {
}

void AwesomiumRenderStage::setState(vg::FrameBuffer* frameBuffer /*= nullptr*/) {
    throw std::logic_error("The method or operation is not implemented.");
}

void AwesomiumRenderStage::draw() {
    glDisable(GL_DEPTH_TEST);
    _awesomiumInterface->draw(_glProgram);
    glEnable(GL_DEPTH_TEST);
}

bool AwesomiumRenderStage::isVisible() {
    throw std::logic_error("The method or operation is not implemented.");
}
