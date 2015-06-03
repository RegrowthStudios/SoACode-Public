#include "stdafx.h"
#include "ColorFilterRenderStage.h"

#include "ShaderLoader.h"

namespace {
    const cString VERT_SRC = R"(
in vec4 vPosition;
void main() {
  gl_Position = vPosition;
}
)";

    const cString FRAG_SRC = R"(
uniform vec4 unColor;
out vec4 pColor;
void main() {
  pColor = unColor;
}
)";
}

ColorFilterRenderStage::ColorFilterRenderStage(vg::FullQuadVBO* quad) :
    m_quad(quad) {
    // Empty
}

ColorFilterRenderStage::~ColorFilterRenderStage() {
    dispose();
}

void ColorFilterRenderStage::render() {

    if (!m_program.isCreated()) {
        m_program = ShaderLoader::createProgram("ColorFilterShader", VERT_SRC, FRAG_SRC);
    }

    m_program.use();

    glUniform4fv(m_program.getUniform("unColor"), 1, &m_color[0]);

    glDisable(GL_DEPTH_TEST);
    m_quad->draw();
    glEnable(GL_DEPTH_TEST);

    m_program.unuse();
}
