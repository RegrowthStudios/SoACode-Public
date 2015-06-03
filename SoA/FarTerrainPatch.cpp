#include "stdafx.h"
#include "FarTerrainPatch.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/TextureRecycler.hpp>
#include <Vorb/utils.h>

#include "Camera.h"
#include "RenderUtils.h"
#include "TerrainPatchMesher.h"
#include "TerrainRpcDispatcher.h"
#include "VoxelCoordinateSpaces.h"
#include "VoxelSpaceConversions.h"
#include "soaUtils.h"

FarTerrainPatch::~FarTerrainPatch() {
    destroy();
}

void FarTerrainPatch::init(const f64v2& gridPosition, WorldCubeFace cubeFace, int lod, const TerrainPatchData* sphericalTerrainData, f64 width, TerrainRpcDispatcher* dispatcher) {
    m_gridPos = gridPosition;
    m_cubeFace = cubeFace;
    m_lod = lod;
    m_terrainPatchData = sphericalTerrainData;
    m_width = width;
    m_dispatcher = dispatcher;

    // Get world position and bounding box
    m_aabbPos = f32v3(m_gridPos.x, 0, m_gridPos.y);
    m_aabbDims = f32v3(m_width, 0, m_width);
}

void FarTerrainPatch::update(const f64v3& cameraPos) {
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
    } else if (m_lod < PATCH_MAX_LOD && m_distance < m_width * DIST_MIN && m_width > MIN_SIZE) {
        m_children = new FarTerrainPatch[4];
        // Segment into 4 children
        for (int z = 0; z < 2; z++) {
            for (int x = 0; x < 2; x++) {
                m_children[(z << 1) + x].init(m_gridPos + f64v2((m_width / 2.0) * x, (m_width / 2.0) * z),
                                                m_cubeFace, m_lod + 1, m_terrainPatchData, m_width / 2.0,
                                                m_dispatcher);
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

bool FarTerrainPatch::isOverHorizon(const f64v3 &relCamPos, const f64v3 &point, f64 planetRadius) {
    const f64 DELTA = 0.1;

    // Position of point relative to sphere tip
    f64v3 spherePoint = point - f64v3(relCamPos.x, -planetRadius, relCamPos.z);
    // We assume the camera is at the tip of the sphere
    f64v3 sphereCamPos(0, relCamPos.y + planetRadius, 0);

    f64 camHeight = glm::length(sphereCamPos);
    f64v3 normalizedCamPos = sphereCamPos / camHeight;

    // Limit the camera depth
    if (camHeight < planetRadius + 1.0) camHeight = planetRadius + 1.0;

    f64 horizonAngle = acos(planetRadius / camHeight);
    f64 lodAngle = acos(glm::dot(normalizedCamPos, glm::normalize(spherePoint)));
    if (lodAngle >= horizonAngle + DELTA) {
        return true;
    }
    return false;
}

void FarTerrainPatch::requestMesh() {
    f32v3 startPos(m_gridPos.x,
                   m_terrainPatchData->radius,
                   m_gridPos.y);
    m_mesh = m_dispatcher->dispatchTerrainGen(startPos,
                                              (f32)m_width,
                                              m_lod,
                                              m_cubeFace, false);
}
