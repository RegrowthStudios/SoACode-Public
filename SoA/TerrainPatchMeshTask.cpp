#include "stdafx.h"
#include "SphericalTerrainCpuGenerator.h"
#include "TerrainPatchMesh.h"
#include "TerrainPatchMeshManager.h"
#include "TerrainPatchMeshTask.h"
#include "TerrainPatchMesher.h"
#include "VoxelSpaceConversions.h"

void TerrainPatchMeshTask::init(const TerrainPatchData* patchData,
                                TerrainPatchMesh* mesh,
                                const f32v3& startPos,
                                float width,
                                WorldCubeFace cubeFace) {
    m_patchData = patchData;
    m_mesh = mesh;
    m_startPos = startPos;
    m_width = width;
    m_cubeFace = cubeFace;
}

void TerrainPatchMeshTask::execute(WorkerData* workerData) {

    f32 heightData[PADDED_PATCH_WIDTH][PADDED_PATCH_WIDTH][4];
    f64v3 positionData[PADDED_PATCH_WIDTH][PADDED_PATCH_WIDTH];
    f64v3 pos;
    f32v3 tmpPos;

    SphericalHeightmapGenerator* generator = m_patchData->generator;
    if (m_mesh->m_shouldDelete) {
        delete m_mesh;
        return;
    }
    const float VERT_WIDTH = m_width / (PATCH_WIDTH - 1);
    bool isSpherical = m_mesh->getIsSpherical();

    if (isSpherical) {
        const i32v3& coordMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)m_cubeFace];
        const f32v2& coordMults = f32v2(VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)m_cubeFace]);
      
        m_startPos.y *= (f32)VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace];
        for (int z = 0; z < PADDED_PATCH_WIDTH; z++) {
            for (int x = 0; x < PADDED_PATCH_WIDTH; x++) {
                pos[coordMapping.x] = (m_startPos.x + (x - 1) * VERT_WIDTH) * coordMults.x;
                pos[coordMapping.y] = m_startPos.y;
                pos[coordMapping.z] = (m_startPos.z + (z - 1) * VERT_WIDTH) * coordMults.y;
                f64v3 normal(glm::normalize(pos));
                pos = normal * m_patchData->radius;
                f64 height = generator->getHeightValue(pos) * KM_PER_M;
                heightData[z][x][0] = (f32)height;
                heightData[z][x][1] = (f32)generator->getTemperatureValue(pos, normal, height);
                heightData[z][x][2] = (f32)generator->getHumidityValue(pos, normal, height);

                // offset position by height;
                positionData[z][x] = normal * (m_patchData->radius + heightData[z][x][0]);
            }
        }
    } else { // Far terrain
        const i32v3& coordMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)m_cubeFace];
        const f32v2& coordMults = f32v2(VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)m_cubeFace]);

        m_startPos.y *= (f32)VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace];
        for (int z = 0; z < PADDED_PATCH_WIDTH; z++) {
            for (int x = 0; x < PADDED_PATCH_WIDTH; x++) {
                f64v2 spos;
                spos.x = (m_startPos.x + (x - 1) * VERT_WIDTH);
                spos.y = (m_startPos.z + (z - 1) * VERT_WIDTH);
                pos[coordMapping.x] = spos.x * coordMults.x;
                pos[coordMapping.y] = m_startPos.y;
                pos[coordMapping.z] = spos.y * coordMults.y;
                f64v3 normal(glm::normalize(pos));
                pos = normal * m_patchData->radius;
                f64 height = generator->getHeightValue(pos) * KM_PER_M;
                heightData[z][x][0] = (f32)height;
                heightData[z][x][1] = (f32)generator->getTemperatureValue(pos, normal, height);
                heightData[z][x][2] = (f32)generator->getHumidityValue(pos, normal, height);

                // offset position by height;
                positionData[z][x] = f64v3(spos.x, heightData[z][x][0], spos.y);
            }
        }
    }
    // Check for early delete
    if (m_mesh->m_shouldDelete) {
        delete m_mesh;
        return;
    }
    if (!workerData->terrainMesher) workerData->terrainMesher = new TerrainPatchMesher();
    workerData->terrainMesher->generateMeshData(m_mesh, generator->getGenData(), m_startPos, m_cubeFace, m_width,
                                                heightData, positionData);

    // Finally, add to the mesh manager
    m_patchData->meshManager->addMeshAsync(m_mesh);
}
