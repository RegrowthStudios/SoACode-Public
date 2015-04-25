#include "stdafx.h"
#include "LenseFlareRenderer.h"

#include "ModPathResolver.h"
#include "ShaderLoader.h"
#include "Errors.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/SamplerState.h>
#include <Vorb/graphics/ShaderManager.h>

struct FlareVertex {
    f32v2 position;
    ui8v2 uv;
    ui8v2 padding;
    f32 offset;
};

const int VERTS_PER_QUAD = 4;
const int INDICES_PER_QUAD = 6;
const int NUM_RINGS = 2;
const int NUM_GLOWS = 4;
const int NUM_QUADS = NUM_GLOWS + NUM_RINGS;
const f32 const RING_SIZES[NUM_RINGS] = { 1.3f, 1.0f };
const f32 const GLOW_SIZES[NUM_GLOWS] = { 1.75f, 0.65f, 0.9f, 0.45f };
// Offsets for the positions of each successive quad
const f32 const offsets[NUM_QUADS] = {
    1.0f, 1.25f, 1.1f, 1.5f, 1.6f, 1.7f
};

LenseFlareRenderer::LenseFlareRenderer(const ModPathResolver* textureResolver) :
m_textureResolver(textureResolver) {
    // Empty
}

LenseFlareRenderer::~LenseFlareRenderer() {
    dispose();
}

void LenseFlareRenderer::render(const f32m4& VP, const f64v3& relCamPos,
                                const f32v3& color,
                                float aspectRatio,
                                f32 size,
                                f32 intensity) {
    if (size <= 0.0f || intensity <= 0.0f) return;
    if (!m_program) {
        lazyInit();
    }

    m_program->use();

    f32v2 dims(size, size * aspectRatio);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    // Upload uniforms
    f32v3 center(-relCamPos);
    glUniform1f(m_program->getUniform("unIntensity"), intensity);
    glUniform3fv(m_program->getUniform("unCenter"), 1, &center[0]);
    glUniform3fv(m_program->getUniform("unColor"), 1, &color[0]);
    glUniform2fv(m_program->getUniform("unDims"), 1, &dims[0]);
    glUniformMatrix4fv(m_program->getUniform("unVP"), 1, GL_FALSE, &VP[0][0]);

    glBindVertexArray(m_vao);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDrawElements(GL_TRIANGLES, INDICES_PER_QUAD * NUM_QUADS, GL_UNSIGNED_SHORT, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);

    m_program->unuse();

}

void LenseFlareRenderer::dispose() {
    if (m_program) {
        vg::ShaderManager::destroyProgram(&m_program);
    }
    if (m_texture) {
        vg::GpuMemory::freeTexture(m_texture);
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ibo) {
        glDeleteBuffers(1, &m_ibo);
        m_vbo = 0;
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

void LenseFlareRenderer::lazyInit() {
   
    { // Load the shader
        m_program = ShaderLoader::createProgramFromFile("Shaders/LensFlare/flare.vert",
                                                        "Shaders/LensFlare/flare.frag");
        m_unColor = m_program->getUniform("unColor");
        // Set constant uniforms
        m_program->use();
        glUniform1i(m_program->getUniform("unTexture"), 0);
        m_program->unuse();
    }
    
    { // Load the texture
        vio::Path path;
        m_textureResolver->resolvePath("Effects/lens_flares.png", path);
        vg::BitmapResource res = vg::ImageIO().load(path);
        if (!res.data) {
            fprintf(stderr, "ERROR: Failed to load Effects/lens_flares.png\n");
        }

        m_texture = vg::GpuMemory::uploadTexture(res.bytesUI8, res.width, res.height, &vg::SamplerState::LINEAR_CLAMP_MIPMAP);
        vg::ImageIO::free(res);
    }

    initMesh();
}

void LenseFlareRenderer::initMesh() {
    const f32v2 positions[4] = {
        f32v2(-1.0f, 1.0f),
        f32v2(-1.0f, -1.0f),
        f32v2(1.0f, -1.0f),
        f32v2(1.0f, 1.0f)
    };
    const ui8v2 uvs[2][4] = {
        {
            ui8v2(127, 255),
            ui8v2(127, 0),
            ui8v2(255, 0),
            ui8v2(255, 255)
        },
        {
            ui8v2(0, 255),
            ui8v2(0, 0),
            ui8v2(127, 0),
            ui8v2(127, 255)
        }
    };
    const ui16 quadIndices[INDICES_PER_QUAD] = { 0, 1, 2, 2, 3, 0 };

    FlareVertex vertices[NUM_QUADS * VERTS_PER_QUAD];
    ui16 indices[NUM_QUADS * INDICES_PER_QUAD];
    int index = 0;
    // Rings
    for (int i = 0; i < NUM_RINGS; i++) {
        vertices[index].position = positions[0] * RING_SIZES[i];
        vertices[index + 1].position = positions[1] * RING_SIZES[i];
        vertices[index + 2].position = positions[2] * RING_SIZES[i];
        vertices[index + 3].position = positions[3] * RING_SIZES[i];
        vertices[index].uv = uvs[0][0];
        vertices[index + 1].uv = uvs[0][1];
        vertices[index + 2].uv = uvs[0][2];
        vertices[index + 3].uv = uvs[0][3];
        index += 4;
    }
    // Glows
    for (int i = 0; i < NUM_GLOWS; i++) {
        vertices[index].position = positions[0] * GLOW_SIZES[i];
        vertices[index + 1].position = positions[1] * GLOW_SIZES[i];
        vertices[index + 2].position = positions[2] * GLOW_SIZES[i];
        vertices[index + 3].position = positions[3] * GLOW_SIZES[i];
        vertices[index].uv = uvs[1][0];
        vertices[index + 1].uv = uvs[1][1];
        vertices[index + 2].uv = uvs[1][2];
        vertices[index + 3].uv = uvs[1][3];
        index += 4;
    }
    // set offsets
    for (int i = 0; i < NUM_QUADS; i++) {
        vertices[i * 4].offset = offsets[i];
        vertices[i * 4 + 1].offset = offsets[i];
        vertices[i * 4 + 2].offset = offsets[i];
        vertices[i * 4 + 3].offset = offsets[i];
    }
    // Set indices
    for (int i = 0; i < NUM_QUADS; i++) {
        for (int j = 0; j < INDICES_PER_QUAD; j++) {
            indices[i * INDICES_PER_QUAD + j] = quadIndices[j] + i * VERTS_PER_QUAD;
        }
    }

    if (m_vbo == 0) glGenBuffers(1, &m_vbo);
    if (m_ibo == 0) glGenBuffers(1, &m_ibo);
    if (m_vao == 0) glGenVertexArrays(1, &m_vao);

    // Upload data and make VAO
    glBindVertexArray(m_vao);
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);

    vg::GpuMemory::uploadBufferData(m_vbo, vg::BufferTarget::ARRAY_BUFFER, sizeof(vertices), vertices);
    vg::GpuMemory::uploadBufferData(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, sizeof(indices), indices);

    m_program->enableVertexAttribArrays();
    glVertexAttribPointer(m_program->getAttribute("vPosition"), 2, GL_FLOAT, GL_FALSE, sizeof(FlareVertex), offsetptr(FlareVertex, position));
    glVertexAttribPointer(m_program->getAttribute("vUV"), 2, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(FlareVertex), offsetptr(FlareVertex, uv));
    glVertexAttribPointer(m_program->getAttribute("vOffset"), 1, GL_FLOAT, GL_FALSE, sizeof(FlareVertex), offsetptr(FlareVertex, offset));

    glBindVertexArray(0);
}