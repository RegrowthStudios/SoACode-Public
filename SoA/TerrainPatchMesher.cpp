#include "stdafx.h"
#include "TerrainPatchMesher.h"

#include "VoxelSpaceConversions.h"
#include "SphericalTerrainComponentUpdater.h"
#include "SphericalHeightmapGenerator.h"
#include "TerrainPatchMeshManager.h"
#include "PlanetGenData.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/GraphicsDevice.h>
#include <Vorb/TextureRecycler.hpp>

/// Debug colors for rendering faces with unique color
const color3 DebugColors[6] {
      color3(255, 0, 0), //TOP
      color3(0, 255, 0), //LEFT
      color3(0, 0, 255), //RIGHT
      color3(255, 255, 0), //FRONT
      color3(0, 255, 255), //BACK
      color3(255, 0, 255) //BOTTOM
};

VGIndexBuffer TerrainPatchMesher::m_sharedIbo = 0; ///< Reusable CCW IBO

void TerrainPatchMesher::generateIndices() {
    // Loop through each quad and set indices
    int vertIndex;
    int index = 0;
    int skirtIndex = PATCH_SIZE;
    ui16 indices[PATCH_INDICES];

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

    vg::GpuMemory::createBuffer(m_sharedIbo);
    vg::GpuMemory::bindBuffer(m_sharedIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_sharedIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER,
                                    PATCH_INDICES * sizeof(ui16),
                                    indices);
}

void TerrainPatchMesher::destroyIndices() {
    vg::GpuMemory::freeBuffer(m_sharedIbo);
}

void TerrainPatchMesher::generateMeshData(TerrainPatchMesh* mesh, const PlanetGenData* planetGenData,
                                          const f32v3& startPos, WorldCubeFace cubeFace, float width,
                                          PlanetHeightData heightData[PADDED_PATCH_WIDTH][PADDED_PATCH_WIDTH],
                                          f64v3 positionData[PADDED_PATCH_WIDTH][PADDED_PATCH_WIDTH]) {

    assert(m_sharedIbo != 0);

    m_planetGenData = planetGenData;
    m_radius = (f32)m_planetGenData->radius;

    m_isSpherical = mesh->getIsSpherical();
    m_cubeFace = cubeFace;
    // Grab mappings so we can rotate the 2D grid appropriately
    if (m_isSpherical) {
        m_coordMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)m_cubeFace];
        m_startPos = startPos;
        m_coordMults = f32v2(VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)m_cubeFace]);
    } else {
        m_coordMapping = i32v3(0, 1, 2);
        m_startPos = f32v3(startPos.x, 0.0f, startPos.z);
        m_coordMults = f32v2(1.0f);
    }
    
    f32 h;
    // f32v3 tmpPos;
    f32 minX = FLT_MAX, maxX = -FLT_MAX;
    f32 minY = FLT_MAX, maxY = -FLT_MAX;
    f32 minZ = FLT_MAX, maxZ = -FLT_MAX;

    // Clear water index grid
    memset(waterIndexGrid, 0, sizeof(waterIndexGrid));
    memset(waterQuads, 0, sizeof(waterQuads));
    m_waterIndex = 0;
    m_waterIndexCount = 0;

    // Loop through and set all vertex attributes
    m_vertWidth = width / (PATCH_WIDTH - 1);
    m_index = 0;

    for (int z = 1; z < PADDED_PATCH_WIDTH - 1; z++) {
        for (int x = 1; x < PADDED_PATCH_WIDTH - 1; x++) {

            auto& v = verts[m_index];

            // Set the position based on which face we are on
            v.position = positionData[z][x];

            // Set color
            v.color = m_planetGenData->terrainTint;
            //v.color = DebugColors[(int)mesh->m_cubeFace]; // Uncomment for unique face colors

            // TODO(Ben): This is temporary edge debugging stuff
           /* const float delta = 100.0f;
            if (abs(v.position[m_coordMapping.x]) >= m_radius - delta
                || abs(v.position[m_coordMapping.z]) >= m_radius - delta) {
                v.color.r = 255;
                v.color.g = 0;
                v.color.b = 0;
            }*/

            // TODO(Ben): This is temporary biome debugging
            // v.color = heightData[z][x].biome->mapColor;

            // Get data from heightmap 
            h = heightData[z][x].height;

            // Water indexing
            if (h < 0) {
                addWater(z - 1, x - 1, heightData);
            }

            // TODO(Ben): Only update when not in frustum. Use double frustum method to start loading at frustum 2 and force in frustum 1
            v.temperature = heightData[z][x].temperature;
            v.humidity = heightData[z][x].humidity;

            // Check bounding box
            // TODO(Ben): Worry about water too!
            if (v.position.x < minX) minX = v.position.x;
            if (v.position.x > maxX) maxX = v.position.x;
            if (v.position.y < minY) minY = v.position.y;
            if (v.position.y > maxY) maxY = v.position.y;
            if (v.position.z < minZ) minZ = v.position.z;
            if (v.position.z > maxZ) maxZ = v.position.z;

            m_index++;
        }
    }

    f64v3 pl;
    f64v3 pr;
    f64v3 pb;
    f64v3 pf;

    // Second pass for normals TODO(Ben): Padding
    for (int z = 1; z < PADDED_PATCH_WIDTH - 1; z++) {
        for (int x = 1; x < PADDED_PATCH_WIDTH - 1; x++) {
            auto& v = verts[(z - 1) * PATCH_WIDTH + x - 1];
            f64v3& p = positionData[z][x];
            pl = positionData[z][x - 1] - p;
            pr = positionData[z][x + 1] - p;
            pb = positionData[z - 1][x] - p;
            pf = positionData[z + 1][x] - p;
            // Calculate smooth normal
            v.normal = glm::normalize(glm::cross(pb, pl) + glm::cross(pl, pf) +
                                      glm::cross(pf, pr) + glm::cross(pr, pb));
        }
    }

    // Get AABB
    mesh->m_aabbPos = f32v3(minX, minY, minZ);
    mesh->m_aabbDims = f32v3(maxX - minX, maxY - minY, maxZ - minZ);
    mesh->m_aabbCenter = mesh->m_aabbPos + mesh->m_aabbDims * 0.5f;
    // Calculate bounding sphere for culling
    mesh->m_boundingSphereRadius = glm::length(mesh->m_aabbCenter - mesh->m_aabbPos);
    // Build the skirts for crack hiding
    buildSkirts();

    // Make all vertices relative to the aabb pos for far terrain
    if (!m_isSpherical) {
        for (int i = 0; i < m_index; i++) {
            verts[i].position = f32v3(f64v3(verts[i].position) - f64v3(mesh->m_aabbPos));
        }
    }

    mesh->m_waterIndexCount = m_waterIndexCount;
    mesh->m_waterVertexCount = m_waterIndex;

    std::vector<ui8>& data = mesh->m_meshDataBuffer;

    data.resize(VERTS_SIZE * sizeof(TerrainVertex) +
                mesh->m_waterVertexCount * sizeof(WaterVertex) +
                mesh->m_waterIndexCount * sizeof(ui16));

    // Copy buffer data
    memcpy(data.data(), verts, VERTS_SIZE * sizeof(TerrainVertex));
    ui32 offset = VERTS_SIZE * sizeof(TerrainVertex);

    // Add water mesh
    if (m_waterIndexCount) {
        // Make all vertices relative to the aabb pos for far terrain
        if (!m_isSpherical) {
            for (int i = 0; i < m_index; i++) {
                waterVerts[i].position -= mesh->m_aabbPos;
            }
        }
        // Copy water data
        memcpy(data.data() + offset, waterVerts, mesh->m_waterVertexCount * sizeof(WaterVertex));
        offset += mesh->m_waterVertexCount * sizeof(WaterVertex);
        memcpy(data.data() + offset, waterIndices, mesh->m_waterIndexCount * sizeof(ui16));
    }
}

void TerrainPatchMesher::uploadMeshData(TerrainPatchMesh* mesh) {
    // Make VAO
    glGenVertexArrays(1, &mesh->m_vao);
    glBindVertexArray(mesh->m_vao);

    ui32 offset;

    // Generate the buffers and upload data
    vg::GpuMemory::createBuffer(mesh->m_vbo);
    vg::GpuMemory::bindBuffer(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER,
                                    VERTS_SIZE * sizeof(TerrainVertex),
                                    mesh->m_meshDataBuffer.data());
    offset = VERTS_SIZE * sizeof(TerrainVertex);

    // Reusable IBO
    vg::GpuMemory::bindBuffer(m_sharedIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);

    // Vertex attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, color));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, temperature));

    // Add water mesh
    if (mesh->m_waterIndexCount) {

        // Make VAO
        glGenVertexArrays(1, &mesh->m_wvao);
        glBindVertexArray(mesh->m_wvao);

        vg::GpuMemory::createBuffer(mesh->m_wvbo);
        vg::GpuMemory::bindBuffer(mesh->m_wvbo, vg::BufferTarget::ARRAY_BUFFER);
        vg::GpuMemory::uploadBufferData(mesh->m_wvbo, vg::BufferTarget::ARRAY_BUFFER,
                                        mesh->m_waterVertexCount * sizeof(WaterVertex),
                                        mesh->m_meshDataBuffer.data() + offset);
        offset += mesh->m_waterVertexCount * sizeof(WaterVertex);
        vg::GpuMemory::createBuffer(mesh->m_wibo);
        vg::GpuMemory::bindBuffer(mesh->m_wibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
        vg::GpuMemory::uploadBufferData(mesh->m_wibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER,
                                        mesh->m_waterIndexCount * sizeof(ui16),
                                        mesh->m_meshDataBuffer.data() + offset);
        // Vertex attribute pointers
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              sizeof(WaterVertex),
                              offsetptr(WaterVertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              sizeof(WaterVertex),
                              offsetptr(WaterVertex, tangent));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                              sizeof(WaterVertex),
                              offsetptr(WaterVertex, color));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE,
                              sizeof(WaterVertex),
                              offsetptr(WaterVertex, depth));
    }
    // Clear the data
    std::vector<ui8>().swap(mesh->m_meshDataBuffer);
    glBindVertexArray(0);
}

void TerrainPatchMesher::buildSkirts() {
    const float SKIRT_DEPTH = m_vertWidth * 3.0f;
    // Top Skirt
    for (int i = 0; i < PATCH_WIDTH; i++) {
        auto& v = verts[m_index];
        // Copy the vertices from the top edge
        v = verts[i];
        // Extrude downward
        if (m_isSpherical) {
            float len = glm::length(v.position) - SKIRT_DEPTH;
            v.position = glm::normalize(v.position) * len;
        } else {
            v.position.y -= SKIRT_DEPTH;
        }
        m_index++;
    }
    // Left Skirt
    for (int i = 0; i < PATCH_WIDTH; i++) {
        auto& v = verts[m_index];
        // Copy the vertices from the left edge
        v = verts[i * PATCH_WIDTH];
        // Extrude downward
        if (m_isSpherical) {
            float len = glm::length(v.position) - SKIRT_DEPTH;
            v.position = glm::normalize(v.position) * len;
        } else {
            v.position.y -= SKIRT_DEPTH;
        }
        m_index++;
    }
    // Right Skirt
    for (int i = 0; i < PATCH_WIDTH; i++) {
        auto& v = verts[m_index];
        // Copy the vertices from the right edge
        v = verts[i * PATCH_WIDTH + PATCH_WIDTH - 1];
        // Extrude downward
        if (m_isSpherical) {
            float len = glm::length(v.position) - SKIRT_DEPTH;
            v.position = glm::normalize(v.position) * len;
        } else {
            v.position.y -= SKIRT_DEPTH;
        }
        m_index++;
    }
    // Bottom Skirt
    for (int i = 0; i < PATCH_WIDTH; i++) {
        auto& v = verts[m_index];
        // Copy the vertices from the bottom edge
        v = verts[PATCH_SIZE - PATCH_WIDTH + i];
        // Extrude downward
        if (m_isSpherical) {
            float len = glm::length(v.position) - SKIRT_DEPTH;
            v.position = glm::normalize(v.position) * len;
        } else {
            v.position.y -= SKIRT_DEPTH;
        }
        m_index++;
    }
}

void TerrainPatchMesher::addWater(int z, int x, PlanetHeightData heightData[PADDED_PATCH_WIDTH][PADDED_PATCH_WIDTH]) {
    // Try add all adjacent vertices if needed
    tryAddWaterVertex(z - 1, x - 1, heightData);
    tryAddWaterVertex(z - 1, x, heightData);
    tryAddWaterVertex(z - 1, x + 1, heightData);
    tryAddWaterVertex(z, x - 1, heightData);
    tryAddWaterVertex(z, x, heightData);
    tryAddWaterVertex(z, x + 1, heightData);
    tryAddWaterVertex(z + 1, x - 1, heightData);
    tryAddWaterVertex(z + 1, x, heightData);
    tryAddWaterVertex(z + 1, x + 1, heightData);

    // Try add quads
    tryAddWaterQuad(z - 1, x - 1);
    tryAddWaterQuad(z - 1, x);
    tryAddWaterQuad(z, x - 1);
    tryAddWaterQuad(z, x);
}

void TerrainPatchMesher::tryAddWaterVertex(int z, int x, PlanetHeightData heightData[PADDED_PATCH_WIDTH][PADDED_PATCH_WIDTH]) {
    // TEMPORARY? Add slight offset so we don't need skirts
    f32 mvw = m_vertWidth * 1.005f;
    // const f32 UV_SCALE = 0.04f;

    if (z < 0 || x < 0 || z >= PATCH_WIDTH || x >= PATCH_WIDTH) return;
    if (waterIndexGrid[z][x] == 0) {
        waterIndexGrid[z][x] = m_waterIndex + 1;
        auto& v = waterVerts[m_waterIndex];
        // Set the position based on which face we are on
        v.position[m_coordMapping.x] = (x * mvw + m_startPos.x) * m_coordMults.x;
        v.position[m_coordMapping.y] = m_startPos.y;
        v.position[m_coordMapping.z] = (z * mvw + m_startPos.z) * m_coordMults.y;

        // Spherify it!
        // TODO(Ben): Use normal data
        f32v3 normal;
        if (m_isSpherical) {
            normal = glm::normalize(v.position);
            v.position = normal * m_radius;
        } else {
            const i32v3& trueMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)m_cubeFace];
            f32v3 tmpPos;
            tmpPos[trueMapping.x] = v.position.x;
            tmpPos[trueMapping.y] = m_radius * (f32)VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace];
            tmpPos[trueMapping.z] = v.position.z;
            normal = glm::normalize(tmpPos);
        }

        f32 d = heightData[z + 1][x + 1].height * (f32)M_PER_VOXEL;
        if (d < 0) {
            v.depth = -d;
        } else {
            v.depth = 0;
        }

        v.temperature = heightData[z + 1][x + 1].temperature;

        // Compute tangent
        f32v3 tmpPos;
        tmpPos[m_coordMapping.x] = ((x + 1) * mvw + m_startPos.x) * m_coordMults.x;
        tmpPos[m_coordMapping.y] = m_startPos.y;
        tmpPos[m_coordMapping.z] = (z * mvw + m_startPos.z) * m_coordMults.y;
        tmpPos = glm::normalize(tmpPos) * m_radius;
        v.tangent = glm::normalize(tmpPos - v.position);

        // Make sure tangent is orthogonal
        f32v3 binormal = glm::normalize(glm::cross(glm::normalize(v.position), v.tangent));
        v.tangent = glm::normalize(glm::cross(binormal, glm::normalize(v.position)));

        v.color = m_planetGenData->liquidTint;

        // TODO(Ben): This is temporary edge debugging stuff
        const float delta = 100.0f;
        if (abs(v.position[m_coordMapping.x]) >= m_radius - delta
            || abs(v.position[m_coordMapping.z]) >= m_radius - delta) {
            v.color.r = 255;
            v.color.g = 0;
            v.color.b = 0;
        }
        m_waterIndex++;
    }
}

void TerrainPatchMesher::tryAddWaterQuad(int z, int x) {
    if (z < 0 || x < 0 || z >= PATCH_WIDTH - 1 || x >= PATCH_WIDTH - 1) return;
    if (!waterQuads[z][x]) {
        waterQuads[z][x] = true;
        waterIndices[m_waterIndexCount++] = waterIndexGrid[z][x] - 1;
        waterIndices[m_waterIndexCount++] = waterIndexGrid[z + 1][x] - 1;
        waterIndices[m_waterIndexCount++] = waterIndexGrid[z + 1][x + 1] - 1;
        waterIndices[m_waterIndexCount++] = waterIndexGrid[z + 1][x + 1] - 1;
        waterIndices[m_waterIndexCount++] = waterIndexGrid[z][x + 1] - 1;
        waterIndices[m_waterIndexCount++] = waterIndexGrid[z][x] - 1;
    }
}
