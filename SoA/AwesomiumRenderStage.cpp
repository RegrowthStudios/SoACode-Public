#include "stdafx.h"
#include "AwesomiumRenderStage.h"
#include "IAwesomiumInterface.h"


AwesomiumRenderStage::AwesomiumRenderStage(IAwesomiumInterface* awesomiumInterface, vg::GLProgram* glProgram) :
    _awesomiumInterface(awesomiumInterface),
    _glProgram(glProgram) {
}

void AwesomiumRenderStage::draw() {
    _awesomiumInterface->draw(_glProgram);
}