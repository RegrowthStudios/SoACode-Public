#include "stdafx.h"
#include "TerrainPatchMeshTask.h"
#include "TerrainPatchMesher.h"
#include "TerrainPatchMeshManager.h"
#include "VoxelSpaceConversions.h"
#include "SphericalTerrainCpuGenerator.h"

void TerrainPatchMeshTask::init(const TerrainPatchData* patchData,
                                TerrainPatchMesh* mesh,
                                const f32v3& startPos,
                                float width,
                                WorldCubeFace cubeFace,
                                bool isSpherical) {
    m_patchData = patchData;
    m_mesh = mesh;
    m_startPos = startPos;
    m_width = width;
    m_cubeFace = cubeFace;
    m_isSpherical = isSpherical;
}

void TerrainPatchMeshTask::execute(WorkerData* workerData) {

    f32 heightData[PATCH_WIDTH][PATCH_WIDTH][4];

    f64v3 pos;
    const i32v3& coordMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)m_cubeFace];
    const f32v2& coordMults = f32v2(VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)m_cubeFace]);
    
    const float VERT_WIDTH = m_width / PATCH_WIDTH;

    SphericalTerrainCpuGenerator* generator = m_patchData->generator;

    memset(heightData, 0, sizeof(heightData));
    for (int z = 0; z < PATCH_WIDTH; z++) {
        for (int x = 0; x < PATCH_WIDTH; x++) {

            pos[coordMapping.x] = m_startPos.x + x * VERT_WIDTH * coordMults.x;
            pos[coordMapping.y] = m_startPos.y;
            pos[coordMapping.z] = m_startPos.z + z * VERT_WIDTH * coordMults.y;

            heightData[z][x][0] = generator->getHeightValue(pos);
            heightData[z][x][1] = generator->getTemperatureValue(pos);
            heightData[z][x][2] = generator->getHumidityValue(pos);
        }
    }

    if (!workerData->terrainMesher) workerData->terrainMesher = new TerrainPatchMesher(generator->getGenData());
    workerData->terrainMesher->buildMesh(m_mesh, m_startPos, m_cubeFace, m_width, heightData, m_isSpherical);

    // Finally, add to the mesh manager
    m_patchData->meshManager->addMesh(m_mesh, m_isSpherical);
}
