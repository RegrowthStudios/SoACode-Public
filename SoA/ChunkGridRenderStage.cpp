#include "stdafx.h"
#include "ChunkGridRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/Mesh.h>

#include "Camera.h"
#include "NChunk.h"
#include "Frustum.h"
#include "GameRenderParams.h"
#include "ShaderLoader.h"
#include "soaUtils.h"
#include "NChunkGrid.h"

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
in float fLogZ;

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
    vg::Mesh mesh;
    mesh.init(vg::PrimitiveType::LINES, true);
    // Reserve the number of vertices and indices we think we will need
    mesh.reserve(200/*chunks.size()*/ * 8, 200/*chunks.size()*/ * 24);
    // Build the mesh
    ColorRGBA8 color;
    // Used to build each grid
    std::vector<vg::MeshVertex> vertices(8);
    std::vector<ui32> indices(24);
    int numVertices = 0;

    f32v3 posOffset;

    for (auto& data : chunkData) {
        posOffset = f32v3(f64v3(data.voxelPosition) - m_gameRenderParams->chunkCamera->getPosition());

        if (1 /*((chunk->mesh && chunk->mesh->inFrustum) || m_gameRenderParams->chunkCamera->sphereInFrustum(posOffset + f32v3(CHUNK_WIDTH / 2), 28.0f))*/) {

            switch (data.genLevel) {
                case GEN_DONE:
                    color = ColorRGBA8(0, 0, 255, 255);
                    break;
                default:
                    color = ColorRGBA8(255, 0, 0, 255);
                    break;
            }
            for (int i = 0; i < 8; i++) {
                vertices[i].color = color;
                vertices[i].uv = f32v2(0.0f, 0.0f);
            }
            // Build the indices
            for (int i = 0; i < 24; i++) {
                indices[i] = numVertices + elementBuffer[i];
            }
            numVertices += 8;
            
            // Build the vertices
            const f32 gmin = 0.00001f;
            const f32 gmax = 31.9999f;
            vertices[0].position = f32v3(gmin, gmin, gmin) + posOffset;
            vertices[1].position = f32v3(gmax, gmin, gmin) + posOffset;
            vertices[2].position = f32v3(gmin, gmin, gmax) + posOffset;
            vertices[3].position = f32v3(gmax, gmin, gmax) + posOffset;
            vertices[4].position = f32v3(gmin, gmax, gmin) + posOffset;
            vertices[5].position = f32v3(gmax, gmax, gmin) + posOffset;
            vertices[6].position = f32v3(gmin, gmax, gmax) + posOffset;
            vertices[7].position = f32v3(gmax, gmax, gmax) + posOffset;
            mesh.addVertices(vertices, indices);       
        }
    }
    
    // Check if a non-empty mesh was built
    if (numVertices != 0) {
        // Upload the data
        mesh.uploadAndClearLocal();
        // Lazily initialize shader
        if (!m_program.isCreated()) m_program = ShaderLoader::createProgram("ChunkLine", VERT_SRC, FRAG_SRC);

        // Bind the program
        m_program.use();

        // Set Matrix
        glUniformMatrix4fv(m_program.getUniform("MVP"), 1,
                           GL_FALSE,
                           &(m_gameRenderParams->chunkCamera->getViewProjectionMatrix()[0][0]));
        glDisable(GL_DEPTH_TEST);
        // Draw the grid
        mesh.draw();
        // Unuse the program
        m_program.unuse();
        glEnable(GL_DEPTH_TEST);
    }
}
