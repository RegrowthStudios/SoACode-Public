#include "stdafx.h"
#include "SphericalTerrainGenerator.h"

const ColorRGB8 DebugColors[6] {
    ColorRGB8(255, 0, 0), //TOP
        ColorRGB8(0, 255, 0), //LEFT
        ColorRGB8(0, 0, 255), //RIGHT
        ColorRGB8(255, 255, 0), //FRONT
        ColorRGB8(0, 255, 255), //BACK
        ColorRGB8(255, 255, 255) //BOTTOM
};

SphericalTerrainGenerator::SphericalTerrainGenerator() {
}


SphericalTerrainGenerator::~SphericalTerrainGenerator() {
}


void SphericalTerrainGenerator::generateMesh(TerrainGenDelegate* data) {

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
