#include "stdafx.h"
#include "TestBlockViewScreen.h"

#include <glm/gtx/transform.hpp>
#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/Random.h>
#include <Vorb/Timing.h>
#include <Vorb/voxel/VoxelMesherCulled.h>

#include "BlockLoader.h"

#pragma region Simple shader code
const cString SRC_VERT_BLOCK = R"(
uniform mat4 unWVP;

in vec4 vPosition;
in vec3 vColor;
in vec3 vUV;

out vec3 fUV;
out vec3 fColor;

void main() {
    fUV = vUV;
    fColor = vColor;
    gl_Position = unWVP * vPosition;
}
)";
const cString SRC_FRAG_BLOCK = R"(
in vec3 fUV;
in vec3 fColor;

out vec4 pColor;

void main() {
    vec3 ff = fract(fUV);
    float f1 = min(ff.x, ff.y);
    float f2 = max(ff.x, ff.y);
    float f = 1.0 - max(1.0 - f1, f2);
    pColor = vec4(fColor * f, 1);
}
)";
#pragma endregion

struct VertexPosColor {
public:
    f32v3 pos;
    f32v2 uv;
    color3 color;
};

f32v3 BLOCK_MODEL[24] = {
    f32v3(0, 1, 0),
    f32v3(0, 1, 1),
    f32v3(0, 0, 0),
    f32v3(0, 0, 1),

    f32v3(1, 1, 1),
    f32v3(1, 1, 0),
    f32v3(1, 0, 1),
    f32v3(1, 0, 0),

    f32v3(0, 0, 1),
    f32v3(1, 0, 1),
    f32v3(0, 0, 0),
    f32v3(1, 0, 0),

    f32v3(0, 1, 0),
    f32v3(1, 1, 0),
    f32v3(0, 1, 1),
    f32v3(1, 1, 1),

    f32v3(1, 1, 0),
    f32v3(0, 1, 0),
    f32v3(1, 0, 0),
    f32v3(0, 0, 0),

    f32v3(0, 1, 1),
    f32v3(1, 1, 1),
    f32v3(0, 0, 1),
    f32v3(1, 0, 1)
};
f32v2 BLOCK_UVS[4] = {
    f32v2(0, 1),
    f32v2(1, 1),
    f32v2(0, 0),
    f32v2(1, 0)
};

const ui32 TEST_CHUNK_SIZE = 34;

class APICullSimple {
public:
    APICullSimple(const ui16* data) :
        m_data(data) {
        // Empty
    }

    vvox::meshalg::VoxelFaces occludes(const ui16& v1, const ui16& v2, const vvox::Axis& axis) const {
        vvox::meshalg::VoxelFaces f = {};
        if (v1 != 0 && v2 == 0) f.block1Face = true;
        else if (v2 != 0 && v1 == 0) f.block2Face = true;
        return f;
    }
    void result(const vvox::meshalg::VoxelQuad& quad) {
        VertexPosColor v;
        switch (m_data[quad.startIndex]) {
        case 1:
            v.color = color::Red.rgb;
            break;
        default:
            v.color = color::Blue.rgb;
            break;
        }

        f32v3 off(quad.voxelPosition);
        f32v3 scale;

        switch (quad.direction) {
        case vvox::Cardinal::X_NEG:
        case vvox::Cardinal::X_POS:
            scale.x = 1;
            scale.z = (f32)quad.size.x;
            scale.y = (f32)quad.size.y;
            break;
        case vvox::Cardinal::Y_NEG:
        case vvox::Cardinal::Y_POS:
            scale.y = 1;
            scale.x = (f32)quad.size.x;
            scale.z = (f32)quad.size.y;
            break;
        case vvox::Cardinal::Z_NEG:
        case vvox::Cardinal::Z_POS:
            scale.z = 1;
            scale.x = (f32)quad.size.x;
            scale.y = (f32)quad.size.y;
            break;
        default:
            break;
        }

        for (size_t i = 0; i < 4; i++) {
            v.uv = BLOCK_UVS[i];
            v.pos = off + BLOCK_MODEL[(size_t)quad.direction * 4 + i] * scale;
            vertices.emplace_back(v);
        }
    }

    std::vector<VertexPosColor> vertices;
private:
    const ui16* m_data;
};

i32 TestBlockView::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestBlockView::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestBlockView::build() {
    // Empty
}
void TestBlockView::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void TestBlockView::onEntry(const vui::GameTime& gameTime) {
    m_hooks.addAutoHook(m_blocks.onBlockAddition, [&] (Sender s, ui16 id) {
        printf("Loaded Block: %s = %d\n", m_blocks[id].name.c_str(), id);
    });
    m_hooks.addAutoHook(vui::InputDispatcher::window.onFile, [&] (Sender s, const vui::WindowFileEvent& e) {
        loadBlocks(e.file);
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [&] (Sender s, const vui::MouseMotionEvent& e) {
        if (m_movingCamera) {
            m_mRotation = glm::rotate(f32m4(), 1.2f * e.dx, f32v3(0.0f, 1.0f, 0.0f)) * m_mRotation;
            m_mRotation = glm::rotate(f32m4(), 1.2f * e.dy, f32v3(1.0f, 0.0f, 0.0f)) * m_mRotation;
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&] (Sender s, const vui::MouseButtonEvent& e) {
        if (e.button == vui::MouseButton::MIDDLE) m_movingCamera = true;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&] (Sender s, const vui::MouseButtonEvent& e) {
        if (e.button == vui::MouseButton::MIDDLE) m_movingCamera = false;
    });

    m_program.init();
    m_program.addShader(vg::ShaderType::VERTEX_SHADER, SRC_VERT_BLOCK);
    m_program.addShader(vg::ShaderType::FRAGMENT_SHADER, SRC_FRAG_BLOCK);
    m_program.link();
    m_program.initAttributes();
    m_program.initUniforms();
    genBlockMesh();
    m_mRotation = f32m4(1.0);
    m_movingCamera = false;

    // Set clear state
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);

}
void TestBlockView::onExit(const vui::GameTime& gameTime) {
    // Empty
}

void TestBlockView::update(const vui::GameTime& gameTime) {
    // Empty
}
void TestBlockView::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program.use();
    f32 tCenter = (f32)TEST_CHUNK_SIZE * -0.5f;
    f32m4 mWVP = glm::perspectiveFov(90.0f, 800.0f, 600.0f, 0.1f, 1000.0f) * glm::lookAt(f32v3(0, 0, TEST_CHUNK_SIZE), f32v3(0, 0, 0), f32v3(0, 1, 0)) * m_mRotation * glm::translate(tCenter, tCenter, tCenter);

    vg::DepthState::FULL.set();
    vg::RasterizerState::CULL_CLOCKWISE.set();

    m_program.use();
    m_program.enableVertexAttribArrays();

    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, false, (f32*)&mWVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glVertexAttribPointer(m_program.getAttribute("vPosition"), 3, GL_FLOAT, false, sizeof(VertexPosColor), offsetptr(VertexPosColor, pos));
    glVertexAttribPointer(m_program.getAttribute("vUV"), 2, GL_FLOAT, false, sizeof(VertexPosColor), offsetptr(VertexPosColor, uv));
    glVertexAttribPointer(m_program.getAttribute("vColor"), 3, GL_UNSIGNED_BYTE, true, sizeof(VertexPosColor), offsetptr(VertexPosColor, color));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glDrawElements(GL_TRIANGLES, m_indCount, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_program.disableVertexAttribArrays();
    vg::GLProgram::unuse();
}

void TestBlockView::loadBlocks(const cString file) {
    vio::IOManager iom;
    BlockLoader::load(iom, file, &m_blocks);
}

void TestBlockView::genBlockMesh() {
    size_t vc = TEST_CHUNK_SIZE * TEST_CHUNK_SIZE * TEST_CHUNK_SIZE;
    ui16* data = new ui16[vc]();
    Random r;
    r.seed(10);
    for (ui32 z = 0; z < TEST_CHUNK_SIZE; z++) {
        for (ui32 x = 0; x < TEST_CHUNK_SIZE; x++) {
            ui32 h = (ui32)(r.genMH() * 4) + 15;
            for (ui32 y = 0; y < h; y++) {
                size_t vi = (y * TEST_CHUNK_SIZE + z) * TEST_CHUNK_SIZE + x;
                data[vi] = 1;
            }
        }
    }

    APICullSimple api(data);
    api.vertices.reserve(10000);
    vvox::meshalg::createCulled<ui16>(data, ui32v3(TEST_CHUNK_SIZE), &api);
    delete[] data;

    glGenBuffers(1, &m_verts);
    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glBufferData(GL_ARRAY_BUFFER, api.vertices.size() * sizeof(VertexPosColor), api.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_indCount = (api.vertices.size()) / 4 * 6;
    printf("Tris: %d\n", m_indCount / 3);

    ui32* inds = vvox::meshalg::generateQuadIndices<ui32>(api.vertices.size() / 4);
    glGenBuffers(1, &m_inds);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indCount * sizeof(ui32), inds, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    delete[] inds;
}
