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

    // Approximate the world position for now //TODO(Ben): Better
    f64v2 centerGridPos = gridPosition + f64v2(width);

    const i32v3& coordMapping = CubeCoordinateMappings[(int)m_cubeFace];
    const f32v3& coordMults = CubeCoordinateMults[(int)m_cubeFace];

    m_worldPosition[coordMapping.x] = centerGridPos.x;
    m_worldPosition[coordMapping.y] = sphericalTerrainData->getRadius() * coordMults.y;
    m_worldPosition[coordMapping.z] = centerGridPos.y;

    m_worldPosition = glm::normalize(m_worldPosition);
}

void SphericalTerrainPatch::update(const f64v3& cameraPos) {
    // Calculate distance from camera
    m_distance = glm::length(m_worldPosition - cameraPos);
    
    if (m_children) {
   
        if (m_distance > m_width * 4.1) {
            // Out of range, kill children
            delete[] m_children;
            m_children = nullptr;
        } else if (hasMesh()) {
            // In range, but we need to remove our mesh.
            // Check to see if all children are renderable
            bool deleteMesh = true;
            for (int i = 0; i < 4; i++) {
                if (!m_children[i].isRenderable()) {
                    deleteMesh = true;
                    break;
                }
            }
            
            if (deleteMesh) {
                // Children are renderable, free mesh
                destroyMesh();
            }
        }
    } else if (m_distance < m_width * 4.0) {
        m_children = new SphericalTerrainPatch[4];
        // Segment into 4 children
        for (int z = 0; z < 2; z++) {
            for (int x = 0; x < 2; x++) {
                m_children[(z << 1) + x].init(m_gridPosition + f64v2(m_width / 2.0 * x, m_width / 2.0 * z),
                                       m_cubeFace, m_sphericalTerrainData, m_width / 2.0);
            }
        }
    }
    
    // Recursively update children if they exist
    if (m_children) {
        for (int i = 0; i < 4; i++) {
            m_children[i].update(cameraPos);
        }
    }
}

void SphericalTerrainPatch::destroy() {
    destroyMesh();
    delete[] m_children;
    m_children = nullptr;
}

void SphericalTerrainPatch::draw(const f64v3& cameraPos, const f32m4& VP, vg::GLProgram* program) {
    if (m_children) {
        // Draw the children
        for (int i = 0; i < 4; i++) {
            m_children[i].draw(cameraPos, VP, program);
        }
        // If we have a mesh, we should still render it for now
        if (!hasMesh()) return;
    }

    // If we don't have a mesh, generate one
    if (!(hasMesh())) {
        update(cameraPos);
        float heightData[PATCH_WIDTH][PATCH_WIDTH];
        memset(heightData, 0, sizeof(heightData));
        generateMesh(heightData);
    }
  
    // Set up matrix
    f32m4 matrix(1.0);
    setMatrixTranslation(matrix, -cameraPos);
    matrix = VP * matrix;


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

    // Get debug face color
    const ColorRGB8 tcolor = DebugColors[(int)m_cubeFace];

    // Grab mappings so we can rotate the 2D grid appropriately
    i32v3 coordMapping = CubeCoordinateMappings[(int)m_cubeFace];
    f32v3 coordMults = CubeCoordinateMults[(int)m_cubeFace];
    float radius = m_sphericalTerrainData->getRadius();
    
    // TODO(Ben): Stack array instead?
    // Preallocate the verts for speed
    std::vector <TerrainVertex> verts(PATCH_WIDTH * PATCH_WIDTH);

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

    // Preallocate indices for speed
    std::vector <ui16> indices(INDICES_PER_PATCH);

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
    // If the buffers haven't been generated, generate them
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
