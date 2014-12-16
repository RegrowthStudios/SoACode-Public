#include "stdafx.h"
#include "TestDeferredScreen.h"

#include <colors.h>
#include <DepthState.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GLRenderTarget.h>
#include <RasterizerState.h>

#pragma region Simple shader code
const cString DEFERRED_SRC_VERT = R"(
uniform mat4 unWVP;

in vec4 vPosition;

void main() {
    gl_Position = unWVP * vPosition;
}
)";
const cString DEFERRED_SRC_FRAG = R"(
out vec4 pColor[3];

void main() {
    pColor[0] = vec4(1, 0, 0, 1);
    pColor[1] = vec4(0, 1, 0, 1);
    pColor[2] = vec4(0, 0, 1, 1);
}
)";
#pragma endregion

// Number cells per row/column in a single grid
const ui32 CELLS = 10;

TestDeferredScreen::TestDeferredScreen() :
m_sb(true, false) {
    // Empty
}

i32 TestDeferredScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestDeferredScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestDeferredScreen::build() {
    // Empty
}
void TestDeferredScreen::destroy(const GameTime& gameTime) {
    // Empty
}

void TestDeferredScreen::onEntry(const GameTime& gameTime) {
    buildGeometry();

    m_gbuffer = vg::GBuffer(_game->getWindow().getWidth(), _game->getWindow().getHeight());
    m_gbuffer.init();
    m_gbuffer.initDepth();

    m_sb.init();

    m_program.init();
    m_program.addShader(vg::ShaderType::VERTEX_SHADER, DEFERRED_SRC_VERT);
    m_program.addShader(vg::ShaderType::FRAGMENT_SHADER, DEFERRED_SRC_FRAG);
    m_program.link();
    m_program.initAttributes();
    m_program.initUniforms();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
}
void TestDeferredScreen::onExit(const GameTime& gameTime) {
    glDeleteBuffers(1, &m_verts);
    glDeleteBuffers(1, &m_inds);
    m_program.dispose();
    m_sb.dispose();
    m_gbuffer.dispose();
}

void TestDeferredScreen::onEvent(const SDL_Event& e) {
    // Empty
}
void TestDeferredScreen::update(const GameTime& gameTime) {
    // Empty
}
void TestDeferredScreen::draw(const GameTime& gameTime) {
    /************************************************************************/
    /* Deferred pass                                                        */
    /************************************************************************/
    m_gbuffer.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f32m4 mWVP = glm::perspectiveFov(90.0f, 800.0f, 600.0f, 0.1f, 100.0f) * glm::lookAt(f32v3(0, 0, 10), f32v3(0, 0, 0), f32v3(0, 1, 0));

    DepthState::FULL.set();
    RasterizerState::CULL_CLOCKWISE.set();

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
    vg::GLRenderTarget::unuse(_game->getWindow().getWidth(), _game->getWindow().getHeight());

    /************************************************************************/
    /* Debug                                                                */
    /************************************************************************/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_sb.begin();
    m_sb.draw(m_gbuffer.getTextureIDs().color, f32v2(0, 0), f32v2(100, 80), color::White);
    m_sb.draw(m_gbuffer.getTextureIDs().normal, f32v2(100, 0), f32v2(100, 80), color::White);
    m_sb.draw(m_gbuffer.getTextureIDs().depth, f32v2(200, 0), f32v2(100, 80), color::White);
    m_sb.end();
    m_sb.renderBatch(f32v2(_game->getWindow().getWidth(), _game->getWindow().getHeight()));

}

void TestDeferredScreen::buildGeometry() {
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
