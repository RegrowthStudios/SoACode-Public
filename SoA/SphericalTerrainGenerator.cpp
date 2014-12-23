#include "stdafx.h"
#include "SphericalTerrainGenerator.h"
#include "SphericalTerrainComponent.h"

#include "GpuMemory.h"
#include "Errors.h"

const ColorRGB8 DebugColors[6] {
    ColorRGB8(255, 0, 0), //TOP
    ColorRGB8(0, 255, 0), //LEFT
    ColorRGB8(0, 0, 255), //RIGHT
    ColorRGB8(255, 255, 0), //FRONT
    ColorRGB8(0, 255, 255), //BACK
    ColorRGB8(255, 255, 255) //BOTTOM
};

TerrainVertex SphericalTerrainGenerator::verts[SphericalTerrainGenerator::VERTS_SIZE];
ui16 SphericalTerrainGenerator::indices[SphericalTerrainPatch::INDICES_PER_PATCH];
float SphericalTerrainGenerator::m_heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH];

VGIndexBuffer SphericalTerrainGenerator::m_cwIbo = 0; ///< Reusable CW IBO
VGIndexBuffer SphericalTerrainGenerator::m_ccwIbo = 0; ///< Reusable CCW IBO

SphericalTerrainGenerator::SphericalTerrainGenerator(float radius, vg::GLProgram* genProgram,
                                                     vg::GLProgram* normalProgram) :
    m_radius(radius),
    m_genProgram(genProgram),
    m_normalProgram(normalProgram),
    unCornerPos(m_genProgram->getUniform("unCornerPos")),
    unCoordMapping(m_genProgram->getUniform("unCoordMapping")),
    unPatchWidth(m_genProgram->getUniform("unPatchWidth")),
    unHeightMap(m_normalProgram->getUniform("unHeightMap")),
    unWidth(m_normalProgram->getUniform("unWidth")),
    unTexelWidth(m_normalProgram->getUniform("unTexelWidth")) {

    m_heightMapDims = ui32v2(PATCH_HEIGHTMAP_WIDTH);
    for (int i = 0; i < PATCHES_PER_FRAME; i++) {
        m_textures[i].init(m_heightMapDims);
    }
    m_quad.init();

    glGenFramebuffers(1, &m_normalFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_normalFbo);

    // Set the output location for pixels
    VGEnum att = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &att);

    // Unbind used resources
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Construct reusable index buffer objects
    if (m_cwIbo == 0) {
        generateIndices(m_cwIbo, false);
        generateIndices(m_ccwIbo, true);
    }

}

SphericalTerrainGenerator::~SphericalTerrainGenerator() {
    // Empty
}

void SphericalTerrainGenerator::update() {

    m_patchCounter = 0;

    // Heightmap Generation
    m_genProgram->use();
    m_genProgram->enableVertexAttribArrays();
    glDisable(GL_DEPTH_TEST);

    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
    m_rpcManager.processRequests(PATCHES_PER_FRAME);

    m_genProgram->disableVertexAttribArrays();
    m_genProgram->unuse();

    if (m_patchCounter) {
        // Normal map generation
        m_normalProgram->enableVertexAttribArrays();
        m_normalProgram->use();

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(unHeightMap, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, m_normalFbo);
        glViewport(0, 0, PATCH_NORMALMAP_WIDTH, PATCH_NORMALMAP_WIDTH);

        glFlush();
        glFinish();
        // Loop through all textures
        for (int i = 0; i < m_patchCounter; i++) {
            TerrainGenDelegate* data = m_delegates[i];
        
            // Create and bind output normal map
            if (data->mesh->m_normalMap == 0) {
                glGenTextures(1, &data->mesh->m_normalMap);
                glBindTexture(GL_TEXTURE_2D, data->mesh->m_normalMap);
                glTexImage2D(GL_TEXTURE_2D, 0, (VGEnum)NORMALGEN_INTERNAL_FORMAT, PATCH_NORMALMAP_WIDTH, PATCH_NORMALMAP_WIDTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                SamplerState::POINT_CLAMP.set(GL_TEXTURE_2D);
            } else {
                glBindTexture(GL_TEXTURE_2D, data->mesh->m_normalMap);
            }
            // Bind texture to fbo
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->mesh->m_normalMap, 0);

            // Bind the heightmap texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_textures[i].getTextureIDs().height);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, m_heightData);
            glUniform1f(unWidth, data->width / PATCH_HEIGHTMAP_WIDTH);
            glUniform1f(unTexelWidth, (float)PATCH_HEIGHTMAP_WIDTH);

            // Generate normal map
            m_quad.draw();

            // And finally build the mesh
            buildMesh(data);
            data->inUse = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_normalProgram->disableVertexAttribArrays();
        m_normalProgram->unuse();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
  
}

void SphericalTerrainGenerator::generateTerrain(TerrainGenDelegate* data) {
  
    m_textures[m_patchCounter].use();
    m_delegates[m_patchCounter] = data;

    // Send uniforms
    glUniform3fv(unCornerPos, 1, &data->startPos[0]);
    glUniform3iv(unCoordMapping, 1, &data->coordMapping[0]);
    glUniform1f(unPatchWidth, data->width);

    m_quad.draw();

    TerrainGenTextures::unuse();

    m_patchCounter++;
}

void SphericalTerrainGenerator::buildMesh(TerrainGenDelegate* data) {

    SphericalTerrainMesh* mesh = data->mesh;
    // Get debug face color
    const ColorRGB8 tcolor = DebugColors[(int)mesh->m_cubeFace];

    // Grab mappings so we can rotate the 2D grid appropriately
    const i32v3& coordMapping = data->coordMapping;
    const f32v3& startPos = data->startPos;
    
    float width = data->width;
    float h;
    f32v3 tmpPos;
  
    // Loop through and set all vertex attributes
    m_vertWidth = width / (PATCH_WIDTH - 1);
    m_index = 0;
    for (int z = 0; z < PATCH_WIDTH; z++) {
        for (int x = 0; x < PATCH_WIDTH; x++) {
            auto& v = verts[m_index];
            // Set the position based on which face we are on
            v.position[coordMapping.x] = x * m_vertWidth + startPos.x;
            v.position[coordMapping.y] = startPos.y;
            v.position[coordMapping.z] = z * m_vertWidth + startPos.z;

            // Get Height 
            h = m_heightData[z * PIXELS_PER_PATCH_NM + 1][x * PIXELS_PER_PATCH_NM + 1];

            // Set texture coordinates
            v.texCoords.x = (ui8)(((float)x / (float)PATCH_WIDTH) * 255.0f);
            v.texCoords.y = (ui8)(((float)z / (float)PATCH_WIDTH) * 255.0f);

            // Spherify it!
            v.position = glm::normalize(v.position) * (m_radius + h);
          
            // Compute tangent
            tmpPos[coordMapping.x] = (x + 1) * m_vertWidth + startPos.x;
            tmpPos[coordMapping.y] = startPos.y;
            tmpPos[coordMapping.z] = (z)* m_vertWidth + startPos.z;
            tmpPos = glm::normalize(tmpPos) * (m_radius + h);
            v.tangent = glm::normalize(tmpPos - v.position);

            // Make sure tangent is orthogonal
            f32v3 binormal = glm::normalize(glm::cross(glm::normalize(v.position), v.tangent));
            v.tangent = glm::normalize(glm::cross(binormal, glm::normalize(v.position)));

            v.color.r = tcolor.r;
            v.color.g = tcolor.g;
            v.color.b = tcolor.b;
            m_index++;
        }
    }

    // TMP: Approximate position with middle
    mesh->m_worldPosition = verts[m_index - PATCH_SIZE / 2].position;

    buildSkirts();
   
    // If the buffers haven't been generated, generate them
    if (mesh->m_vbo == 0) {
        //     glGenVertexArrays(1, &m_vao);
        vg::GpuMemory::createBuffer(mesh->m_vbo);
    }

    if (CubeWindings[(int)mesh->m_cubeFace]) {
        mesh->m_ibo = m_ccwIbo;
    } else {
        mesh->m_ibo = m_cwIbo;
    }

    // TODO: Using a VAO makes it not work??
    //  glBindVertexArray(m_vao);

    // Upload buffer data
    vg::GpuMemory::bindBuffer(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER,
                                    VERTS_SIZE * sizeof(TerrainVertex),
                                    verts);

    // TODO: Using a VAO makes it not work??
    //   glBindVertexArray(0);
}

void SphericalTerrainGenerator::buildSkirts() {
    const float SKIRT_DEPTH = m_vertWidth * 2.0f;
    // Top Skirt
    for (int i = 0; i < PATCH_WIDTH; i++) {
        auto& v = verts[m_index];
        // Copy the vertices from the top edge
        v = verts[i];
        // Extrude downward
        float len = glm::length(v.position) - SKIRT_DEPTH;
        v.position = glm::normalize(v.position) * len;
        m_index++;
    }
    // Left Skirt
    for (int i = 0; i < PATCH_WIDTH; i++) {
        auto& v = verts[m_index];
        // Copy the vertices from the left edge
        v = verts[i * PATCH_WIDTH];
        // Extrude downward
        float len = glm::length(v.position) - SKIRT_DEPTH;
        v.position = glm::normalize(v.position) * len;
        m_index++;
    }
    // Right Skirt
    for (int i = 0; i < PATCH_WIDTH; i++) {
        auto& v = verts[m_index];
        // Copy the vertices from the right edge
        v = verts[i * PATCH_WIDTH + PATCH_WIDTH - 1];
        // Extrude downward
        float len = glm::length(v.position) - SKIRT_DEPTH;
        v.position = glm::normalize(v.position) * len;
        m_index++;
    }
    // Bottom Skirt
    for (int i = 0; i < PATCH_WIDTH; i++) {
        auto& v = verts[m_index];
        // Copy the vertices from the bottom edge
        v = verts[PATCH_SIZE - PATCH_WIDTH + i];
        // Extrude downward
        float len = glm::length(v.position) - SKIRT_DEPTH;
        v.position = glm::normalize(v.position) * len;
        m_index++;
    }
}

void SphericalTerrainGenerator::generateIndices(VGIndexBuffer& ibo, bool ccw) {
    // Loop through each quad and set indices
    int vertIndex;
    int index = 0;
    int skirtIndex = PATCH_SIZE;
    if (ccw) {
        // CCW
        // Main vertices
        for (int z = 0; z < PATCH_WIDTH - 1; z++) {
            for (int x = 0; x < PATCH_WIDTH - 1; x++) {
                // Compute index of back left vertex
                vertIndex = z * PATCH_WIDTH + x;
                // Change triangle orientation based on odd or even
                if ((x + z) % 2) {
                    indices[index] = vertIndex;
                    indices[index + 1] = vertIndex + PATCH_WIDTH;
                    indices[index + 2] = vertIndex + PATCH_WIDTH + 1;
                    indices[index + 3] = vertIndex + PATCH_WIDTH + 1;
                    indices[index + 4] = vertIndex + 1;
                    indices[index + 5] = vertIndex;
                } else {
                    indices[index] = vertIndex + 1;
                    indices[index + 1] = vertIndex;
                    indices[index + 2] = vertIndex + PATCH_WIDTH;
                    indices[index + 3] = vertIndex + PATCH_WIDTH;
                    indices[index + 4] = vertIndex + PATCH_WIDTH + 1;
                    indices[index + 5] = vertIndex + 1;
                }
                index += SphericalTerrainPatch::INDICES_PER_QUAD;
            }
        }
        // Skirt vertices
        // Top Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i;
            indices[index] = skirtIndex;
            indices[index + 1] = vertIndex;
            indices[index + 2] = vertIndex + 1;
            indices[index + 3] = vertIndex + 1;
            indices[index + 4] = skirtIndex + 1;
            indices[index + 5] = skirtIndex;
            index += SphericalTerrainPatch::INDICES_PER_QUAD;
            skirtIndex++;
        }
        skirtIndex++; // Skip last vertex
        // Left Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i * PATCH_WIDTH;
            indices[index] = skirtIndex;
            indices[index + 1] = skirtIndex + 1;
            indices[index + 2] = vertIndex + PATCH_WIDTH;
            indices[index + 3] = vertIndex + PATCH_WIDTH;
            indices[index + 4] = vertIndex;
            indices[index + 5] = skirtIndex;
            index += SphericalTerrainPatch::INDICES_PER_QUAD;
            skirtIndex++;
        }
        skirtIndex++; // Skip last vertex
        // Right Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i * PATCH_WIDTH + PATCH_WIDTH - 1;
            indices[index] = vertIndex;
            indices[index + 1] = vertIndex + PATCH_WIDTH;
            indices[index + 2] = skirtIndex + 1;
            indices[index + 3] = skirtIndex + 1;
            indices[index + 4] = skirtIndex;
            indices[index + 5] = vertIndex;
            index += SphericalTerrainPatch::INDICES_PER_QUAD;
            skirtIndex++;
        }
        skirtIndex++;
        // Bottom Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = PATCH_SIZE - PATCH_WIDTH + i;
            indices[index] = vertIndex;
            indices[index + 1] = skirtIndex;
            indices[index + 2] = skirtIndex + 1;
            indices[index + 3] = skirtIndex + 1;
            indices[index + 4] = vertIndex + 1;
            indices[index + 5] = vertIndex;
            index += SphericalTerrainPatch::INDICES_PER_QUAD;
            skirtIndex++;
        }

    } else {
        //CW
        // Main vertices
        for (int z = 0; z < PATCH_WIDTH - 1; z++) {
            for (int x = 0; x < PATCH_WIDTH - 1; x++) {
                // Compute index of back left vertex
                vertIndex = z * PATCH_WIDTH + x;
                // Change triangle orientation based on odd or even
                if ((x + z) % 2) {
                    indices[index] = vertIndex;
                    indices[index + 1] = vertIndex + 1;
                    indices[index + 2] = vertIndex + PATCH_WIDTH + 1;
                    indices[index + 3] = vertIndex + PATCH_WIDTH + 1;
                    indices[index + 4] = vertIndex + PATCH_WIDTH;
                    indices[index + 5] = vertIndex;
                } else {
                    indices[index] = vertIndex + 1;
                    indices[index + 1] = vertIndex + PATCH_WIDTH + 1;
                    indices[index + 2] = vertIndex + PATCH_WIDTH;
                    indices[index + 3] = vertIndex + PATCH_WIDTH;
                    indices[index + 4] = vertIndex;
                    indices[index + 5] = vertIndex + 1;
                }
                index += SphericalTerrainPatch::INDICES_PER_QUAD;
            }
        }
        // Skirt vertices
        // Top Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i;
            indices[index] = skirtIndex;
            indices[index + 1] = skirtIndex + 1;
            indices[index + 2] = vertIndex + 1;
            indices[index + 3] = vertIndex + 1;
            indices[index + 4] = vertIndex;
            indices[index + 5] = skirtIndex;
            index += SphericalTerrainPatch::INDICES_PER_QUAD;
            skirtIndex++;
        }
        skirtIndex++; // Skip last vertex
        // Left Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i * PATCH_WIDTH;
            indices[index] = skirtIndex;
            indices[index + 1] = vertIndex;
            indices[index + 2] = vertIndex + PATCH_WIDTH;
            indices[index + 3] = vertIndex + PATCH_WIDTH;
            indices[index + 4] = skirtIndex + 1;
            indices[index + 5] = skirtIndex;
            index += SphericalTerrainPatch::INDICES_PER_QUAD;
            skirtIndex++;
        }
        skirtIndex++; // Skip last vertex
        // Right Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i * PATCH_WIDTH + PATCH_WIDTH - 1;
            indices[index] = vertIndex;
            indices[index + 1] = skirtIndex;
            indices[index + 2] = skirtIndex + 1;
            indices[index + 3] = skirtIndex + 1;
            indices[index + 4] = vertIndex + PATCH_WIDTH;
            indices[index + 5] = vertIndex;
            index += SphericalTerrainPatch::INDICES_PER_QUAD;
            skirtIndex++;
        }
        skirtIndex++;
        // Bottom Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = PATCH_SIZE - PATCH_WIDTH + i;
            indices[index] = vertIndex;
            indices[index + 1] = vertIndex + 1;
            indices[index + 2] = skirtIndex + 1;
            indices[index + 3] = skirtIndex + 1;
            indices[index + 4] = skirtIndex;
            indices[index + 5] = vertIndex;
            index += SphericalTerrainPatch::INDICES_PER_QUAD;
            skirtIndex++;
        }
    }
    
    vg::GpuMemory::createBuffer(ibo);
    vg::GpuMemory::bindBuffer(ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER,
                                    SphericalTerrainPatch::INDICES_PER_PATCH * sizeof(ui16),
                                    indices);
}