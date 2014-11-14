#include "stdafx.h"
#include "ChunkGridRenderStage.h"

#include "Camera.h"
#include "Chunk.h"
#include "Frustum.h"
#include "GLProgram.h"
#include "GameRenderParams.h"
#include "Mesh.h"


ChunkGridRenderStage::ChunkGridRenderStage(const GameRenderParams* gameRenderParams,
                                           const std::vector<ChunkSlot>& chunkSlots) :
    _gameRenderParams(gameRenderParams),
    _chunkSlots(chunkSlots) {
    // Empty
}


ChunkGridRenderStage::~ChunkGridRenderStage() {
    // Empty
}

void ChunkGridRenderStage::draw() {
    if (!_isVisible) return;
    // Element pattern
    const ui32 elementBuffer[24] = { 0, 1, 0, 2, 1, 3, 2, 3, 4, 5, 4, 6, 5, 7, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7 };
    // Shader that is lazily initialized
    static vg::GLProgram* chunkLineProgram = nullptr;
    // The mesh that is built from the chunks
    vcore::Mesh mesh;
    mesh.init(vcore::PrimitiveType::LINES, true);
    // Reserve the number of vertices and indices we think we will need
    mesh.reserve(_chunkSlots.size() * 8, _chunkSlots.size() * 24);
    // Build the mesh
    Chunk* chunk;
    ColorRGBA8 color;
    // Used to build each grid
    vector <vcore::MeshVertex> vertices(8);
    vector <ui32> indices(24);
    int numVertices = 0;

    f32v3 posOffset;

    for (i32 i = 0; i < _chunkSlots.size(); i++) {
        chunk = _chunkSlots[i].chunk;
        if (!chunk) continue;
        posOffset = f32v3(f64v3(chunk->gridPosition) - _gameRenderParams->chunkCamera->getPosition());

        if (((chunk->mesh && chunk->mesh->inFrustum) || SphereInFrustum((float)(posOffset.x + CHUNK_WIDTH / 2), (float)(posOffset.y + CHUNK_WIDTH / 2), (float)(posOffset.z + CHUNK_WIDTH / 2), 28.0f, gridFrustum))) {


            switch (chunk->getState()) {
                case ChunkStates::GENERATE:
                    color = ColorRGBA8(255, 0, 255, 255);
                    break;
                case ChunkStates::LOAD:
                    color = ColorRGBA8(255, 255, 255, 255);
                    break;
                case ChunkStates::LIGHT:
                    color = ColorRGBA8(255, 255, 0, 255);
                    break;
                case ChunkStates::TREES:
                    color = ColorRGBA8(0, 128, 0, 255);
                    break;
                case ChunkStates::DRAW:
                    color = ColorRGBA8(0, 0, 255, 255);
                    break;
                case ChunkStates::MESH:
                    color = ColorRGBA8(0, 255, 0, 255);
                    break;
                case ChunkStates::WATERMESH:
                    color = ColorRGBA8(0, 255, 255, 255);
                    break;
                default:
                    color = ColorRGBA8(0, 0, 0, 255);
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
            if (chunk->getState() != ChunkStates::INACTIVE) {
                // Build the vertices
                const float gmin = 0.00001;
                const float gmax = 31.9999;
                vertices[0].position = f32v3(gmin, gmin, gmin) + posOffset;
                vertices[1].position = f32v3(gmax, gmin, gmin) + posOffset;
                vertices[2].position = f32v3(gmin, gmin, gmax) + posOffset;
                vertices[3].position = f32v3(gmax, gmin, gmax) + posOffset;
                vertices[4].position = f32v3(gmin, gmax, gmin) + posOffset;
                vertices[5].position = f32v3(gmax, gmax, gmin) + posOffset;
                vertices[6].position = f32v3(gmin, gmax, gmax) + posOffset;
                vertices[7].position = f32v3(gmax, gmax, gmax) + posOffset;
            }
            mesh.addVertices(vertices, indices);
        }
    }
    // Check if a non-empty mesh was built
    if (numVertices != 0) {
        // Upload the data
        mesh.uploadAndClearLocal();
        // Lazily initialize shader
        if (chunkLineProgram == nullptr) {
            chunkLineProgram = new vg::GLProgram(true);
            chunkLineProgram->addShader(vg::ShaderType::VERTEX_SHADER, vcore::Mesh::defaultVertexShaderSource);
            chunkLineProgram->addShader(vg::ShaderType::FRAGMENT_SHADER, vcore::Mesh::defaultFragmentShaderSource);
            chunkLineProgram->setAttributes(vcore::Mesh::defaultShaderAttributes);
            chunkLineProgram->link();
            chunkLineProgram->initUniforms();
        }
        // Bind the program
        chunkLineProgram->use();
        // Set Matrix
        glUniformMatrix4fv(chunkLineProgram->getUniform("MVP"), 1, GL_FALSE, &(_gameRenderParams->VP[0][0]));
        // Set Texture
        glUniform1i(chunkLineProgram->getUniform("tex"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, BlankTextureID.ID);
        // Draw the grid
        mesh.draw();
        // Unuse the program
        chunkLineProgram->unuse();
    }
}
