#include "stdafx.h"
#include "ColoredFullQuadRenderer.h"

#include <Vorb/graphics/GLProgram.h>

const cString VERT_SRC = R"(
in vec4 vPosition;
void main() {
    gl_Position = vPosition;
}
)";

const cString FRAG_SRC = R"(
uniform vec3 unColor;
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

void ColoredFullQuadRenderer::draw(vg::FullQuadVBO& quad, const f32v3& color) {
    // Lazy shader init
    if (!m_program) {
        m_program = new vg::GLProgram(true);

        m_program->addShader(vg::ShaderType::VERTEX_SHADER, VERT_SRC);
        m_program->addShader(vg::ShaderType::FRAGMENT_SHADER, FRAG_SRC);

        m_program->link();
        m_program->initUniforms();
        m_program->initAttributes();
    }
    // Draw the quad
    m_program->use();
    m_program->enableVertexAttribArrays();
    quad.draw();
    m_program->disableVertexAttribArrays();
    m_program->unuse();
}
