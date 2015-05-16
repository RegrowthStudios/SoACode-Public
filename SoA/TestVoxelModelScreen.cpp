#include "stdafx.h"
#include "TestVoxelModelScreen.h"

#include <glm/gtx/transform.hpp>
#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/io/IOManager.h>

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

class VoxelModelVertex {
public:
    VoxelModelVertex(f32v3 pos, color3 color):
        pos(pos),
        color(color)
    {}
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

ui32 VOXEL_INDICES[6] = {
    0, 2, 1,
    1, 2, 3
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
    f32m4 mWVP = glm::perspectiveFov(90.0f, 800.0f, 600.0f, 0.1f, 1000.0f) * glm::lookAt(f32v3(0, 0, 5), f32v3(0, 0, 0), f32v3(0, 1, 0)) * m_mRotation * glm::translate(tCenter, tCenter, tCenter);

    vg::DepthState::FULL.set();
    vg::RasterizerState::CULL_CLOCKWISE.set();

    m_program.use();
    m_program.enableVertexAttribArrays();

    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, false, (f32*)&mWVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glVertexAttribPointer(m_program.getAttribute("vPosition"), 3, GL_FLOAT, false, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, pos));
    glVertexAttribPointer(m_program.getAttribute("vColor"), 3, GL_UNSIGNED_BYTE, true, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, color));
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

    std::vector<VoxelModelVertex> vertices;
    std::vector<ui32> indices;
    genMatrixMesh(matrices[0], vertices, indices);

    glGenBuffers(1, &m_verts);
    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelModelVertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    m_indCount = indices.size();
    glGenBuffers(1, &m_inds);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indCount * sizeof(ui32), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    for(int i = matrices.size()-1; i >= 0; i--) {
        VoxelMatrix* matrix = matrices.back();
        matrices.pop_back();
        delete matrix;
    }
}

void TestVoxelModelScreen::genMatrixMesh(const VoxelMatrix* matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices) {
    for(i32 i = 0; i < matrix->size.x; i++) {
        for(i32 j = 0; j < matrix->size.y; j++) {
            for(i32 k = 0; k < matrix->size.z; k++) {
                ColorRGBA8 voxel = matrix->getColor(i,j,k);
                if(voxel.a == 0) continue;
                f32v3 offset(i, j, k);
                ColorRGBA8 temp = matrix->getColor(i - 1, j, k);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i + 1, j, k);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[4 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i, j-1, k);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[8 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i, j+1, k);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[12 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i, j, k-1);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[16 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i, j, k+1);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[20 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
            }
        }
    }

}

