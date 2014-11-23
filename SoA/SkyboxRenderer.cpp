#include "stdafx.h"
#include "SkyboxRenderer.h"

#include "GLEnums.h"
#include "GpuMemory.h"

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

const float skyboxUVs[48] = { 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,  // v1-v2-v3-v0 (front)
1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // v0-v3-v4-v5 (right)
1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,// v6-v1-v0-v5 (top)
1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,// v6-v7-v2-v1 (left)
1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,  // v7-v4-v3-v2 (bottom)
1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 }; // v5-v4-v7-v6 (back)

SkyboxRenderer::SkyboxRenderer() : 
    _vao(0),
    _vbo(0),
    _ibo(0) {
    // Empty
}


SkyboxRenderer::~SkyboxRenderer() {
    destroy();
}

void SkyboxRenderer::drawSkybox(vg::GLProgram* program, const f32m4& VP, vg::Texture textures[]) {

    // Bind shader
    program->use();

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(program->getUniform("unTex"), 0);
    // Upload VP matrix
    glUniformMatrix4fv(program->getUniform("unWVP"), 1, GL_FALSE, &VP[0][0]);
    
    // Create the buffer objects if they aren't initialized
    if (_vbo == 0) {
        initBuffers(program);
    } else {
        glBindVertexArray(_vao);
    }

    // Draw each of the 6 sides
    for (int i = 0; i < SKYBOX_FACES; i++) {
        glBindTexture(GL_TEXTURE_2D, textures[i].ID);
        glDrawElements(GL_TRIANGLES, INDICES_PER_QUAD, GL_UNSIGNED_SHORT, (void*)(i * sizeof(ui16)* INDICES_PER_QUAD)); //offset
    }

    // Unbind shader
    program->unuse();

    // Always unbind the vao
    glBindVertexArray(0);

}

void SkyboxRenderer::destroy() {
    if (_vao) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
    if (_vbo) {
        vg::GpuMemory::freeBuffer(_vbo);
    }
    if (_ibo) {
        vg::GpuMemory::freeBuffer(_ibo);
    }
}

void SkyboxRenderer::initBuffers(vg::GLProgram* program) {
    // Vertex Array Object
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // Vertex Buffer Object
    vg::GpuMemory::createBuffer(_vbo);
    vg::GpuMemory::bindBuffer(_vbo, vg::BufferTarget::ARRAY_BUFFER);

    SkyboxVertex verts[VERTS_PER_QUAD * SKYBOX_FACES];
    for (int i = 0; i < VERTS_PER_QUAD * SKYBOX_FACES; i++) {
        verts[i].position = f32v3(skyboxVertices[i * 3],
                                  skyboxVertices[i * 3 + 1],
                                  skyboxVertices[i * 3 + 2]);
        verts[i].texCoords = f32v2(skyboxUVs[i * 2], skyboxUVs[i * 2 + 1]);
    }

    vg::GpuMemory::uploadBufferData(_vbo, vg::BufferTarget::ARRAY_BUFFER, sizeof(verts), verts, vg::BufferUsageHint::STATIC_DRAW);

    vg::GpuMemory::createBuffer(_ibo);
    vg::GpuMemory::bindBuffer(_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);

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

    vg::GpuMemory::uploadBufferData(_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, vg::BufferUsageHint::STATIC_DRAW);

    // Set up attribute pointers
    program->enableVertexAttribArrays();
    glVertexAttribPointer(program->getAttribute("vPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(SkyboxVertex), offsetptr(SkyboxVertex, position));
    glVertexAttribPointer(program->getAttribute("vUV"), 2, GL_FLOAT, GL_FALSE, sizeof(SkyboxVertex), offsetptr(SkyboxVertex, texCoords));
}
