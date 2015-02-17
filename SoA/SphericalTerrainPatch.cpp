#include "stdafx.h"
#include "SphericalTerrainPatch.h"
#include "SphericalTerrainPatchMesher.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/TextureRecycler.hpp>
#include <Vorb/utils.h>

#include "Camera.h"
#include "Chunk.h"
#include "RPC.h"
#include "RenderUtils.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainComponentUpdater.h"
#include "SphericalTerrainGpuGenerator.h"
#include "VoxelCoordinateSpaces.h"
#include "VoxelSpaceConversions.h"

const f32v3 NormalMults[6] = {
    f32v3(1.0f, 1.0f, -1.0f), //TOP
    f32v3(1.0f, 1.0f, -1.0f), //LEFT
    f32v3(1.0f, 1.0f, 1.0f), //RIGHT
    f32v3(1.0f, 1.0f, 1.0f), //FRONT
    f32v3(1.0f, 1.0f, -1.0f), //BACK
    f32v3(1.0f, 1.0f, 1.0f) //BOTTOM
};

SphericalTerrainMesh::~SphericalTerrainMesh() {
    if (m_vbo) {
        vg::GpuMemory::freeBuffer(m_vbo);
    }
    if (m_wvbo) {
        vg::GpuMemory::freeBuffer(m_wvbo);
    }
    if (m_wibo) {
        vg::GpuMemory::freeBuffer(m_wibo);
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (m_normalMap) {
        glDeleteTextures(1, &m_normalMap);
    }
}

void SphericalTerrainMesh::recycleNormalMap(vg::TextureRecycler* recycler) {
    if (m_normalMap) {
        recycler->recycle(m_normalMap);
        m_normalMap = 0;
    }
}

void SphericalTerrainMesh::draw(const f64v3& relativePos, const Camera* camera,
                                const f32m4& rot, vg::GLProgram* program) const {
    // Set up matrix
    f32m4 W(1.0);
    setMatrixTranslation(W, -relativePos);

    f32m4 WVP = camera->getViewProjectionMatrix() * W * rot;
    W *= rot;

    glUniform3fv(program->getUniform("unNormMult"), 1, &NormalMults[(int)m_cubeFace][0]);
    glUniformMatrix4fv(program->getUniform("unWVP"), 1, GL_FALSE, &WVP[0][0]);
    glUniformMatrix4fv(program->getUniform("unW"), 1, GL_FALSE, &W[0][0]);

    // TODO: Using a VAO makes it not work??
   // glBindVertexArray(m_vao);

    glBindTexture(GL_TEXTURE_2D, m_normalMap);

    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, tangent));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, texCoords));
    glVertexAttribPointer(3, 3, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, color));
    glVertexAttribPointer(4, 2, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, normTexCoords));
    glVertexAttribPointer(5, 2, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, temperature));

    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLES, SphericalTerrainPatch::INDICES_PER_PATCH, GL_UNSIGNED_SHORT, 0);
 //   glBindVertexArray(0);
}

void SphericalTerrainMesh::drawWater(const f64v3& relativePos, const Camera* camera,
                                     const f32m4& rot, vg::GLProgram* program) const {
    // Set up matrix
    f32m4 W(1.0);
    setMatrixTranslation(W, -relativePos);
    W *= rot;
    f32m4 WVP = camera->getViewProjectionMatrix() * W;

    glUniform3fv(program->getUniform("unNormMult"), 1, &NormalMults[(int)m_cubeFace][0]);
    glUniformMatrix4fv(program->getUniform("unWVP"), 1, GL_FALSE, &WVP[0][0]);
    glUniformMatrix4fv(program->getUniform("unW"), 1, GL_FALSE, &W[0][0]);

    // TODO: Using a VAO makes it not work??
    // glBindVertexArray(m_vao);

    vg::GpuMemory::bindBuffer(m_wvbo, vg::BufferTarget::ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, tangent));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, color));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, texCoords));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, depth));


    vg::GpuMemory::bindBuffer(m_wibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLES, m_waterIndexCount, GL_UNSIGNED_SHORT, 0);

    //   glBindVertexArray(0);
}

f32v3 SphericalTerrainMesh::getClosestPoint(const f32v3& camPos) const {
    return getClosestPointOnAABB(camPos, m_aabbPos, m_aabbDims);
}
f64v3 SphericalTerrainMesh::getClosestPoint(const f64v3& camPos) const {
    return getClosestPointOnAABB(camPos, f64v3(m_aabbPos), f64v3(m_aabbDims));
}


SphericalTerrainPatch::~SphericalTerrainPatch() {
    destroy();
}

void SphericalTerrainPatch::init(const f64v2& gridPosition,
                                 WorldCubeFace cubeFace,
                                 int lod,
                                 const SphericalTerrainData* sphericalTerrainData,
                                 f64 width,
                                 TerrainRpcDispatcher* dispatcher) {
    m_gridPos = gridPosition;
    m_cubeFace = cubeFace;
    m_lod = lod;
    m_sphericalTerrainData = sphericalTerrainData;
    m_width = width;
    m_dispatcher = dispatcher;

    // Construct an approximate AABB
    const i32v3& coordMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)m_cubeFace];
    const i32v2& coordMults = VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)m_cubeFace];
    f64v3 corners[4];
    corners[0][coordMapping.x] = gridPosition.x * coordMults.x;
    corners[0][coordMapping.y] = sphericalTerrainData->getRadius() * VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace];
    corners[0][coordMapping.z] = gridPosition.y * coordMults.y;
    corners[0] = glm::normalize(corners[0]) * m_sphericalTerrainData->getRadius();
    corners[1][coordMapping.x] = gridPosition.x * coordMults.x;
    corners[1][coordMapping.y] = sphericalTerrainData->getRadius() * VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace];
    corners[1][coordMapping.z] = (gridPosition.y + m_width) * coordMults.y;
    corners[1] = glm::normalize(corners[1]) * m_sphericalTerrainData->getRadius();
    corners[2][coordMapping.x] = (gridPosition.x + m_width) * coordMults.x;
    corners[2][coordMapping.y] = sphericalTerrainData->getRadius() * VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace];
    corners[2][coordMapping.z] = (gridPosition.y + m_width) * coordMults.y;
    corners[2] = glm::normalize(corners[2]) * m_sphericalTerrainData->getRadius();
    corners[3][coordMapping.x] = (gridPosition.x + m_width) * coordMults.x;
    corners[3][coordMapping.y] = sphericalTerrainData->getRadius() * VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace];
    corners[3][coordMapping.z] = gridPosition.y * coordMults.y;
    corners[3] = glm::normalize(corners[3]) * m_sphericalTerrainData->getRadius();

    f64 minX = INT_MAX, maxX = INT_MIN;
    f64 minY = INT_MAX, maxY = INT_MIN;
    f64 minZ = INT_MAX, maxZ = INT_MIN;
    for (int i = 0; i < 4; i++) {
        auto& c = corners[i];
        if (c.x < minX) {
            minX = c.x;
        } else if (c.x > maxX) {
            maxX = c.x;
        }
        if (c.y < minY) {
            minY = c.y;
        } else if (c.y > maxY) {
            maxY = c.y;
        }
        if (c.z < minZ) {
            minZ = c.z;
        } else if (c.z > maxZ) {
            maxZ = c.z;
        }
    }
    // Get world position and bounding box
    m_aabbPos = f32v3(minX, minY, minZ);
    m_aabbDims = f32v3(maxX - minX, maxY - minY, maxZ - minZ);
}

void SphericalTerrainPatch::update(const f64v3& cameraPos) {
    const float DIST_MIN = 3.0f;
    const float DIST_MAX = 3.1f;

    const float MIN_SIZE = 0.4096f;
    
    // Calculate distance from camera
    f64v3 closestPoint = calculateClosestPointAndDist(cameraPos);
    
    if (m_children) {
        if (m_distance > m_width * DIST_MAX) {
            if (!m_mesh) {
                requestMesh();
            }
            if (hasMesh()) {
                // Out of range, kill children
                delete[] m_children;
                m_children = nullptr;
            }
        } else if (m_mesh) {
            // In range, but we need to remove our mesh.
            // Check to see if all children are renderable
            bool deleteMesh = true;
            for (int i = 0; i < 4; i++) {
                if (!m_children[i].isRenderable()) {
                    deleteMesh = false;
                    break;
                }
            }
            
            if (deleteMesh) {
                // Children are renderable, free mesh.
                // Render thread will deallocate.
                m_mesh->m_shouldDelete = true;
                m_mesh = nullptr;
            }
        }
    } else if (m_lod < MAX_LOD && m_distance < m_width * DIST_MIN && m_width > MIN_SIZE) {
        // Check if we are over horizon. If we are, don't divide.
        if (!isOverHorizon(cameraPos, closestPoint, m_sphericalTerrainData->getRadius())) {
            m_children = new SphericalTerrainPatch[4];
            // Segment into 4 children
            for (int z = 0; z < 2; z++) {
                for (int x = 0; x < 2; x++) {
                    m_children[(z << 1) + x].init(m_gridPos + f64v2((m_width / 2.0) * x, (m_width / 2.0) * z),
                                                  m_cubeFace, m_lod + 1, m_sphericalTerrainData, m_width / 2.0,
                                                  m_dispatcher);
                }
            }
        }
    } else if (!m_mesh) {
        requestMesh();
    }
    
    // Recursively update children if they exist
    if (m_children) {
        for (int i = 0; i < 4; i++) {
            m_children[i].update(cameraPos);
        }
    }
}

void SphericalTerrainPatch::destroy() {
    if (m_mesh) {
        m_mesh->m_shouldDelete = true;
        m_mesh = nullptr;
    }
    delete[] m_children;
    m_children = nullptr;
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

bool SphericalTerrainPatch::isOverHorizon(const f32v3 &relCamPos, const f32v3 &point, f32 planetRadius) {
    const float DELTA = 0.1f;
    f32 pLength = glm::length(relCamPos);
    f32v3 ncp = relCamPos / pLength;

    if (pLength < planetRadius + 1.0f) pLength = planetRadius + 1.0f;
    f32 horizonAngle = acos(planetRadius / pLength);
    f32 lodAngle = acos(glm::dot(ncp, glm::normalize(point)));
    if (lodAngle >= horizonAngle + DELTA) return true;
    return false;
}
bool SphericalTerrainPatch::isOverHorizon(const f64v3 &relCamPos, const f64v3 &point, f64 planetRadius) {
    const float DELTA = 0.1;
    f64 pLength = glm::length(relCamPos);
    f64v3 ncp = relCamPos / pLength;

    if (pLength < planetRadius + 1.0) pLength = planetRadius + 1.0;
    f64 horizonAngle = acos(planetRadius / pLength);
    f64 lodAngle = acos(glm::dot(ncp, glm::normalize(point)));
    if (lodAngle >= horizonAngle + DELTA) return true;
    return false;
}

void SphericalTerrainPatch::requestMesh() {
    // Try to generate a mesh
    const i32v2& coordMults = VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)m_cubeFace];

    f32v3 startPos(m_gridPos.x * coordMults.x,
                   m_sphericalTerrainData->getRadius() * VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace],
                   m_gridPos.y* coordMults.y);
    m_mesh = m_dispatcher->dispatchTerrainGen(startPos,
                                              m_width,
                                              m_lod,
                                              m_cubeFace, true);
}
f64v3 SphericalTerrainPatch::calculateClosestPointAndDist(const f64v3& cameraPos) {
    if (hasMesh()) {
        // If we have a mesh, we can use it's accurate bounding box    
        m_mesh->getClosestPoint(cameraPos, closestPoint);
    } else {
        getClosestPointOnAABB(cameraPos, m_aabbPos, m_aabbDims, closestPoint);
    }
    m_distance = glm::length(closestPoint - cameraPos);
}
