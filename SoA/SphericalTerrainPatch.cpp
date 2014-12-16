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
#include "RenderUtils.h"
#include "VoxelPlanetMapper.h"

#include "utils.h"

int lodDetailLevels[DetailLevels+3] = {8/scale, 16/scale, 32/scale, 64/scale, 128/scale, 256/scale, 512/scale, 1024/scale, 2048/scale, 4096/scale, 8192/scale, 16384/scale, 32768/scale, 65536/scale, 131072/scale, 262144/scale, 524228/scale};
int lodDistanceLevels[DetailLevels] = {500/scale, 1000/scale, 2000/scale, 4000/scale, 8000/scale, 16000/scale, 32000/scale, 64000/scale, 128000/scale, 256000/scale, 512000/scale, 1024000/scale, 4096000/scale, INT_MAX};

int WaterIndexMap[(maxVertexWidth+3)*(maxVertexWidth+3)*2];
int MakeWaterQuadMap[(maxVertexWidth+3)*(maxVertexWidth+3)];

#define INDICES_PER_QUAD 6
const int INDICES_PER_PATCH = (PATCH_WIDTH - 1) * (PATCH_WIDTH - 1) * INDICES_PER_QUAD;

// Coordinate mapping for rotating 2d grid to quadcube positions
// Pain of i32v3, first is coordinates
const i32v3 CubeCoordinateMappings[6] = {
    i32v3(0, 1, 2), //TOP
    i32v3(1, 0, 2), //LEFT
    i32v3(1, 0, 2), //RIGHT
    i32v3(0, 2, 1), //FRONT
    i32v3(0, 2, 1), //BACK
    i32v3(0, 1, 2) //BOTTOM
};

// Multipliers for coordinate mappings
const f32v3 CubeCoordinateMults[6] = {
    f32v3(1.0f, 1.0f, 1.0f), //TOP
    f32v3(1.0f, -1.0f, 1.0f), //LEFT
    f32v3(1.0f, 1.0f, 1.0f), //RIGHT
    f32v3(1.0f, 1.0f, 1.0f), //FRONT
    f32v3(1.0f, -1.0f, 1.0f), //BACK
    f32v3(1.0f, -1.0f, 1.0f) //BOTTOM
};

const ColorRGB8 DebugColors[6] {
    ColorRGB8(255, 0, 0), //TOP
        ColorRGB8(0, 255, 0), //LEFT
        ColorRGB8(0, 0, 255), //RIGHT
        ColorRGB8(255, 255, 0), //FRONT
        ColorRGB8(0, 255, 255), //BACK
        ColorRGB8(255, 255, 255) //BOTTOM
};

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

SphericalTerrainPatch::~SphericalTerrainPatch() {
    destroy();
}

void SphericalTerrainPatch::init(const f64v2& gridPosition,
                                 CubeFace cubeFace,
                                 const SphericalTerrainData* sphericalTerrainData,
                                 f64 width) {
    m_gridPosition = gridPosition;
    m_cubeFace = cubeFace;
    m_sphericalTerrainData = sphericalTerrainData;
    m_width = width;

    // Approximate the world position for now
    f64v2 centerGridPos = gridPosition + f64v2(width);

    const i32v3& coordMapping = CubeCoordinateMappings[(int)m_cubeFace];
    const f32v3& coordMults = CubeCoordinateMults[(int)m_cubeFace];

    m_worldPosition[coordMapping.x] = centerGridPos.x;
    m_worldPosition[coordMapping.y] = sphericalTerrainData->getRadius() * coordMults.y;
    m_worldPosition[coordMapping.z] = centerGridPos.y;

    m_worldPosition = glm::normalize(m_worldPosition);
}

void SphericalTerrainPatch::update(const f64v3& cameraPos) {
    m_distance = glm::length(m_worldPosition - cameraPos);
    
    if (m_children) {
        if (m_distance > m_width * 4.1) {
            delete[] m_children;
            m_children = nullptr;
        } else if (hasMesh()) {
            bool deleteMesh = true;
            for (int i = 0; i < 4; i++) {
                if (!m_children[i].isRenderable()) {
                    deleteMesh = true;
                    break;
                }
            }
            if (deleteMesh) {
                destroyMesh();
            }
        }
    } else if (m_distance < m_width * 4.0) {
        m_children = new SphericalTerrainPatch[4];
        // Segment in to 4 children
        for (int z = 0; z < 2; z++) {
            for (int x = 0; x < 2; x++) {
                m_children[(z << 1) + x].init(m_gridPosition + f64v2(m_width / 2.0 * x, m_width / 2.0 * z),
                                       m_cubeFace, m_sphericalTerrainData, m_width / 2.0);
            }
        }
    }
    
    if (m_children) {
        for (int i = 0; i < 4; i++) {
            m_children[i].update(cameraPos);
        }
    }
}

void SphericalTerrainPatch::destroy() {
    destroyMesh();
    delete[] m_children;
}

void SphericalTerrainPatch::draw(const f64v3& cameraPos, const f32m4& VP, vg::GLProgram* program) {
    if (m_children) {
        for (int i = 0; i < 4; i++) {
            m_children[i].draw(cameraPos, VP, program);
        }
        if (!hasMesh()) return;
    }

    if (!(hasMesh())) {
        update(cameraPos);
        float heightData[PATCH_WIDTH][PATCH_WIDTH];
        memset(heightData, 0, sizeof(heightData));
        generateMesh(heightData);
    }
  
    f32m4 matrix(1.0);
    setMatrixTranslation(matrix, -cameraPos);
    matrix = VP * matrix;


    // Point already has orientation encoded
    glUniformMatrix4fv(program->getUniform("unWVP"), 1, GL_FALSE, &matrix[0][0]);
    
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, position));
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, color));

    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLES, INDICES_PER_PATCH, GL_UNSIGNED_SHORT, 0);
  //  glBindVertexArray(0);
}

bool SphericalTerrainPatch::isRenderable() const {
    if (hasMesh()) return true;
    if (m_children) {
        for (int i = 0; i < 4; i++) {
            if (!m_children[i].isRenderable()) return false;
        }
        return true;
    }
    return false;
}

void SphericalTerrainPatch::destroyMesh() {
    if (m_vbo) {
        vg::GpuMemory::freeBuffer(m_vbo);
        m_vbo = 0;
    }
    if (m_ibo) {
        vg::GpuMemory::freeBuffer(m_ibo);
        m_ibo = 0;
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

void SphericalTerrainPatch::generateMesh(float heightData[PATCH_WIDTH][PATCH_WIDTH]) {

    const ColorRGB8 tcolor = DebugColors[(int)m_cubeFace];

    i32v3 coordMapping = CubeCoordinateMappings[(int)m_cubeFace];
    f32v3 coordMults = CubeCoordinateMults[(int)m_cubeFace];
    float radius = m_sphericalTerrainData->getRadius();

    std::vector <TerrainVertex> verts;
    verts.resize(PATCH_WIDTH * PATCH_WIDTH);
    // Loop through and set all vertex attributes
    float vertWidth = m_width / (PATCH_WIDTH - 1);
    int index = 0;
    for (int z = 0; z < PATCH_WIDTH; z++) {
        for (int x = 0; x < PATCH_WIDTH; x++) {
            auto& v = verts[index];
            // Set the position based on which face we are on
            v.position[coordMapping.x] = x * vertWidth + m_gridPosition.x * coordMults.x;
            v.position[coordMapping.y] = radius * coordMults.y;
            v.position[coordMapping.z] = z * vertWidth + m_gridPosition.y * coordMults.z;
            
            // Spherify it!
            v.position = glm::normalize(v.position) * radius;
            if (x == PATCH_WIDTH / 2 && z == PATCH_WIDTH / 2) {
                m_worldPosition = v.position;
            }

            v.color.r = tcolor.r;
            v.color.g = tcolor.g;
            v.color.b = tcolor.b;
            index++;
        }
    }

    std::vector <ui16> indices;
    indices.resize(INDICES_PER_PATCH);

    // Loop through each quad and set indices
    int vertIndex;
    index = 0;
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
            index += INDICES_PER_QUAD;
        }
    }
    if (m_vbo == 0) {
   //     glGenVertexArrays(1, &m_vao);
        vg::GpuMemory::createBuffer(m_vbo);
        vg::GpuMemory::createBuffer(m_ibo);
    }

  //  glBindVertexArray(m_vao);

    // Upload buffer data
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_vbo, vg::BufferTarget::ARRAY_BUFFER,
                                    verts.size() * sizeof(TerrainVertex),
                                    verts.data());
    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER,
                                    indices.size() * sizeof(ui16),
                                    indices.data());

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, position));
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, color));

 //   glBindVertexArray(0);
}
