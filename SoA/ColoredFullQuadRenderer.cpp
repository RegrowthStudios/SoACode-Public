#include "stdafx.h"
#include "ColoredFullQuadRenderer.h"

#include <Vorb/graphics/GLProgram.h>

const cString COL_VERT_SRC = R"(
in vec4 vPosition;
void main() {
    gl_Position = vPosition;
}
)";

const cString COL_FRAG_SRC = R"(
uniform vec4 unColor;
out vec4 fColor;
void main() {
    fColor = unColor;
}
)";

ColoredFullQuadRenderer::~ColoredFullQuadRenderer() {
    if (m_program) {
        m_program->dispose();
        delete m_program;
    }
}

void ColoredFullQuadRenderer::draw(vg::FullQuadVBO& quad, const f32v4& color) {
    // Lazy shader init
    if (!m_program) {
        m_program = new vg::GLProgram(true);

        m_program->addShader(vg::ShaderType::VERTEX_SHADER, COL_VERT_SRC);
        m_program->addShader(vg::ShaderType::FRAGMENT_SHADER, COL_FRAG_SRC);

        m_program->link();
        m_program->initUniforms();
        m_program->initAttributes();
    }
    // Draw the quad
    m_program->use();
    m_program->enableVertexAttribArrays();
    glUniform4fv(m_program->getUniform("unColor"), 1, &color[0]);
    quad.draw();
    m_program->disableVertexAttribArrays();
    m_program->unuse();
}
