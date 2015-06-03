#include "stdafx.h"
#include "SphericalTerrainGpuGenerator.h"

#include <Vorb/Timing.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/TextureRecycler.hpp>
#include <Vorb/graphics/GraphicsDevice.h>

#include "Chunk.h"
#include "Errors.h"
#include "PlanetData.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainComponentUpdater.h"
#include "TerrainPatchMeshManager.h"
#include "soaUtils.h"

void RawHeightGenerator::invoke(Sender s, void* data) {
    generator->generateRawHeightmap(this);
}

void RawHeightGenerator::release() {
    inUse = false;
    gridData.reset();
}

float SphericalTerrainGpuGenerator::m_heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4];

HeightmapGenRpcDispatcher::HeightmapGenRpcDispatcher(SphericalTerrainGpuGenerator* generator) :
    m_generator(generator) {
    // Empty    
}

HeightmapGenRpcDispatcher::~HeightmapGenRpcDispatcher() {
    delete[] m_generators;
}

bool HeightmapGenRpcDispatcher::dispatchHeightmapGen(std::shared_ptr<ChunkGridData>& cgd, const ChunkPosition3D& facePosition, float planetRadius) {
    // Lazy init
    if (!m_generators) {
        m_generators = new RawHeightGenerator[NUM_GENERATORS];
        for (int i = 0; i < NUM_GENERATORS; i++) {
            m_generators[i].generator = m_generator;
        }
    }
    
    // Check if there is a free generator
    if (!m_generators[counter].inUse) {
        auto& gen = m_generators[counter];
        // Mark the generator as in use
        gen.inUse = true;
        cgd->wasRequestSent = true;
        gen.gridData = cgd;

        // Set the data
        gen.startPos = f32v3(facePosition.pos.x * CHUNK_WIDTH * KM_PER_VOXEL,
                             planetRadius * KM_PER_VOXEL,
                             facePosition.pos.z * CHUNK_WIDTH * KM_PER_VOXEL);

        gen.cubeFace = facePosition.face;

        gen.width = 32;
        gen.step = KM_PER_VOXEL;
        // Invoke generator
        m_generator->invokeRawGen(&gen.rpc);
        // Go to next generator
        counter++;
        if (counter == NUM_GENERATORS) counter = 0;
        return true;
    }
    return false;
}

SphericalTerrainGpuGenerator::SphericalTerrainGpuGenerator(TerrainPatchMeshManager* meshManager,
                                                     PlanetGenData* planetGenData,
                                                     vg::GLProgram* normalProgram,
                                                     vg::TextureRecycler* normalMapRecycler) :
    m_meshManager(meshManager),
    m_planetGenData(planetGenData),
    m_genProgram(planetGenData->program),
    m_normalProgram(normalProgram),
    m_normalMapRecycler(normalMapRecycler),
    unCornerPos(m_genProgram.getUniform("unCornerPos")),
    unCoordMults(m_genProgram.getUniform("unCoordMults")),
    unCoordMapping(m_genProgram.getUniform("unCoordMapping")),
    unPatchWidth(m_genProgram.getUniform("unPatchWidth")),
    unRadius(m_genProgram.getUniform("unRadius")),
    unHeightMap(m_normalProgram->getUniform("unHeightMap")),
    unWidth(m_normalProgram->getUniform("unWidth")),
    heightmapGenRpcDispatcher(this) {
    // Empty
}

SphericalTerrainGpuGenerator::~SphericalTerrainGpuGenerator() {
    for (int i = 0; i < PATCHES_PER_FRAME; i++) {
        vg::GpuMemory::freeBuffer(m_patchPbos[0][i]);
        vg::GpuMemory::freeBuffer(m_patchPbos[1][i]);
    }
    for (int i = 0; i < RAW_PER_FRAME; i++) {
        vg::GpuMemory::freeBuffer(m_rawPbos[0][i]);
        vg::GpuMemory::freeBuffer(m_rawPbos[1][i]);
    }
    glDeleteFramebuffers(1, &m_normalFbo);
    m_genProgram.dispose();
}

void SphericalTerrainGpuGenerator::update() {

    // Lazily initialize
    if (!m_mesher) {
        init();
    }

    // Need to disable alpha blending
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

    if (m_rawCounter[m_dBufferIndex]) {
        updateRawGeneration();
    }
    if (m_patchCounter[m_dBufferIndex]) {
        updatePatchGeneration();
    }
    
    // Heightmap Generation
    m_genProgram.use();
    m_genProgram.enableVertexAttribArrays();

    if (m_planetGenData->baseBiomeLookupTexture) {
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(m_genProgram.getUniform("unBaseBiomes"), 0);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->baseBiomeLookupTexture);
        nString glVendor = vg::GraphicsDevice::getCurrent()->getProperties().glVendor;
        if (glVendor.find("Intel") != nString::npos) {
            glActiveTexture(GL_TEXTURE1);
            glUniform1i(m_genProgram.getUniform("unBiomes"), 1);
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_planetGenData->biomeArrayTexture);
        }
    }

    glDisable(GL_DEPTH_TEST);
    m_rawRpcManager.processRequests(RAW_PER_FRAME);
    m_patchRpcManager.processRequests(PATCHES_PER_FRAME);
    TerrainGenTextures::unuse();

    m_genProgram.disableVertexAttribArrays();
    m_genProgram.unuse();

    // Restore state
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
  
    // Release pixel pack buffer
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // Change double buffer index
    m_dBufferIndex = (m_dBufferIndex == 0) ? 1 : 0;
}

void SphericalTerrainGpuGenerator::generateTerrainPatch(TerrainGenDelegate* data) {
  
    // TODO(Ben): Precision begins to be lost at 8,388,608
    // Aldrin has coordinates that go up to 4,600,000

    int &patchCounter = m_patchCounter[m_dBufferIndex];

    // Check for early delete
    if (data->mesh->m_shouldDelete) {
        delete data->mesh;
        data->release();
        return;
    }

    m_patchTextures[m_dBufferIndex][patchCounter].use();
    m_patchDelegates[m_dBufferIndex][patchCounter] = data;

    f32v3 cornerPos = data->startPos;
    f32 texelSize = 1.0f / (TEXELS_PER_PATCH);
    // Get padded position for heightmap (2 texel border)
    cornerPos.x -= 2.0f * texelSize * data->width;
    cornerPos.z -= 2.0f * texelSize * data->width;

    const i32v3& coordMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)data->cubeFace];
    const f32v2 coordMults = f32v2(VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)data->cubeFace]);

    // Map to world space
    cornerPos.x *= coordMults.x;
    cornerPos.y *= (f32)VoxelSpaceConversions::FACE_Y_MULTS[(int)data->cubeFace];
    cornerPos.z *= coordMults.y;

    // Send uniforms
    glUniform3fv(unCornerPos, 1, &cornerPos[0]);
    glUniform2fv(unCoordMults, 1, &coordMults[0]);
    glUniform3iv(unCoordMapping, 1, &coordMapping[0]);
    glUniform1f(unPatchWidth, data->width + (texelSize * 4.0f) * data->width);
    glUniform1f(unRadius, m_planetGenData->radius);

    m_quad.draw();

    // Bind PBO
    vg::GpuMemory::bindBuffer(m_patchPbos[m_dBufferIndex][patchCounter], vg::BufferTarget::PIXEL_PACK_BUFFER);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, PATCH_HEIGHTMAP_WIDTH, PATCH_HEIGHTMAP_WIDTH, GL_RGBA, GL_FLOAT, 0);

    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::PIXEL_PACK_BUFFER);

    patchCounter++;
}

void SphericalTerrainGpuGenerator::generateRawHeightmap(RawHeightGenerator* data) {

    int &rawCounter = m_rawCounter[m_dBufferIndex];

    m_rawTextures[m_dBufferIndex][rawCounter].use();
    m_rawDelegates[m_dBufferIndex][rawCounter] = data;

    f32v3 cornerPos = data->startPos;
    const f32v2 coordMults = f32v2(VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)data->cubeFace]);

    // Map to world space
    cornerPos.x *= coordMults.x;
    cornerPos.y *= (f32)VoxelSpaceConversions::FACE_Y_MULTS[(int)data->cubeFace];
    cornerPos.z *= coordMults.y;

    // Send uniforms
    glUniform3fv(unCornerPos, 1, &cornerPos[0]);
    glUniform2fv(unCoordMults, 1, &coordMults[0]);
    i32v3 coordMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)data->cubeFace];
    glUniform3iv(unCoordMapping, 1, &coordMapping[0]);
    glUniform1f(unRadius, m_planetGenData->radius);

    glUniform1f(unPatchWidth, (float)data->width * data->step);
    m_quad.draw();

    // Bind PBO
    vg::GpuMemory::bindBuffer(m_rawPbos[m_dBufferIndex][rawCounter], vg::BufferTarget::PIXEL_PACK_BUFFER);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, data->width, data->width, GL_RGBA, GL_FLOAT, 0);

    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::PIXEL_PACK_BUFFER);

    rawCounter++;
}

void SphericalTerrainGpuGenerator::init() {
    m_mesher = std::make_unique<TerrainPatchMesher>(m_meshManager, m_planetGenData);
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

    // Set up normal map uniforms
    m_normalProgram->use();
    glUniform1f(m_normalProgram->getUniform("unTexelWidth"), 1.0f / (float)PATCH_HEIGHTMAP_WIDTH);
    glUniform1f(m_normalProgram->getUniform("unNormalmapWidth"), (float)PATCH_NORMALMAP_WIDTH / (float)PATCH_HEIGHTMAP_WIDTH);
    m_normalProgram->unuse();
}

void SphericalTerrainGpuGenerator::updatePatchGeneration() {
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
            data->release();
            continue;
        }

        // Create and bind output normal map
        if (data->mesh->m_normalMap == 0) {
            data->mesh->m_normalMap = m_normalMapRecycler->produce();
        } else {
            glBindTexture(GL_TEXTURE_2D, data->mesh->m_normalMap);
        }

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

        // Set uniform
        glUniform1f(unWidth, (data->width / TEXELS_PER_PATCH) * M_PER_KM);

        // Generate normal map
        m_quad.draw();

        // And finally build the mesh
        m_mesher->buildMesh(data->mesh, data->startPos, data->cubeFace, data->width, m_heightData, data->isSpherical);

        data->release();
    }
    m_patchCounter[m_dBufferIndex] = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_normalProgram->disableVertexAttribArrays();
    m_normalProgram->unuse();
}

void SphericalTerrainGpuGenerator::updateRawGeneration() {

    float heightData[CHUNK_WIDTH][CHUNK_WIDTH][4];

    // Loop through all textures
    for (int i = 0; i < m_rawCounter[m_dBufferIndex]; i++) {

        RawHeightGenerator* data = m_rawDelegates[m_dBufferIndex][i];

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
        data->release();
    }
    m_rawCounter[m_dBufferIndex] = 0;
}