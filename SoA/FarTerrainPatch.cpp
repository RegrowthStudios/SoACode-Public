#include "stdafx.h"
#include "FarTerrainPatch.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/TextureRecycler.hpp>
#include <Vorb/utils.h>

#include "Camera.h"
#include "RenderUtils.h"
#include "VoxelCoordinateSpaces.h"
#include "VoxelSpaceConversions.h"
#include "TerrainPatchMesher.h"
#include "TerrainRpcDispatcher.h"

const f32v3 NormalMults[6] = {
    f32v3(1.0f, 1.0f, -1.0f), //TOP
    f32v3(1.0f, 1.0f, -1.0f), //LEFT
    f32v3(1.0f, 1.0f, 1.0f), //RIGHT
    f32v3(1.0f, 1.0f, 1.0f), //FRONT
    f32v3(1.0f, 1.0f, -1.0f), //BACK
    f32v3(1.0f, 1.0f, 1.0f) //BOTTOM
};

FarTerrainPatch::~FarTerrainPatch() {
    destroy();
}

void FarTerrainPatch::init(const f64v2& gridPosition, WorldCubeFace cubeFace, int lod, const TerrainPatchData* sphericalTerrainData, f64 width, TerrainRpcDispatcher* dispatcher) {
    m_gridPos = gridPosition;
    m_cubeFace = cubeFace;
    m_lod = lod;
    m_sphericalTerrainData = sphericalTerrainData;
    m_width = width;
    m_dispatcher = dispatcher;

    f64v2 centerGridPos = gridPosition + f64v2(width / 2.0);
    m_aabbPos = f64v3(m_gridPos.x, 0, m_gridPos.y);
}

void FarTerrainPatch::update(const f64v3& cameraPos) {
    const float DIST_MIN = 3.0f;
    const float DIST_MAX = 3.1f;

    const float MIN_SIZE = 0.4096f;

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
        // Only subdivide if we are visible over horizon
        bool divide = true;
        if (hasMesh()) {
            if (isOverHorizon(cameraPos, closestPoint, m_sphericalTerrainData->getRadius())) {
                //     divide = false;
            }
        } else if (isOverHorizon(cameraPos, m_aabbPos, m_sphericalTerrainData->getRadius())) {
            //  divide = false;
        }

        if (divide) {
            m_children = new FarTerrainPatch[4];
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

void FarTerrainPatch::requestMesh() {
    // Try to generate a mesh
    const i32v2& coordMults = VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)m_cubeFace];

    f32v3 startPos(m_gridPos.x * coordMults.x,
                   m_sphericalTerrainData->getRadius() * VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace],
                   m_gridPos.y* coordMults.y);
    m_mesh = m_dispatcher->dispatchTerrainGen(startPos,
                                              m_width,
                                              m_lod,
                                              m_cubeFace, false);
}
