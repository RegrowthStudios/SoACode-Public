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

SphericalTerrainGenerator::SphericalTerrainGenerator(float radius, vg::GLProgram* genProgram) :
    m_radius(radius),
    m_genProgram(genProgram) {
    m_textures.init(ui32v2(PATCH_WIDTH));
    m_quad.init();
}

SphericalTerrainGenerator::~SphericalTerrainGenerator() {
    // Empty
}

void SphericalTerrainGenerator::update() {
    m_genProgram->use();
    m_genProgram->enableVertexAttribArrays();
    m_textures.use();

    #define MAX_REQUESTS 16UL
    m_rpcManager.processRequests(MAX_REQUESTS);

    m_textures.unuse();
    m_genProgram->disableVertexAttribArrays();
    m_genProgram->unuse();
}

void SphericalTerrainGenerator::generateTerrain(TerrainGenDelegate* data) {
    
    m_quad.draw();
    checkGlError("DRAW");
    glFlush();
    glFinish();
    checkGlError("FINISH");
    glBindTexture(GL_TEXTURE_2D, m_textures.getTextureIDs().height);
    checkGlError("BIND");
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data->heightData);
    checkGlError("GET");
    std::cout << data->heightData[0][0] << std::endl;
    
    buildMesh(data);
    checkGlError("BUILD");
}

void SphericalTerrainGenerator::buildMesh(TerrainGenDelegate* data) {

    // Get debug face color
    const ColorRGB8 tcolor = DebugColors[1];

    // Grab mappings so we can rotate the 2D grid appropriately
    const i32v3& coordMapping = data->coordMapping;
    const f32v3& startPos = data->startPos;
    SphericalTerrainMesh* mesh = data->mesh;
    float width = data->width;
  
    // TODO(Ben): Stack array instead?
    // Preallocate the verts for speed
    std::vector <TerrainVertex> verts(PATCH_WIDTH * PATCH_WIDTH);

    // Loop through and set all vertex attributes
    float vertWidth = width / (PATCH_WIDTH - 1);
    int index = 0;
    for (int z = 0; z < PATCH_WIDTH; z++) {
        for (int x = 0; x < PATCH_WIDTH; x++) {
            auto& v = verts[index];
            // Set the position based on which face we are on
            v.position[coordMapping.x] = x * vertWidth + startPos.x;
            v.position[coordMapping.y] = data->heightData[z][x] + startPos.y;
            v.position[coordMapping.z] = z * vertWidth + startPos.z;

            // Spherify it!
            v.position = glm::normalize(v.position) * m_radius;
            if (x == PATCH_WIDTH / 2 && z == PATCH_WIDTH / 2) {
                mesh->worldPosition = v.position;
            }

            v.color.r = tcolor.r;
            v.color.g = tcolor.g;
            v.color.b = tcolor.b;
            index++;
        }
    }

    // If the buffers haven't been generated, generate them
    if (mesh->m_vbo == 0) {
        //     glGenVertexArrays(1, &m_vao);
        vg::GpuMemory::createBuffer(mesh->m_vbo);
        vg::GpuMemory::createBuffer(mesh->m_ibo);
    }

    generateIndices(data);

    // TODO: Using a VAO makes it not work??
    //  glBindVertexArray(m_vao);

    // Upload buffer data
    vg::GpuMemory::bindBuffer(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(mesh->m_vbo, vg::BufferTarget::ARRAY_BUFFER,
                                    verts.size() * sizeof(TerrainVertex),
                                    verts.data());

    // TODO: Using a VAO makes it not work??
    //   glBindVertexArray(0);
}

void SphericalTerrainGenerator::generateIndices(TerrainGenDelegate* data) {
    // Preallocate indices for speed
    std::vector <ui16> indices(SphericalTerrainPatch::INDICES_PER_PATCH);
    // Loop through each quad and set indices
    int vertIndex;
    int index = 0;
    if (CubeWindings[(int)(data->cubeFace)]) {
        // CCW
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
    } else {
        //CW
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
    }
    SphericalTerrainMesh* mesh = data->mesh;
    vg::GpuMemory::bindBuffer(mesh->m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(mesh->m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER,
                                    indices.size() * sizeof(ui16),
                                    indices.data());
}