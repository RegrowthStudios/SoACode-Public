#include "stdafx.h"
#include "TestVoxelModelScreen.h"

#include <glm/gtx/transform.hpp>
#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/Random.h>
#include <Vorb/Timing.h>
#include <Vorb/voxel/VoxelMesherCulled.h>

#include "BlockLoader.h"

#include "VoxelModelLoader.h"

#pragma region Simple voxel mesh shader code
const cString SRC_VERT_VOXEL = R"(
uniform mat4 unWVP;

in vec4 vPosition;
in vec3 vColor;

out vec3 fColor;

void main() {
    fColor = vColor;
    gl_Position = unWVP * vPosition;
}
)";
const cString SRC_FRAG_VOXEL = R"(
in vec3 fColor;

out vec4 pColor;

void main() {
    pColor = vec4(fColor, 1);
}
)";
#pragma endregion

struct VertexPosColor {
public:
    f32v3 pos;
    color3 color;
};

f32v3 VOXEL_MODEL[24] = {
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

class APIVoxelCullSimple {
public:
    APIVoxelCullSimple(const ColorRGBA8* data):
        m_data(data) {
        // Empty
    }

    vvox::meshalg::VoxelFaces occludes(const ColorRGBA8& v1, const ColorRGBA8& v2, const vvox::Axis& axis) const {
        vvox::meshalg::VoxelFaces f = {};
        if(v1.a != 0 && v2.a == 0) f.block1Face = true;
        else if(v1.a == 0 && v2.a != 0) f.block2Face = true;
        return f;
    }
    void result(const vvox::meshalg::VoxelQuad& quad) {
        VertexPosColor v;
        switch(m_data[quad.startIndex].a) {
        case 0:
            v.color = color::Blue.rgb;
            break;
        default:
            v.color = m_data[quad.startIndex].rgb;
            break;
        }

        f32v3 off(quad.voxelPosition);
        f32v3 scale;

        switch(quad.direction) {
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

        for(size_t i = 0; i < 4; i++) {
            v.pos = off + VOXEL_MODEL[(size_t)quad.direction * 4 + i] * scale;
            vertices.emplace_back(v);
        }
    }

    std::vector<VertexPosColor> vertices;
private:
    const ColorRGBA8* m_data;
};

i32 TestVoxelModelScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestVoxelModelScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestVoxelModelScreen::build() {
    // Empty
}
void TestVoxelModelScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void TestVoxelModelScreen::onEntry(const vui::GameTime& gameTime) {
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [&](Sender s, const vui::MouseMotionEvent& e) {
        if(m_movingCamera) {
            m_mRotation = glm::rotate(f32m4(), 1.2f * e.dx, f32v3(0.0f, 1.0f, 0.0f)) * m_mRotation;
            m_mRotation = glm::rotate(f32m4(), 1.2f * e.dy, f32v3(1.0f, 0.0f, 0.0f)) * m_mRotation;
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s, const vui::MouseButtonEvent& e) {
        if(e.button == vui::MouseButton::MIDDLE) m_movingCamera = true;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s, const vui::MouseButtonEvent& e) {
        if(e.button == vui::MouseButton::MIDDLE) m_movingCamera = false;
    });

    m_program.init();
    m_program.addShader(vg::ShaderType::VERTEX_SHADER, SRC_VERT_VOXEL);
    m_program.addShader(vg::ShaderType::FRAGMENT_SHADER, SRC_FRAG_VOXEL);
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
void TestVoxelModelScreen::onExit(const vui::GameTime& gameTime) {
    // Empty
}

void TestVoxelModelScreen::update(const vui::GameTime& gameTime) {
    // Empty
}
void TestVoxelModelScreen::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program.use();
    f32 tCenter = (f32)20 * -0.5f;
    f32m4 mWVP = glm::perspectiveFov(90.0f, 800.0f, 600.0f, 0.1f, 1000.0f) * glm::lookAt(f32v3(0, 0, -1), f32v3(0, 0, 0), f32v3(0, 1, 0)) * m_mRotation * glm::translate(tCenter, tCenter, tCenter);

    vg::DepthState::FULL.set();
    vg::RasterizerState::CULL_CLOCKWISE.set();

    m_program.use();
    m_program.enableVertexAttribArrays();

    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, false, (f32*)&mWVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glVertexAttribPointer(m_program.getAttribute("vPosition"), 3, GL_FLOAT, false, sizeof(VertexPosColor), offsetptr(VertexPosColor, pos));
    glVertexAttribPointer(m_program.getAttribute("vColor"), 3, GL_UNSIGNED_BYTE, true, sizeof(VertexPosColor), offsetptr(VertexPosColor, color));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glDrawElements(GL_TRIANGLES, m_indCount, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_program.disableVertexAttribArrays();
    vg::GLProgram::unuse();
}

void TestVoxelModelScreen::genBlockMesh() {
    printf("Loading Models!\n");
    std::vector<VoxelMatrix*> matrices = VoxelModelLoader::loadModel("Models\\deer.qb");
    printf("Loaded %d matrices\n", matrices.size());
    printf("Matrix Size: (%d, %d, %d)\n", matrices[0]->size.x, matrices[0]->size.y, matrices[0]->size.z);

    APIVoxelCullSimple* api = new APIVoxelCullSimple(matrices[0]->data);
    //api.vertices.reserve(10000);
    vvox::meshalg::createCulled<ColorRGBA8>(matrices[0]->data, matrices[0]->size, api);

    glGenBuffers(1, &m_verts);
    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glBufferData(GL_ARRAY_BUFFER, api->vertices.size() * sizeof(VertexPosColor), api->vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_indCount = (api->vertices.size()) / 4 * 6;
    printf("Tris: %d\n", m_indCount / 3);

    ui32* inds = vvox::meshalg::generateQuadIndices<ui32>(api->vertices.size() / 4);
    glGenBuffers(1, &m_inds);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indCount * sizeof(ui32), inds, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    delete[] inds;
    for(int i = matrices.size()-1; i >= 0; i--) {
        VoxelMatrix* matrix = matrices.back();
        matrices.pop_back();
        delete matrix;
    }
    delete api;
}

