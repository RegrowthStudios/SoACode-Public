#include "stdafx.h"
#include "TestMappingScreen.h"

#include <Vorb/graphics/GLStates.h>
#include <glm/gtc/matrix_transform.hpp>

#ifdef _MSC_VER
#pragma region Simple shader code
#endif// _MSC_VER
const cString SRC_VERT = R"(
uniform mat4 unWVP;

in vec4 vPosition;

void main() {
    gl_Position = unWVP * vPosition;
}
)";
const cString SRC_FRAG = R"(
out vec4 pColor;

void main() {
    pColor = vec4(1, 0, 0, 1);
}
)";
#ifdef _MSC_VER
#pragma endregion
#endif// _MSC_VER

// Number cells per row/column in a single grid
const ui32 CELLS = 30;

i32 TestMappingScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestMappingScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestMappingScreen::build() {
    // Empty
}
void TestMappingScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void TestMappingScreen::onEntry(const vui::GameTime& gameTime) {
    buildGeometry();

    m_program.init();
    m_program.addShader(vg::ShaderType::VERTEX_SHADER, SRC_VERT);
    m_program.addShader(vg::ShaderType::FRAGMENT_SHADER, SRC_FRAG);
    m_program.link();
    m_program.initAttributes();
    m_program.initUniforms();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
}
void TestMappingScreen::onExit(const vui::GameTime& gameTime) {
    glDeleteBuffers(1, &m_verts);
    glDeleteBuffers(1, &m_inds);
    m_program.dispose();
}

void TestMappingScreen::update(const vui::GameTime& gameTime) {
    // Empty
}
void TestMappingScreen::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f32m4 mWVP = glm::perspectiveFov(90.0f, 800.0f, 600.0f, 0.1f, 100.0f) * glm::lookAt(f32v3(0, 0, 10), f32v3(0, 0, 0), f32v3(0, 1, 0)) ;

    vg::DepthState::FULL.set();
    vg::RasterizerState::CULL_CLOCKWISE.set();

    m_program.use();
    m_program.enableVertexAttribArrays();

    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, false, (f32*)&mWVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glVertexAttribPointer(m_program.getAttribute("vPosition"), 3, GL_FLOAT, false, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_program.disableVertexAttribArrays();
    vg::GLProgram::unuse();
}

void TestMappingScreen::buildGeometry() {
    // Make vertices
    ui32 cellPoints = CELLS + 1;
    ui32 gridPoints = cellPoints * cellPoints;
    f32v3* verts = new f32v3[gridPoints * 2];
    size_t i = 0;
    for (size_t y = 0; y < cellPoints; y++) {
        for (size_t x = 0; x < cellPoints; x++) {
            f32v3 pos((f32)x / (f32)CELLS, 0, (f32)y / (f32)CELLS);
            pos.x -= 0.5f;
            pos.x *= 2.0f;
            pos.z -= 0.5f;
            pos.z *= 2.0f;
            pos.x = glm::sign(pos.x) * (sin(abs(pos.x) * 1.57079f));
            pos.z = glm::sign(pos.z) * (sin(abs(pos.z) * 1.57079f));

            f32 lp = glm::length(pos);
            if (lp < 0.00001) {
                pos.y = 1.0f;
            } else {
                f32 d = std::max(abs(pos.x), abs(pos.z));
                pos /= lp;
                pos *= d;
                pos.y = sin(acos(d));
            }

            verts[i] = pos;
            pos.y = -pos.y;
            verts[gridPoints + i] = pos;
            i++;
        }
    }

    // Make indices
    ui32 gridInds = 6 * CELLS * CELLS;
    m_indexCount = gridInds * 2;
    ui32* inds = new ui32[m_indexCount];
    i = 0;
    for (size_t y = 0; y < CELLS; y++) {
        for (size_t x = 0; x < CELLS; x++) {
            ui32 vi = y * cellPoints + x;
            inds[i] = vi;
            inds[i + 1] = vi + cellPoints;
            inds[i + 2] = vi + 1;
            inds[i + 3] = vi + 1;
            inds[i + 4] = vi + cellPoints;
            inds[i + 5] = vi + cellPoints + 1;

            vi += gridPoints;
            inds[gridInds + i] = vi;
            inds[gridInds + i + 1] = vi + 1;
            inds[gridInds + i + 2] = vi + cellPoints;
            inds[gridInds + i + 3] = vi + cellPoints;
            inds[gridInds + i + 4] = vi + 1;
            inds[gridInds + i + 5] = vi + cellPoints + 1;

            i += 6;
        }
    }

    // Fill buffers
    glGenBuffers(1, &m_verts);
    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glBufferData(GL_ARRAY_BUFFER, gridPoints * 2 * sizeof(f32v3), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    delete[] verts;
    glGenBuffers(1, &m_inds);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(ui32), inds, GL_STATIC_DRAW);
    delete[] inds;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
