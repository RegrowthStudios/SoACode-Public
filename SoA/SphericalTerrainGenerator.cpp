#include "stdafx.h"
#include "SphericalTerrainGenerator.h"

#include "Errors.h"
#include "GpuMemory.h"
#include "PlanetLoader.h"
#include "SphericalTerrainComponent.h"
#include "SphericalTerrainMeshManager.h"
#include "TextureRecycler.hpp"
#include "Timing.h"

const ColorRGB8 DebugColors[6] {
    ColorRGB8(255, 0, 0), //TOP
    ColorRGB8(0, 255, 0), //LEFT
    ColorRGB8(0, 0, 255), //RIGHT
    ColorRGB8(255, 255, 0), //FRONT
    ColorRGB8(0, 255, 255), //BACK
    ColorRGB8(255, 255, 255) //BOTTOM
};

TerrainVertex SphericalTerrainGenerator::verts[SphericalTerrainGenerator::VERTS_SIZE];
WaterVertex SphericalTerrainGenerator::waterVerts[SphericalTerrainGenerator::VERTS_SIZE];
float SphericalTerrainGenerator::m_heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][3];

ui16 SphericalTerrainGenerator::waterIndexGrid[PATCH_WIDTH][PATCH_WIDTH];
ui16 SphericalTerrainGenerator::waterIndices[SphericalTerrainPatch::INDICES_PER_PATCH];
bool SphericalTerrainGenerator::waterQuads[PATCH_WIDTH - 1][PATCH_WIDTH - 1];

VGIndexBuffer SphericalTerrainGenerator::m_cwIbo = 0; ///< Reusable CW IBO
VGIndexBuffer SphericalTerrainGenerator::m_ccwIbo = 0; ///< Reusable CCW IBO

SphericalTerrainGenerator::SphericalTerrainGenerator(float radius,
                                                     SphericalTerrainMeshManager* meshManager,
                                                     PlanetGenData* planetGenData,
                                                     vg::GLProgram* normalProgram,
                                                     vg::TextureRecycler* normalMapRecycler) :
    m_radius(radius),
    m_meshManager(meshManager),
    m_planetGenData(planetGenData),
    m_genProgram(planetGenData->program),
    m_normalProgram(normalProgram),
    m_normalMapRecycler(normalMapRecycler),
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

    // Generate pixel buffer objects
    for (int i = 0; i < PATCHES_PER_FRAME; i++) {
        vg::GpuMemory::createBuffer(m_pbos[i]);
        vg::GpuMemory::bindBuffer(m_pbos[i], vg::BufferTarget::PIXEL_PACK_BUFFER);
        glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(m_heightData), NULL, GL_STREAM_READ);
    }

}

SphericalTerrainGenerator::~SphericalTerrainGenerator() {
    for (int i = 0; i < PATCHES_PER_FRAME; i++) {
        vg::GpuMemory::freeBuffer(m_pbos[i]);
    }
}

void SphericalTerrainGenerator::update() {

    // Need to disable alpha blending
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

    if (m_patchCounter) {
        // Normal map generation
        m_normalProgram->enableVertexAttribArrays();
        m_normalProgram->use();

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(unHeightMap, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, m_normalFbo);
        glViewport(0, 0, PATCH_NORMALMAP_WIDTH, PATCH_NORMALMAP_WIDTH);

        // Loop through all textures
        for (int i = 0; i < m_patchCounter; i++) {

            TerrainGenDelegate* data = m_delegates[i];
        
            // Create and bind output normal map
            if (data->mesh->m_normalMap == 0) {
                data->mesh->m_normalMap = m_normalMapRecycler->produce();
            } else {
                glBindTexture(GL_TEXTURE_2D, data->mesh->m_normalMap);
            }

           // std::cout << m_normalMapRecycler->getNumTextures() << " " << vg::GpuMemory::getTextureVramUsage() / 1024.0 / 1024.0 << std::endl;

            // Bind normal map texture to fbo
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->mesh->m_normalMap, 0);

            // Grab the pixel data from the PBO
            vg::GpuMemory::bindBuffer(m_pbos[i], vg::BufferTarget::PIXEL_PACK_BUFFER);
            void* src = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
            memcpy(m_heightData, src, sizeof(m_heightData));
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
            vg::GpuMemory::bindBuffer(0, vg::BufferTarget::PIXEL_PACK_BUFFER);
            
            // Bind texture for normal map gen
            glBindTexture(GL_TEXTURE_2D, m_textures[i].getTextureIDs().height_temp_hum);
 
            // Set uniforms
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

    m_patchCounter = 0;

    // Heightmap Generation
    m_genProgram->use();
    m_genProgram->enableVertexAttribArrays();

    glDisable(GL_DEPTH_TEST);
    m_rpcManager.processRequests(PATCHES_PER_FRAME);

    m_genProgram->disableVertexAttribArrays();
    m_genProgram->unuse();
    checkGlError("UPDATE");

    // Restore state
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

    // Bind PBO
    vg::GpuMemory::bindBuffer(m_pbos[m_patchCounter], vg::BufferTarget::PIXEL_PACK_BUFFER);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, PATCH_HEIGHTMAP_WIDTH, PATCH_HEIGHTMAP_WIDTH, GL_RGB, GL_FLOAT, 0);

    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::PIXEL_PACK_BUFFER);

    TerrainGenTextures::unuse();

    m_patchCounter++;
}

void SphericalTerrainGenerator::buildMesh(TerrainGenDelegate* data) {

    SphericalTerrainMesh* mesh = data->mesh;

    // Grab mappings so we can rotate the 2D grid appropriately
    m_coordMapping = data->coordMapping;
    m_startPos = data->startPos;
    m_ccw = CubeWindings[(int)mesh->m_cubeFace];
    
    float width = data->width;
    float h;
    f32v3 tmpPos;
    int xIndex;
    int zIndex;
    
    // Clear water index grid
    memset(waterIndexGrid, 0, sizeof(waterIndexGrid));
    memset(waterQuads, 0, sizeof(waterQuads));
    m_waterIndex = 0;
    m_waterIndexCount = 0;
  
    // Loop through and set all vertex attributes
    m_vertWidth = width / (PATCH_WIDTH - 1);
    m_index = 0;
    for (int z = 0; z < PATCH_WIDTH; z++) {
        for (int x = 0; x < PATCH_WIDTH; x++) {
            auto& v = verts[m_index];
            // Set the position based on which face we are on
            v.position[m_coordMapping.x] = x * m_vertWidth + m_startPos.x;
            v.position[m_coordMapping.y] = m_startPos.y;
            v.position[m_coordMapping.z] = z * m_vertWidth + m_startPos.z;

            // Get data from heightmap 
            zIndex = z * PIXELS_PER_PATCH_NM + 1;
            xIndex = x * PIXELS_PER_PATCH_NM + 1;
            h = m_heightData[zIndex][xIndex][0];
            v.temperature = (ui8)m_heightData[zIndex][xIndex][1];
            v.humidity = (ui8)m_heightData[zIndex][xIndex][2];

            // Water indexing
            if (h < 0) {
                addWater(z, x);
            }

            // Set texture coordinates
            v.texCoords.x = (ui8)(((float)x / (float)PATCH_WIDTH) * 255.0f);
            v.texCoords.y = (ui8)(((float)z / (float)PATCH_WIDTH) * 255.0f);

            // Spherify it!
            v.position = glm::normalize(v.position) * (m_radius + h);
          
            // Compute tangent
            tmpPos[m_coordMapping.x] = (x + 1) * m_vertWidth + m_startPos.x;
            tmpPos[m_coordMapping.y] = m_startPos.y;
            tmpPos[m_coordMapping.z] = z * m_vertWidth + m_startPos.z;
            tmpPos = glm::normalize(tmpPos) * (m_radius + h);
            v.tangent = glm::normalize(tmpPos - v.position);

            // Make sure tangent is orthogonal
            f32v3 binormal = glm::normalize(glm::cross(glm::normalize(v.position), v.tangent));
            v.tangent = glm::normalize(glm::cross(binormal, glm::normalize(v.position)));

            v.color = m_planetGenData->terrainTint;
            m_index++;
        }
    }

    // TMP: Approximate position with middle
    mesh->m_worldPosition = verts[m_index - PATCH_SIZE / 2].position;

    buildSkirts();
   
    // Generate the buffers and upload data
    vg::GpuMemory::createBuffer(mesh->m_vbo);
    vg::GpuMemory::bindBuffer(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER,
                                    VERTS_SIZE * sizeof(TerrainVertex),
                                    verts);
    // Reusable IBOs
    if (m_ccw) {
        mesh->m_ibo = m_ccwIbo;
    } else {
        mesh->m_ibo = m_cwIbo;
    }

    // Add water mesh
    if (m_waterIndexCount) {
        mesh->m_waterIndexCount = m_waterIndexCount;
        vg::GpuMemory::createBuffer(mesh->m_wvbo);
        vg::GpuMemory::bindBuffer(mesh->m_wvbo, vg::BufferTarget::ARRAY_BUFFER);
        vg::GpuMemory::uploadBufferData(mesh->m_wvbo, vg::BufferTarget::ARRAY_BUFFER,
                                        m_waterIndex * sizeof(WaterVertex),
                                        waterVerts);
        vg::GpuMemory::createBuffer(mesh->m_wibo);
        vg::GpuMemory::bindBuffer(mesh->m_wibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
        vg::GpuMemory::uploadBufferData(mesh->m_wibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER,
                                        m_waterIndexCount * sizeof(ui16),
                                        waterIndices);
    }

    // Finally, add to the mesh manager
    m_meshManager->addMesh(mesh);


    // TODO: Using a VAO makes it not work??
   //    glBindVertexArray(0);
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

void SphericalTerrainGenerator::addWater(int z, int x) {
    // Try add all adjacent vertices if needed
    tryAddWaterVertex(z - 1, x - 1);
    tryAddWaterVertex(z - 1, x);
    tryAddWaterVertex(z - 1, x + 1);
    tryAddWaterVertex(z, x - 1);
    tryAddWaterVertex(z, x);
    tryAddWaterVertex(z, x + 1);
    tryAddWaterVertex(z + 1, x - 1);
    tryAddWaterVertex(z + 1, x);
    tryAddWaterVertex(z + 1, x + 1);

    // Try add quads
    tryAddWaterQuad(z - 1, x - 1);
    tryAddWaterQuad(z - 1, x);
    tryAddWaterQuad(z, x - 1);
    tryAddWaterQuad(z, x);
}

void SphericalTerrainGenerator::tryAddWaterVertex(int z, int x) {
    // TEMPORARY? Add slight offset so we don't need skirts
    float mvw = m_vertWidth * 1.01;
    const float UV_SCALE = 0.01;

    if (z < 0 || x < 0 || z >= PATCH_WIDTH || x >= PATCH_WIDTH) return;
    if (waterIndexGrid[z][x] == 0) {
        waterIndexGrid[z][x] = m_waterIndex + 1;
        auto& v = waterVerts[m_waterIndex];
        // Set the position based on which face we are on
        v.position[m_coordMapping.x] = x * mvw + m_startPos.x;
        v.position[m_coordMapping.y] = m_startPos.y;
        v.position[m_coordMapping.z] = z * mvw + m_startPos.z;
   
        // Set texture coordinates
        v.texCoords.x = v.position[m_coordMapping.x] * UV_SCALE;
        v.texCoords.y = v.position[m_coordMapping.z] * UV_SCALE;

        // Spherify it!
        v.position = glm::normalize(v.position) * m_radius;

        float h = m_heightData[z * PIXELS_PER_PATCH_NM + 1][x * PIXELS_PER_PATCH_NM + 1][0];
        if (h < 0) {
            v.depth = -h;
        } else {
            v.depth = 0;
        }
        v.temperature = (ui8)m_heightData[z * PIXELS_PER_PATCH_NM + 1][x * PIXELS_PER_PATCH_NM + 1][1];

        // Compute tangent
        f32v3 tmpPos;
        tmpPos[m_coordMapping.x] = (x + 1) * mvw + m_startPos.x;
        tmpPos[m_coordMapping.y] = m_startPos.y;
        tmpPos[m_coordMapping.z] = z* mvw + m_startPos.z;
        tmpPos = glm::normalize(tmpPos) * m_radius;
        v.tangent = glm::normalize(tmpPos - v.position);

        // Make sure tangent is orthogonal
        f32v3 binormal = glm::normalize(glm::cross(glm::normalize(v.position), v.tangent));
        v.tangent = glm::normalize(glm::cross(binormal, glm::normalize(v.position)));

        v.color = m_planetGenData->liquidTint;
        m_waterIndex++;
    }
}

void SphericalTerrainGenerator::tryAddWaterQuad(int z, int x) {
    if (z < 0 || x < 0 || z >= PATCH_WIDTH - 1 || x >= PATCH_WIDTH - 1) return;
    if (!waterQuads[z][x]) {
        waterQuads[z][x] = true;

        if (m_ccw) {
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z][x] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z + 1][x] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z + 1][x + 1] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z + 1][x + 1] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z][x + 1] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z][x] - 1;
        } else {
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z][x] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z][x + 1] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z + 1][x + 1] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z + 1][x + 1] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z + 1][x] - 1;
            waterIndices[m_waterIndexCount++] = waterIndexGrid[z][x] - 1;
        }
    }
}

void SphericalTerrainGenerator::generateIndices(VGIndexBuffer& ibo, bool ccw) {
    // Loop through each quad and set indices
    int vertIndex;
    int index = 0;
    int skirtIndex = PATCH_SIZE;
    ui16 indices[SphericalTerrainPatch::INDICES_PER_PATCH];
    if (ccw) {
        // CCW
        // Main vertices
        for (int z = 0; z < PATCH_WIDTH - 1; z++) {
            for (int x = 0; x < PATCH_WIDTH - 1; x++) {
                // Compute index of back left vertex
                vertIndex = z * PATCH_WIDTH + x;
                // Change triangle orientation based on odd or even
                if ((x + z) % 2) {
                    indices[index++] = vertIndex;
                    indices[index++] = vertIndex + PATCH_WIDTH;
                    indices[index++] = vertIndex + PATCH_WIDTH + 1;
                    indices[index++] = vertIndex + PATCH_WIDTH + 1;
                    indices[index++] = vertIndex + 1;
                    indices[index++] = vertIndex;
                } else {
                    indices[index++] = vertIndex + 1;
                    indices[index++] = vertIndex;
                    indices[index++] = vertIndex + PATCH_WIDTH;
                    indices[index++] = vertIndex + PATCH_WIDTH;
                    indices[index++] = vertIndex + PATCH_WIDTH + 1;
                    indices[index++] = vertIndex + 1;
                }
            }
        }
        // Skirt vertices
        // Top Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i;
            indices[index++] = skirtIndex;
            indices[index++] = vertIndex;
            indices[index++] = vertIndex + 1;
            indices[index++] = vertIndex + 1;
            indices[index++] = skirtIndex + 1;
            indices[index++] = skirtIndex;
            skirtIndex++;
        }
        skirtIndex++; // Skip last vertex
        // Left Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i * PATCH_WIDTH;
            indices[index++] = skirtIndex;
            indices[index++] = skirtIndex + 1;
            indices[index++] = vertIndex + PATCH_WIDTH;
            indices[index++] = vertIndex + PATCH_WIDTH;
            indices[index++] = vertIndex;
            indices[index++] = skirtIndex;
            skirtIndex++;
        }
        skirtIndex++; // Skip last vertex
        // Right Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i * PATCH_WIDTH + PATCH_WIDTH - 1;
            indices[index++] = vertIndex;
            indices[index++] = vertIndex + PATCH_WIDTH;
            indices[index++] = skirtIndex + 1;
            indices[index++] = skirtIndex + 1;
            indices[index++] = skirtIndex;
            indices[index++] = vertIndex;
            skirtIndex++;
        }
        skirtIndex++;
        // Bottom Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = PATCH_SIZE - PATCH_WIDTH + i;
            indices[index++] = vertIndex;
            indices[index++] = skirtIndex;
            indices[index++] = skirtIndex + 1;
            indices[index++] = skirtIndex + 1;
            indices[index++] = vertIndex + 1;
            indices[index++] = vertIndex;
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
                    indices[index++] = vertIndex;
                    indices[index++] = vertIndex + 1;
                    indices[index++] = vertIndex + PATCH_WIDTH + 1;
                    indices[index++] = vertIndex + PATCH_WIDTH + 1;
                    indices[index++] = vertIndex + PATCH_WIDTH;
                    indices[index++] = vertIndex;
                } else {
                    indices[index++] = vertIndex + 1;
                    indices[index++] = vertIndex + PATCH_WIDTH + 1;
                    indices[index++] = vertIndex + PATCH_WIDTH;
                    indices[index++] = vertIndex + PATCH_WIDTH;
                    indices[index++] = vertIndex;
                    indices[index++] = vertIndex + 1;
                }
            }
        }
        // Skirt vertices
        // Top Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i;
            indices[index++] = skirtIndex;
            indices[index++] = skirtIndex + 1;
            indices[index++] = vertIndex + 1;
            indices[index++] = vertIndex + 1;
            indices[index++] = vertIndex;
            indices[index++] = skirtIndex;
            skirtIndex++;
        }
        skirtIndex++; // Skip last vertex
        // Left Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i * PATCH_WIDTH;
            indices[index++] = skirtIndex;
            indices[index++] = vertIndex;
            indices[index++] = vertIndex + PATCH_WIDTH;
            indices[index++] = vertIndex + PATCH_WIDTH;
            indices[index++] = skirtIndex + 1;
            indices[index++] = skirtIndex;
            skirtIndex++;
        }
        skirtIndex++; // Skip last vertex
        // Right Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = i * PATCH_WIDTH + PATCH_WIDTH - 1;
            indices[index++] = vertIndex;
            indices[index++] = skirtIndex;
            indices[index++] = skirtIndex + 1;
            indices[index++] = skirtIndex + 1;
            indices[index++] = vertIndex + PATCH_WIDTH;
            indices[index++] = vertIndex;
            skirtIndex++;
        }
        skirtIndex++;
        // Bottom Skirt
        for (int i = 0; i < PATCH_WIDTH - 1; i++) {
            vertIndex = PATCH_SIZE - PATCH_WIDTH + i;
            indices[index++] = vertIndex;
            indices[index++] = vertIndex + 1;
            indices[index++] = skirtIndex + 1;
            indices[index++] = skirtIndex + 1;
            indices[index++] = skirtIndex;
            indices[index++] = vertIndex;
            skirtIndex++;
        }
    }
    
    vg::GpuMemory::createBuffer(ibo);
    vg::GpuMemory::bindBuffer(ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER,
                                    SphericalTerrainPatch::INDICES_PER_PATCH * sizeof(ui16),
                                    indices);
}