#include "stdafx.h"
#include "SphericalTerrainGenerator.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/GraphicsDevice.h>
#include <Vorb/TextureRecycler.hpp>
#include <Vorb/Timing.h>

#include "Chunk.h"
#include "Errors.h"
#include "PlanetLoader.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainComponentUpdater.h"
#include "SphericalTerrainMeshManager.h"

#define M_PER_KM 1000.0f
#define KM_PER_M 0.001f
#define KM_PER_VOXEL 0.0005f
#define VOXELS_PER_M 2.0f

const ColorRGB8 DebugColors[12] {
    ColorRGB8(255, 0, 0), //TOP
    ColorRGB8(0, 255, 0), //LEFT
    ColorRGB8(0, 0, 255), //RIGHT
    ColorRGB8(255, 255, 0), //FRONT
    ColorRGB8(0, 255, 255), //BACK
    ColorRGB8(255, 0, 255), //BOTTOM
    ColorRGB8(255, 33, 55), //?
    ColorRGB8(125, 125, 125), //?
    ColorRGB8(255, 125, 125), //?
    ColorRGB8(125, 255, 255), //?
    ColorRGB8(125, 255, 125), //?
    ColorRGB8(255, 125, 255) //?
};

TerrainVertex SphericalTerrainGenerator::verts[SphericalTerrainGenerator::VERTS_SIZE];
WaterVertex SphericalTerrainGenerator::waterVerts[SphericalTerrainGenerator::VERTS_SIZE];
float SphericalTerrainGenerator::m_heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4];

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

    // Zero counters
    m_patchCounter[0] = 0;
    m_patchCounter[1] = 0;
    m_rawCounter[0] = 0;
    m_rawCounter[1] = 0;

    m_heightMapDims = ui32v2(PATCH_HEIGHTMAP_WIDTH);
    ui32v2 chunkDims = ui32v2(CHUNK_WIDTH);
    for (int i = 0; i < PATCHES_PER_FRAME; i++) {
        m_patchTextures[0][i].init(m_heightMapDims);
        m_patchTextures[1][i].init(m_heightMapDims);
    }
    for (int i = 0; i < RAW_PER_FRAME; i++) {
        m_rawTextures[0][i].init(chunkDims);
        m_rawTextures[1][i].init(chunkDims);
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
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < PATCHES_PER_FRAME; j++) {
            vg::GpuMemory::createBuffer(m_patchPbos[i][j]);
            vg::GpuMemory::bindBuffer(m_patchPbos[i][j], vg::BufferTarget::PIXEL_PACK_BUFFER);
            glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(m_heightData), NULL, GL_STREAM_READ);
        }
    }
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < RAW_PER_FRAME; j++) {
            vg::GpuMemory::createBuffer(m_rawPbos[i][j]);
            vg::GpuMemory::bindBuffer(m_rawPbos[i][j], vg::BufferTarget::PIXEL_PACK_BUFFER);
            glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(float) * 4 * CHUNK_LAYER, NULL, GL_STREAM_READ);
        }
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

SphericalTerrainGenerator::~SphericalTerrainGenerator() {
    for (int i = 0; i < PATCHES_PER_FRAME; i++) {
        vg::GpuMemory::freeBuffer(m_patchPbos[0][i]);
        vg::GpuMemory::freeBuffer(m_patchPbos[1][i]);
    }
    for (int i = 0; i < RAW_PER_FRAME; i++) {
        vg::GpuMemory::freeBuffer(m_rawPbos[0][i]);
        vg::GpuMemory::freeBuffer(m_rawPbos[1][i]);
    }
    vg::GpuMemory::freeBuffer(m_cwIbo);
    vg::GpuMemory::freeBuffer(m_ccwIbo);
    glDeleteFramebuffers(1, &m_normalFbo);
}

void SphericalTerrainGenerator::update() {

    // Need to disable alpha blending
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

    if (m_rawCounter[m_dBufferIndex]) {
        updateRawGeneration();
    }
    if (m_patchCounter[m_dBufferIndex]) {
        updatePatchGeneration();
    }
    
    // Heightmap Generation
    m_genProgram->use();
    m_genProgram->enableVertexAttribArrays();

    if (m_planetGenData->baseBiomeLookupTexture) {
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(m_genProgram->getUniform("unBaseBiomes"), 0);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->baseBiomeLookupTexture);
        nString glVendor = GraphicsDevice::getCurrent()->getProperties().glVendor;
        if (glVendor.find("Intel") != nString::npos) {
            glActiveTexture(GL_TEXTURE1);
            glUniform1i(m_genProgram->getUniform("unBiomes"), 1);
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_planetGenData->biomeArrayTexture);
        }
    }

    glDisable(GL_DEPTH_TEST);
    m_rawRpcManager.processRequests(RAW_PER_FRAME);
    m_patchRpcManager.processRequests(PATCHES_PER_FRAME);

    m_genProgram->disableVertexAttribArrays();
    m_genProgram->unuse();
    checkGlError("UPDATE");

    // Restore state
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
  
    // Release pixel pack buffer
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // Change double buffer index
    m_dBufferIndex = (m_dBufferIndex == 0) ? 1 : 0;
}

void SphericalTerrainGenerator::generateTerrain(TerrainGenDelegate* data) {
  
    int &patchCounter = m_patchCounter[m_dBufferIndex];

    // Check for early delete
    if (data->mesh->m_shouldDelete) {
        delete data->mesh;
        data->inUse = false;
        return;
    }

    m_patchTextures[m_dBufferIndex][patchCounter].use();
    m_patchDelegates[m_dBufferIndex][patchCounter] = data;

    // Get padded position
    f32v3 cornerPos = data->startPos;
    cornerPos[data->coordMapping.x] -= (1.0f / PATCH_HEIGHTMAP_WIDTH) * data->width;
    cornerPos[data->coordMapping.z] -= (1.0f / PATCH_HEIGHTMAP_WIDTH) * data->width;

    // Send uniforms
    glUniform3fv(unCornerPos, 1, &cornerPos[0]);
    glUniform3iv(unCoordMapping, 1, &data->coordMapping[0]);
    glUniform1f(unPatchWidth, data->width);

    m_quad.draw();

    // Bind PBO
    vg::GpuMemory::bindBuffer(m_patchPbos[m_dBufferIndex][patchCounter], vg::BufferTarget::PIXEL_PACK_BUFFER);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, PATCH_HEIGHTMAP_WIDTH, PATCH_HEIGHTMAP_WIDTH, GL_RGBA, GL_FLOAT, 0);

    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::PIXEL_PACK_BUFFER);

    TerrainGenTextures::unuse();

    patchCounter++;
}

void SphericalTerrainGenerator::generateRawHeightmap(RawGenDelegate* data) {

    int &rawCounter = m_rawCounter[m_dBufferIndex];

    m_rawTextures[m_dBufferIndex][rawCounter].use();
    m_rawDelegates[m_dBufferIndex][rawCounter] = data;

    // Get padded position
    f32v3 cornerPos = data->startPos;

    // Send uniforms
    glUniform3fv(unCornerPos, 1, &cornerPos[0]);
    glUniform3iv(unCoordMapping, 1, &data->coordMapping[0]);

    glUniform1f(unPatchWidth, (data->width * data->step));
    m_quad.draw();

    // Bind PBO
    vg::GpuMemory::bindBuffer(m_rawPbos[m_dBufferIndex][rawCounter], vg::BufferTarget::PIXEL_PACK_BUFFER);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, data->width, data->width, GL_RGBA, GL_FLOAT, 0);

    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::PIXEL_PACK_BUFFER);

    TerrainGenTextures::unuse();

    rawCounter++;
}

void SphericalTerrainGenerator::buildMesh(TerrainGenDelegate* data) {

    SphericalTerrainMesh* mesh = data->mesh;

    // Grab mappings so we can rotate the 2D grid appropriately
    m_coordMapping = data->coordMapping;
    m_startPos = data->startPos;
    m_ccw = CubeWindings[(int)mesh->m_cubeFace];
    
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
            v.position[m_coordMapping.x] = x * m_vertWidth + m_startPos.x;
            v.position[m_coordMapping.y] = m_startPos.y;
            v.position[m_coordMapping.z] = z * m_vertWidth + m_startPos.z;

            // Get data from heightmap 
            zIndex = z * PIXELS_PER_PATCH_NM + 1;
            xIndex = x * PIXELS_PER_PATCH_NM + 1;
            h = m_heightData[zIndex][xIndex][0] * KM_PER_M;
            
            // Water indexing
            if (h < 0) {
                addWater(z, x);
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
            
            v.temperature = calculateTemperature(m_planetGenData->tempLatitudeFalloff, angle, m_heightData[zIndex][xIndex][1]);
            v.humidity = calculateHumidity(m_planetGenData->humLatitudeFalloff, angle, m_heightData[zIndex][xIndex][2]);

            // Compute tangent
            tmpPos[m_coordMapping.x] = (x + 1) * m_vertWidth + m_startPos.x;
            tmpPos[m_coordMapping.y] = m_startPos.y;
            tmpPos[m_coordMapping.z] = z * m_vertWidth + m_startPos.z;
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
         //   v.color = DebugColors[(int)mesh->m_cubeFace];

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

void SphericalTerrainGenerator::updatePatchGeneration() {
    // Normal map generation
    m_normalProgram->enableVertexAttribArrays();
    m_normalProgram->use();

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(unHeightMap, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_normalFbo);
    glViewport(0, 0, PATCH_NORMALMAP_WIDTH, PATCH_NORMALMAP_WIDTH);

    // Loop through all textures
    for (int i = 0; i < m_patchCounter[m_dBufferIndex]; i++) {

        TerrainGenDelegate* data = m_patchDelegates[m_dBufferIndex][i];

        // Check for early delete
        if (data->mesh->m_shouldDelete) {
            delete data->mesh;
            data->inUse = false;
            continue;
        }

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
        vg::GpuMemory::bindBuffer(m_patchPbos[m_dBufferIndex][i], vg::BufferTarget::PIXEL_PACK_BUFFER);
        void* src = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        memcpy(m_heightData, src, sizeof(m_heightData));
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        vg::GpuMemory::bindBuffer(0, vg::BufferTarget::PIXEL_PACK_BUFFER);

        // Bind texture for normal map gen
        glBindTexture(GL_TEXTURE_2D, m_patchTextures[m_dBufferIndex][i].getTextureIDs().height_temp_hum);

        // Set uniforms
        glUniform1f(unWidth, (data->width / PATCH_HEIGHTMAP_WIDTH) * M_PER_KM);
        glUniform1f(unTexelWidth, (float)PATCH_HEIGHTMAP_WIDTH);

        // Generate normal map
        m_quad.draw();

        // And finally build the mesh
        buildMesh(data);

        data->inUse = false;
    }
    m_patchCounter[m_dBufferIndex] = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_normalProgram->disableVertexAttribArrays();
    m_normalProgram->unuse();
}

void SphericalTerrainGenerator::updateRawGeneration() {

    float heightData[CHUNK_WIDTH][CHUNK_WIDTH][4];

    // Loop through all textures
    for (int i = 0; i < m_rawCounter[m_dBufferIndex]; i++) {

        RawGenDelegate* data = m_rawDelegates[m_dBufferIndex][i];

        // Grab the pixel data from the PBO
        vg::GpuMemory::bindBuffer(m_rawPbos[m_dBufferIndex][i], vg::BufferTarget::PIXEL_PACK_BUFFER);
        void* src = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        memcpy(heightData, src, sizeof(heightData));
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        vg::GpuMemory::bindBuffer(0, vg::BufferTarget::PIXEL_PACK_BUFFER);
        
        // Set the height data using the src
        int c = 0;
        for (int y = 0; y < CHUNK_WIDTH; y++) {
            for (int x = 0; x < CHUNK_WIDTH; x++, c++) {
                data->gridData->heightData[c].height = heightData[y][x][0] * VOXELS_PER_M;
                data->gridData->heightData[c].temperature = heightData[y][x][1];
                data->gridData->heightData[c].rainfall = heightData[y][x][2];
                //TODO(Ben): Biomes
                data->gridData->heightData[c].biome = nullptr;
                data->gridData->heightData[c].surfaceBlock = STONE;
                data->gridData->heightData[c].depth = 0;
                data->gridData->heightData[c].sandDepth = 0; // TODO(Ben): kill this
                data->gridData->heightData[c].snowDepth = 0;
                data->gridData->heightData[c].flags = 0;
            }
        }

        data->gridData->isLoaded = true;
        data->gridData->refCount--; //TODO(Ben): This will result in a memory leak since when it hits 0, it wont deallocate
        data->inUse = false;
    }
    m_rawCounter[m_dBufferIndex] = 0;
}

// Thanks to tetryds for these
ui8 SphericalTerrainGenerator::calculateTemperature(float range, float angle, float baseTemp) {
    float tempFalloff = 1.0f - pow(cos(angle), 2.0f * angle);
    float temp = baseTemp - tempFalloff * range;
    return (ui8)(glm::clamp(temp, 0.0f, 255.0f));
}

// Thanks to tetryds for these
ui8 SphericalTerrainGenerator::calculateHumidity(float range, float angle, float baseHum) {
    float cos3x = cos(3.0f * angle);
    float humFalloff = 1.0f - (-0.25f * angle + 1.0f) * (cos3x * cos3x);
    float hum = baseHum - humFalloff * range;
    return (ui8)(glm::clamp(hum, 0.0f, 255.0f));
}

float SphericalTerrainGenerator::computeAngleFromNormal(const f32v3& normal) {
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

void SphericalTerrainGenerator::buildSkirts() {
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
    float mvw = m_vertWidth * 1.005;
    const float UV_SCALE = 0.04;
    int xIndex;
    int zIndex;

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
        f32v3 normal = glm::normalize(v.position);
        v.position = normal * m_radius;

        zIndex = z * PIXELS_PER_PATCH_NM + 1;
        xIndex = x * PIXELS_PER_PATCH_NM + 1;
        float d = m_heightData[zIndex][xIndex][0];
        if (d < 0) {
            v.depth = -d;
        } else {
            v.depth = 0;
        }

        v.temperature = calculateTemperature(m_planetGenData->tempLatitudeFalloff, computeAngleFromNormal(normal), m_heightData[zIndex][xIndex][1]);

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