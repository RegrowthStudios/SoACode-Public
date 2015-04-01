#include "stdafx.h"
#include "AwesomiumRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/ShaderManager.h>

#include "IAwesomiumInterface.h"

AwesomiumRenderStage::AwesomiumRenderStage(IAwesomiumInterface* awesomiumInterface) :
m_awesomiumInterface(awesomiumInterface) {}

void AwesomiumRenderStage::render() {
    glDisable(GL_DEPTH_TEST);

    if (!m_program) {
        m_program = vg::ShaderManager::createProgramFromFile("Shaders/TextureShading/Texture2dShader.vert",
                                                             "Shaders/TextureShading/Texture2dShader.frag");
    }
    m_program->use();
    m_program->enableVertexAttribArrays();

    m_awesomiumInterface->draw(m_program);
    glEnable(GL_DEPTH_TEST);
}
