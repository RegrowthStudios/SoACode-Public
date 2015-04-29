#include "stdafx.h"
#include "SkyboxRenderer.h"

#include <Vorb/graphics/GLEnums.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/ShaderManager.h>

#include "ShaderLoader.h"

// Skybox Cube //
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3

#define INDICES_PER_QUAD 6
#define VERTS_PER_QUAD 4
#define SKYBOX_FACES 6

const float skyboxSize = 10.0f;
const float skyboxVertices[72] = {
    -skyboxSize, skyboxSize, skyboxSize, -skyboxSize, -skyboxSize, skyboxSize, skyboxSize, -skyboxSize, skyboxSize, skyboxSize, skyboxSize, skyboxSize,  // v1-v2-v3-v0 (front)
    skyboxSize, skyboxSize, skyboxSize, skyboxSize, -skyboxSize, skyboxSize, skyboxSize, -skyboxSize, -skyboxSize, skyboxSize, skyboxSize, -skyboxSize,     // v0-v3-v4-v5 (right)
    -skyboxSize, skyboxSize, -skyboxSize, -skyboxSize, skyboxSize, skyboxSize, skyboxSize, skyboxSize, skyboxSize, skyboxSize, skyboxSize, -skyboxSize,    // v6-v1-v0-v5 (top)
    -skyboxSize, skyboxSize, -skyboxSize, -skyboxSize, -skyboxSize, -skyboxSize, -skyboxSize, -skyboxSize, skyboxSize, -skyboxSize, skyboxSize, skyboxSize,   // v6-v7-v2-v1 (left)
    -skyboxSize, -skyboxSize, -skyboxSize, skyboxSize, -skyboxSize, -skyboxSize, skyboxSize, -skyboxSize, skyboxSize, -skyboxSize, -skyboxSize, skyboxSize,    // v7-v4-v3-v2 (bottom)
    skyboxSize, skyboxSize, -skyboxSize, skyboxSize, -skyboxSize, -skyboxSize, -skyboxSize, -skyboxSize, -skyboxSize, -skyboxSize, skyboxSize, -skyboxSize };     // v5-v4-v7-v6 (back)

// These are the X,Y components. The Z component is determined by integer division
const float skyboxUVs[48] = { 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,  // v1-v2-v3-v0 (front)
1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // v0-v3-v4-v5 (right)
1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,// v6-v1-v0-v5 (top)
1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,// v6-v7-v2-v1 (left)
1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,  // v7-v4-v3-v2 (bottom)
1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 }; // v5-v4-v7-v6 (back)

namespace {
    const cString VERT_SRC = R"(
uniform mat4 unWVP;
in vec4 vPosition;
in vec3 vUVW;
out vec3 fUVW;
void main() {
  fUVW = vUVW;
  gl_Position = unWVP * vPosition;
}
)";
    const cString FRAG_SRC = R"(
uniform sampler2DArray unTex;
in vec3 fUVW;
out vec4 pColor;
void main() {
  pColor = texture(unTex, vec3(fUVW.xy, 0.0));
  pColor.a = 1.0;
})";
}

SkyboxRenderer::SkyboxRenderer() : 
    m_vao(0),
    m_vbo(0),
    m_ibo(0) {
    // Empty
}


SkyboxRenderer::~SkyboxRenderer() {
    destroy();
}

void SkyboxRenderer::drawSkybox(const f32m4& VP, VGTexture textureArray) {

    if (!m_program) initShader();
    // Bind shader
    m_program->use();

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
    // Upload VP matrix
    glUniformMatrix4fv(m_program->getUniform("unWVP"), 1, GL_FALSE, &VP[0][0]);
    
    // Create the buffer objects if they aren't initialized
    if (m_vbo == 0) {
        initBuffers();
    }

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, INDICES_PER_QUAD * 6, GL_UNSIGNED_SHORT, (void*)0); //offset
    glBindVertexArray(0);

    // Unbind shader
    m_program->unuse();  
}

void SkyboxRenderer::destroy() {
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo) {
        vg::GpuMemory::freeBuffer(m_vbo);
        m_vbo = 0;
    }
    if (m_ibo) {
        vg::GpuMemory::freeBuffer(m_ibo);
        m_ibo = 0;
    }
    if (m_program) {
        vg::ShaderManager::destroyProgram(&m_program);
    }
}

void SkyboxRenderer::initShader() {
    m_program = ShaderLoader::createProgram("Skybox", VERT_SRC, FRAG_SRC);

    // Constant uniforms
    m_program->use();
    glUniform1i(m_program->getUniform("unTex"), 0);
    m_program->unuse();
}

void SkyboxRenderer::initBuffers() {
    // Vertex Array Object
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Vertex Buffer Object
    vg::GpuMemory::createBuffer(m_vbo);
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);

    SkyboxVertex verts[VERTS_PER_QUAD * SKYBOX_FACES];
    for (int i = 0; i < VERTS_PER_QUAD * SKYBOX_FACES; i++) {
        verts[i].position = f32v3(skyboxVertices[i * 3],
                                  skyboxVertices[i * 3 + 1],
                                  skyboxVertices[i * 3 + 2]);
        verts[i].texCoords = f32v3(skyboxUVs[i * 2], skyboxUVs[i * 2 + 1], (f32)(i / VERTS_PER_QUAD));
    }

    vg::GpuMemory::uploadBufferData(m_vbo, vg::BufferTarget::ARRAY_BUFFER, sizeof(verts), verts, vg::BufferUsageHint::STATIC_DRAW);

    vg::GpuMemory::createBuffer(m_ibo);
    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);

    // Index buffer Object
    ui16 skyboxIndices[INDICES_PER_QUAD * SKYBOX_FACES];
    // Set up indices
    i32 ci = 0;
    for (i32 i = 0; i < INDICES_PER_QUAD * SKYBOX_FACES; i += INDICES_PER_QUAD, ci += VERTS_PER_QUAD) {
        skyboxIndices[i] = ci;
        skyboxIndices[i + 1] = ci + 3;
        skyboxIndices[i + 2] = ci + 2;
        skyboxIndices[i + 3] = ci + 2;
        skyboxIndices[i + 4] = ci + 1;
        skyboxIndices[i + 5] = ci;
    }

    vg::GpuMemory::uploadBufferData(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, vg::BufferUsageHint::STATIC_DRAW);

    // Set up attribute pointers
    m_program->enableVertexAttribArrays();
    glVertexAttribPointer(m_program->getAttribute("vPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(SkyboxVertex), offsetptr(SkyboxVertex, position));
    glVertexAttribPointer(m_program->getAttribute("vUVW"), 3, GL_FLOAT, GL_FALSE, sizeof(SkyboxVertex), offsetptr(SkyboxVertex, texCoords));
    glBindVertexArray(0);
}
