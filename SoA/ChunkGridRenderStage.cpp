#include "stdafx.h"
#include "ChunkGridRenderStage.h"

#include <Vorb/graphics/GLProgram.h>

#include "Camera.h"
#include "Chunk.h"
#include "Frustum.h"
#include "GameRenderParams.h"
#include "ShaderLoader.h"
#include "soaUtils.h"
#include "ChunkGrid.h"

namespace {
    // Default shader source
    const cString VERT_SRC = R"(
uniform mat4 MVP;

in vec3 vPosition;
in vec4 vTint;
in vec2 vUV;

out vec4 fTint;

void main() {
    fTint = vTint;
    gl_Position = MVP * vec4(vPosition, 1.0);
}
)";
    const cString FRAG_SRC = R"(
uniform float unZCoef;

in vec4 fTint;

out vec4 fColor;

void main() {
    fColor = fTint;
}
)";
}

void ChunkGridRenderStage::hook(const GameRenderParams* gameRenderParams) {
    m_gameRenderParams = gameRenderParams;
}

/// NOTE: There is a race condition with _chunkSlots here, but since _chunkSlots is a read only vector,
/// it should not cause a crash. However data may be partially incorrect.
void ChunkGridRenderStage::render(const Camera* camera) {
    if (!m_isActive) return;
    if (!m_state) return;

    const std::vector<DebugChunkData>& chunkData = m_state->debugChunkData;
    
    // Element pattern
    const ui32 elementBuffer[24] = { 0, 1, 0, 2, 1, 3, 2, 3, 4, 5, 4, 6, 5, 7, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7 };
    // The mesh that is built from the chunks

    // Build the mesh
    ColorRGBA8 color;
    // Used to build each grid
    std::vector<ChunkGridVertex> vertices(chunkData.size() * 8);
    std::vector<ui32> indices(chunkData.size() * 24);
    int numVertices = 0;
    int numIndices = 0;

    f32v3 posOffset;

    for (auto& data : chunkData) {
        posOffset = f32v3(f64v3(data.voxelPosition) - m_gameRenderParams->chunkCamera->getPosition());

        if (true /*((chunk->mesh && chunk->mesh->inFrustum) || m_gameRenderParams->chunkCamera->sphereInFrustum(posOffset + f32v3(CHUNK_WIDTH / 2), 28.0f))*/) {

            switch (data.genLevel) {
                case GEN_DONE:
                    color = ColorRGBA8(0, 0, 255, 255);
                    break;
                case GEN_FLORA:
                    color = ColorRGBA8(0, 255, 0, 255);
                    break;
                default:
                    color = ColorRGBA8(255, 0, 0, 255);
                    break;
            }
            for (int i = 0; i < 8; i++) {
                vertices[numVertices + i].color = color;
                vertices[numVertices + i].uv = f32v2(0.0f, 0.0f);
            }
            // Build the indices
            for (int i = 0; i < 24; i++) {
                indices.push_back(numVertices + elementBuffer[i]);
            }
            
            // Build the vertices
            const f32 gmin = 0.01f;
            const f32 gmax = 31.99f;
            vertices[numVertices + 0].position = f32v3(gmin, gmin, gmin) + posOffset;
            vertices[numVertices + 1].position = f32v3(gmax, gmin, gmin) + posOffset;
            vertices[numVertices + 2].position = f32v3(gmin, gmin, gmax) + posOffset;
            vertices[numVertices + 3].position = f32v3(gmax, gmin, gmax) + posOffset;
            vertices[numVertices + 4].position = f32v3(gmin, gmax, gmin) + posOffset;
            vertices[numVertices + 5].position = f32v3(gmax, gmax, gmin) + posOffset;
            vertices[numVertices + 6].position = f32v3(gmin, gmax, gmax) + posOffset;
            vertices[numVertices + 7].position = f32v3(gmax, gmax, gmax) + posOffset;

            numVertices += 8;
            numIndices += 24;
        }
    }

    // Check for non-empty mesh then draw
    if(numVertices != 0) drawGrid(vertices, indices);
}

void ChunkGridRenderStage::drawGrid(std::vector<ChunkGridVertex> vertices, std::vector<ui32> indices) {
    GLuint vbo;
    GLuint ibo;
    GLuint vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // Generate and bind VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Generate and bind element buffer
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Set attribute arrays
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    // Set attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(ChunkGridVertex), offsetptr(ChunkGridVertex, position));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(ChunkGridVertex), offsetptr(ChunkGridVertex, color));
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(ChunkGridVertex), offsetptr(ChunkGridVertex, uv));
    // Unbind VAO
    glBindVertexArray(0);

    // Upload the data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ChunkGridVertex)* vertices.size(), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ChunkGridVertex)* vertices.size(), vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLuint numVertices = vertices.size();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ui32)* indices.size(), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(ui32)* indices.size(), indices.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLsizei numIndices = (GLsizei)indices.size();

    // Lazily initialize shader
    if(!m_program.isCreated()) m_program = ShaderLoader::createProgram("ChunkLine", VERT_SRC, FRAG_SRC);

    // Bind the program
    m_program.use();

    // Set Matrix
    glUniformMatrix4fv(m_program.getUniform("MVP"), 1,
        GL_FALSE,
        &(m_gameRenderParams->chunkCamera->getViewProjectionMatrix()[0][0]));
    // Draw the grid     
    // Bind the VAO
    glBindVertexArray(vao);
    // Perform draw call
    glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    // Unuse the program
    m_program.unuse();

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}
