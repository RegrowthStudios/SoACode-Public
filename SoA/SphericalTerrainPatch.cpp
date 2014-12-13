#include "stdafx.h"
#include "SphericalTerrainPatch.h"

#include "BlockData.h"
#include "Camera.h"
#include "Chunk.h"
#include "FloraGenerator.h"
#include "GameManager.h"
#include "MessageManager.h"
#include "Options.h"
#include "WorldStructs.h"
#include "GpuMemory.h"

#include "utils.h"

int lodDetailLevels[DetailLevels+3] = {8/scale, 16/scale, 32/scale, 64/scale, 128/scale, 256/scale, 512/scale, 1024/scale, 2048/scale, 4096/scale, 8192/scale, 16384/scale, 32768/scale, 65536/scale, 131072/scale, 262144/scale, 524228/scale};
int lodDistanceLevels[DetailLevels] = {500/scale, 1000/scale, 2000/scale, 4000/scale, 8000/scale, 16000/scale, 32000/scale, 64000/scale, 128000/scale, 256000/scale, 512000/scale, 1024000/scale, 4096000/scale, INT_MAX};

int WaterIndexMap[(maxVertexWidth+3)*(maxVertexWidth+3)*2];
int MakeWaterQuadMap[(maxVertexWidth+3)*(maxVertexWidth+3)];

//a   b
//c   d
inline double BilinearInterpolation(int &a, int &b, int &c, int &d, int &step, float &x, float &z)
{
    double px, pz;
    px = ((double)(x))/step;
    pz = ((double)(z))/step;

    return  (a)*(1-px)*(1-pz) +
            (b)*px*(1-pz) +
            (c)*(1-px)*pz +
            (d)*px*pz;
}

SphericalTerrainPatch::SphericalTerrainPatch(const f64v2& gridPosition,
                                             const SphericalTerrainData* sphericalTerrainData) :
    m_gridPosition(gridPosition),
    m_sphericalTerrainData(sphericalTerrainData) {

}

void SphericalTerrainPatch::update(const f64v3& cameraPos) {
    m_distance = glm::length(m_worldPosition - cameraPos);

    if (m_hasMesh == false) {
        float heightData[PATCH_WIDTH][PATCH_WIDTH];
        memset(heightData, 0, sizeof(heightData));
        generateMesh(heightData);
    }
}

void SphericalTerrainPatch::generateMesh(float heightData[PATCH_WIDTH][PATCH_WIDTH]) {
#define INDICES_PER_QUAD 6

    static const ColorRGB8 tcolor(0, 128, 0);

    std::vector <TerrainVertex> verts;
    verts.resize(PATCH_WIDTH * PATCH_WIDTH);
    // Loop through each vertex
    int index = 0;
    for (int z = 0; z < PATCH_WIDTH; z++) {
        for (int x = 0; x < PATCH_WIDTH; x++) {
            verts[index].position = f32v3(x, 0, z);
            verts[index].color = tcolor;
            index++;
        }
    }

    std::vector <ui16> indices;
    indices.resize((PATCH_WIDTH - 1) * (PATCH_WIDTH - 1) * INDICES_PER_QUAD);

    // Loop through each quad and set indices
    int vertIndex;
    index = 0;
    for (int z = 0; z < PATCH_WIDTH - 1; z++) {
        for (int x = 0; x < PATCH_WIDTH - 1; x++) {
            // Compute index of back left vertex
            vertIndex = z * PATCH_WIDTH + x;
            indices[index] = vertIndex;
            indices[index + 1] = vertIndex + PATCH_WIDTH;
            indices[index + 2] = vertIndex + PATCH_WIDTH + 1;
            indices[index + 3] = vertIndex + PATCH_WIDTH + 1;
            indices[index + 4] = vertIndex + 1;
            indices[index + 5] = vertIndex;
            index += INDICES_PER_QUAD;
        }
    }
    if (m_vbo == 0) {
        vg::GpuMemory::createBuffer(m_vbo);
        vg::GpuMemory::createBuffer(m_ibo);
    }

    // Upload buffer data
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_vbo, vg::BufferTarget::ARRAY_BUFFER,
                                    verts.size() * sizeof(TerrainVertex),
                                    verts.data());
    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER,
                                    indices.size() * sizeof(ui16),
                                    indices.data());

    m_hasMesh = true;
}
