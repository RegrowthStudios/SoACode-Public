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
    f32v3 worldNormalData[PADDED_PATCH_WIDTH][PADDED_PATCH_WIDTH];
    f64v3 pos;
    f32v3 tmpPos;
    const i32v3& coordMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)m_cubeFace];
    const f32v2& coordMults = f32v2(VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)m_cubeFace]);
    const i32v3& trueMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)m_cubeFace];

    const float VERT_WIDTH = m_width / (PATCH_WIDTH - 1);
    bool isSpherical = m_mesh->getIsSpherical();
    // Check for early delete
    if (m_mesh->m_shouldDelete) {
        delete m_mesh;
        return;
    }
    SphericalTerrainCpuGenerator* generator = m_patchData->generator;
    m_startPos.y *= (f32)VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace];
    memset(heightData, 0, sizeof(heightData));
    for (int z = 0; z < PADDED_PATCH_WIDTH; z++) {
        for (int x = 0; x < PADDED_PATCH_WIDTH; x++) {
            pos[coordMapping.x] = (m_startPos.x + (x - 1) * VERT_WIDTH) * coordMults.x;
            pos[coordMapping.y] = m_startPos.y;
            pos[coordMapping.z] = (m_startPos.z + (z - 1) * VERT_WIDTH) * coordMults.y;
            f64v3 normal(glm::normalize(pos));
            worldNormalData[z][x] = normal;
            pos = normal * m_patchData->radius;
            heightData[z][x][0] = (f32)generator->getHeightValue(pos) * KM_PER_M;
            heightData[z][x][1] = (f32)generator->getTemperatureValue(pos);
            heightData[z][x][2] = (f32)generator->getHumidityValue(pos);
           
            // offset position by height;
            // TODO(Ben): Separate this out to prevent branching inside
            if (isSpherical) {
                positionData[z][x] = normal * (m_patchData->radius + heightData[z][x][0]);
            } else {
                tmpPos[trueMapping.x] = pos.x;
                tmpPos[trueMapping.y] = m_patchData->radius * (f32)VoxelSpaceConversions::FACE_Y_MULTS[(int)m_cubeFace];
                tmpPos[trueMapping.z] = pos.z;
                worldNormalData[z][x] = glm::normalize(tmpPos);
                pos.y += heightData[z][x][0];
                positionData[z][x] = pos;
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
                                                heightData, positionData, worldNormalData);

    // Finally, add to the mesh manager
    m_patchData->meshManager->addMeshAsync(m_mesh);
}
