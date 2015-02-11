#include "stdafx.h"
#include "SphericalTerrainPatchMesher.h"

#include "VoxelSpaceConversions.h"
#include "SphericalTerrainComponentUpdater.h"
#include "SphericalTerrainMeshManager.h"
#include "PlanetData.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/GraphicsDevice.h>
#include <Vorb/TextureRecycler.hpp>

#define KM_PER_M 0.001f
#define M_PER_KM 1000

/// Debug colors for rendering faces with unique color
const ColorRGB8 DebugColors[6] {
    ColorRGB8(255, 0, 0), //TOP
        ColorRGB8(0, 255, 0), //LEFT
        ColorRGB8(0, 0, 255), //RIGHT
        ColorRGB8(255, 255, 0), //FRONT
        ColorRGB8(0, 255, 255), //BACK
        ColorRGB8(255, 0, 255) //BOTTOM
};

TerrainVertex SphericalTerrainPatchMesher::verts[SphericalTerrainPatchMesher::VERTS_SIZE];
WaterVertex SphericalTerrainPatchMesher::waterVerts[SphericalTerrainPatchMesher::VERTS_SIZE];

ui16 SphericalTerrainPatchMesher::waterIndexGrid[PATCH_WIDTH][PATCH_WIDTH];
ui16 SphericalTerrainPatchMesher::waterIndices[SphericalTerrainPatch::INDICES_PER_PATCH];
bool SphericalTerrainPatchMesher::waterQuads[PATCH_WIDTH - 1][PATCH_WIDTH - 1];

VGIndexBuffer SphericalTerrainPatchMesher::m_sharedIbo = 0; ///< Reusable CCW IBO


SphericalTerrainPatchMesher::SphericalTerrainPatchMesher(SphericalTerrainMeshManager* meshManager,
                                       PlanetGenData* planetGenData) :
    m_meshManager(meshManager),
    m_planetGenData(planetGenData) {
    // Construct reusable index buffer object
    if (m_sharedIbo == 0) {
        generateIndices(m_sharedIbo);
    }

    m_radius = m_planetGenData->radius;
}

SphericalTerrainPatchMesher::~SphericalTerrainPatchMesher() {
    vg::GpuMemory::freeBuffer(m_sharedIbo);
}

void SphericalTerrainPatchMesher::buildMesh(TerrainGenDelegate* data, float heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4]) {

    SphericalTerrainMesh* mesh = data->mesh;

    // Grab mappings so we can rotate the 2D grid appropriately
    m_coordMapping = VoxelSpaceConversions::GRID_TO_WORLD[(int)data->cubeFace];
    m_startPos = data->startPos;
    m_coordMults = f32v2(VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)data->cubeFace][0]);

    float width = data->width;
    float h;
    float angle;
    f32v3 tmpPos;
    int xIndex;
    int zIndex;
    float minX = INT_MAX, maxX = INT_MIN;
    float minY = INT_MAX, maxY = INT_MIN;
    float minZ = INT_MAX, maxZ = INT_MIN;

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
            v.position[m_coordMapping.x] = x * m_vertWidth * m_coordMults.x + m_startPos.x;
            v.position[m_coordMapping.y] = m_startPos.y;
            v.position[m_coordMapping.z] = z * m_vertWidth * m_coordMults.y + m_startPos.z;

            // Get data from heightmap 
            zIndex = z * PIXELS_PER_PATCH_NM + 1;
            xIndex = x * PIXELS_PER_PATCH_NM + 1;
            h = heightData[zIndex][xIndex][0] * KM_PER_M;

            // Water indexing
            if (h < 0) {
                addWater(z, x, heightData);
            }

            // Set texture coordinates using grid position
            v.texCoords.x = v.position[m_coordMapping.x];
            v.texCoords.y = v.position[m_coordMapping.z];

            // Set normal map texture coordinates
            v.normTexCoords.x = (ui8)(((float)x / (float)PATCH_WIDTH) * 255.0f);
            v.normTexCoords.y = (ui8)(((float)z / (float)PATCH_WIDTH) * 255.0f);

            // Spherify it!
            f32v3 normal = glm::normalize(v.position);
            v.position = normal * (m_radius + h);

            angle = computeAngleFromNormal(normal);

            v.temperature = calculateTemperature(m_planetGenData->tempLatitudeFalloff, angle, heightData[zIndex][xIndex][1]);
            v.humidity = calculateHumidity(m_planetGenData->humLatitudeFalloff, angle, heightData[zIndex][xIndex][2]);

            // Compute tangent
            tmpPos[m_coordMapping.x] = (x + 1) * m_vertWidth * m_coordMults.x + m_startPos.x;
            tmpPos[m_coordMapping.y] = m_startPos.y;
            tmpPos[m_coordMapping.z] = z * m_vertWidth * m_coordMults.y + m_startPos.z;
            tmpPos = glm::normalize(tmpPos) * (m_radius + h);
            v.tangent = glm::normalize(tmpPos - v.position);

            // Make sure tangent is orthogonal
            f32v3 binormal = glm::normalize(glm::cross(glm::normalize(v.position), v.tangent));
            v.tangent = glm::normalize(glm::cross(binormal, glm::normalize(v.position)));

            // Check bounding box
            if (v.position.x < minX) {
                minX = v.position.x;
            } else if (v.position.x > maxX) {
                maxX = v.position.x;
            }
            if (v.position.y < minY) {
                minY = v.position.y;
            } else if (v.position.y > maxY) {
                maxY = v.position.y;
            }
            if (v.position.z < minZ) {
                minZ = v.position.z;
            } else if (v.position.z > maxZ) {
                maxZ = v.position.z;
            }

            v.color = m_planetGenData->terrainTint;
            // v.color = DebugColors[(int)mesh->m_cubeFace];

            m_index++;
        }
    }

    // Get world position and bounding box
    mesh->m_worldPosition = f32v3(minX, minY, minZ);
    mesh->m_boundingBox = f32v3(maxX - minX, maxY - minY, maxZ - minZ);

    buildSkirts();

    // Generate the buffers and upload data
    vg::GpuMemory::createBuffer(mesh->m_vbo);
    vg::GpuMemory::bindBuffer(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER,
                                    VERTS_SIZE * sizeof(TerrainVertex),
                                    verts);
    // Reusable IBO
    mesh->m_ibo = m_sharedIbo;

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

// Thanks to tetryds for these
ui8 SphericalTerrainPatchMesher::calculateTemperature(float range, float angle, float baseTemp) {
    float tempFalloff = 1.0f - pow(cos(angle), 2.0f * angle);
    float temp = baseTemp - tempFalloff * range;
    return (ui8)(glm::clamp(temp, 0.0f, 255.0f));
}

// Thanks to tetryds for these
ui8 SphericalTerrainPatchMesher::calculateHumidity(float range, float angle, float baseHum) {
    float cos3x = cos(3.0f * angle);
    float humFalloff = 1.0f - (-0.25f * angle + 1.0f) * (cos3x * cos3x);
    float hum = baseHum - humFalloff * range;
    return (ui8)(glm::clamp(hum, 0.0f, 255.0f));
}

void SphericalTerrainPatchMesher::buildSkirts() {
    const float SKIRT_DEPTH = m_vertWidth * 3.0f;
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

void SphericalTerrainPatchMesher::addWater(int z, int x, float heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4]) {
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

void SphericalTerrainPatchMesher::tryAddWaterVertex(int z, int x, float heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4]) {
    // TEMPORARY? Add slight offset so we don't need skirts
    float mvw = m_vertWidth * 1.005;
    const float UV_SCALE = 0.04;
    int xIndex;
    int zIndex;

    if (z < 0 || x < 0 || z >= PATCH_WIDTH || x >= PATCH_WIDTH) return;
    if (waterIndexGrid[z][x] == 0) {
        waterIndexGrid[z][x] = m_waterIndex + 1;
        auto& v = waterVerts[m_waterIndex];
        // Set the position based on which face we are on
        v.position[m_coordMapping.x] = x * mvw * m_coordMults.x + m_startPos.x;
        v.position[m_coordMapping.y] = m_startPos.y;
        v.position[m_coordMapping.z] = z * mvw * m_coordMults.y + m_startPos.z;

        // Set texture coordinates
        v.texCoords.x = v.position[m_coordMapping.x] * UV_SCALE;
        v.texCoords.y = v.position[m_coordMapping.z] * UV_SCALE;

        // Spherify it!
        f32v3 normal = glm::normalize(v.position);
        v.position = normal * m_radius;

        zIndex = z * PIXELS_PER_PATCH_NM + 1;
        xIndex = x * PIXELS_PER_PATCH_NM + 1;
        float d = heightData[zIndex][xIndex][0] * KM_PER_M;
        if (d < 0) {
            v.depth = -d;
        } else {
            v.depth = 0;
        }

        v.temperature = calculateTemperature(m_planetGenData->tempLatitudeFalloff, computeAngleFromNormal(normal), heightData[zIndex][xIndex][1]);

        // Compute tangent
        f32v3 tmpPos;
        tmpPos[m_coordMapping.x] = (x + 1) * mvw * m_coordMults.x + m_startPos.x;
        tmpPos[m_coordMapping.y] = m_startPos.y;
        tmpPos[m_coordMapping.z] = z * mvw * m_coordMults.y + m_startPos.z;
        tmpPos = glm::normalize(tmpPos) * m_radius;
        v.tangent = glm::normalize(tmpPos - v.position);

        // Make sure tangent is orthogonal
        f32v3 binormal = glm::normalize(glm::cross(glm::normalize(v.position), v.tangent));
        v.tangent = glm::normalize(glm::cross(binormal, glm::normalize(v.position)));

        v.color = m_planetGenData->liquidTint;
        m_waterIndex++;
    }
}

void SphericalTerrainPatchMesher::tryAddWaterQuad(int z, int x) {
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

void SphericalTerrainPatchMesher::generateIndices(OUT VGIndexBuffer& ibo) {
    // Loop through each quad and set indices
    int vertIndex;
    int index = 0;
    int skirtIndex = PATCH_SIZE;
    ui16 indices[SphericalTerrainPatch::INDICES_PER_PATCH];
    
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

    vg::GpuMemory::createBuffer(ibo);
    vg::GpuMemory::bindBuffer(ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER,
                                    SphericalTerrainPatch::INDICES_PER_PATCH * sizeof(ui16),
                                    indices);
}

float SphericalTerrainPatchMesher::computeAngleFromNormal(const f32v3& normal) {
    // Compute angle
    if (normal.y == 1.0f || normal.y == -1.0f) {
        return M_PI / 2.0;
    } else if (abs(normal.y) < 0.001) {
        // Need to do this to fix an equator bug
        return 0.0f;
    } else {
        f32v3 equator = glm::normalize(f32v3(normal.x, 0.0f, normal.z));
        return acos(glm::dot(equator, normal));
    }
}