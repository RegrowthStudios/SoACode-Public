#include "stdafx.h"
#include "FarTerrainComponentUpdater.h"


#include "SpaceSystem.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainGpuGenerator.h"
#include "SphericalTerrainCpuGenerator.h"
#include "VoxelCoordinateSpaces.h"
#include "FarTerrainPatch.h"
#include "soaUtils.h"

void FarTerrainComponentUpdater::update(SpaceSystem* spaceSystem, const f64v3& cameraPos) {
    for (auto& it : spaceSystem->m_farTerrainCT) {
        if (!it.second.gpuGenerator) break;

        FarTerrainComponent& cmp = it.second;
        /// Calculate camera distance
        f64v3 relativeCameraPos = cameraPos;
        f64 distance = glm::length(relativeCameraPos);

        printVec("Position: ", cameraPos);

        if (distance <= LOAD_DIST) {
            // In range, allocate if needed
            if (!cmp.patches) {
                initPatches(cmp, cameraPos);
            } else {
                // Check to see if the grid should shift
                const f64& patchWidth = (cmp.sphericalTerrainData->radius * 2.000) / FT_PATCH_ROW;
                i32v2 newCenter(fastFloor(cameraPos.x / patchWidth), fastFloor(cameraPos.z / patchWidth));
                checkGridShift(cmp, newCenter);
            }

            // Update patches
            for (int i = 0; i < FT_TOTAL_PATCHES; i++) {
                cmp.patches[i].update(relativeCameraPos);
            }
        } else {
            // Out of range, delete everything
            if (cmp.patches) {
                delete[] cmp.patches;
                cmp.patches = nullptr;
            }
        }
    }
}

void FarTerrainComponentUpdater::glUpdate(SpaceSystem* spaceSystem) {
    for (auto& it : spaceSystem->m_farTerrainCT) {
        if (it.second.gpuGenerator) it.second.gpuGenerator->update();
    }
}

void FarTerrainComponentUpdater::initPatches(FarTerrainComponent& cmp, const f64v3& cameraPos) {
    const f64& patchWidth = (cmp.sphericalTerrainData->radius * 2.000) / FT_PATCH_ROW;

    // Allocate top level patches
    cmp.patches = new FarTerrainPatch[FT_TOTAL_PATCHES];

    cmp.center.x = fastFloor(cameraPos.x / patchWidth);
    cmp.center.y = fastFloor(cameraPos.z / patchWidth);

    int centerX = FT_PATCH_ROW / 2 - cmp.center.x;
    int centerZ = FT_PATCH_ROW / 2 - cmp.center.y;

    f64v2 gridPos;
    int index = 0;

    // Init all the top level patches for each of the 6 grids

    for (int z = 0; z < FT_PATCH_ROW; z++) {
        for (int x = 0; x < FT_PATCH_ROW; x++) {
            FarTerrainPatch& p = cmp.patches[index++];
            gridPos.x = (x - centerX) * patchWidth;
            gridPos.y = (z - centerZ) * patchWidth;
            p.init(gridPos, cmp.face,
                   0, cmp.sphericalTerrainData, patchWidth,
                   cmp.rpcDispatcher);
        }
    }
}

void FarTerrainComponentUpdater::checkGridShift(FarTerrainComponent& cmp, const i32v2& newCenter) {
    f64v2 gridPos;
    const f64& patchWidth = (cmp.sphericalTerrainData->radius * 2.000) / FT_PATCH_ROW;
    // X shift
    if (newCenter.x > cmp.center.x) { // +X shift
        std::cout << "+X Shift\n";
        // Shift center
        cmp.center.x++;
        // Destroy and re-init the leftmost column of chunks
        i32 gx = cmp.origin.x;
        for (i32 z = 0; z < FT_PATCH_ROW; z++) {
            i32 gz = (cmp.origin.y + z) % FT_PATCH_ROW;
            FarTerrainPatch& p = cmp.patches[gz * FT_PATCH_ROW + gx];
            p.destroy();
            gridPos.x = (cmp.center.x + FT_PATCH_ROW / 2 - 1) * patchWidth;
            gridPos.y = (cmp.center.y + z - FT_PATCH_ROW / 2) * patchWidth;
            p.init(gridPos, cmp.face,
                   0, cmp.sphericalTerrainData, patchWidth,
                   cmp.rpcDispatcher);
        }
        // Shift origin
        cmp.origin.x++;
        // Origin is % FT_PATCH_ROW
        if (cmp.origin.x >= FT_PATCH_ROW) cmp.origin.x = 0;
        return;
    } else if (newCenter.x < cmp.center.x) { // -X shift
        std::cout << "-X Shift\n";
        // Shift center
        cmp.center.x--;
        // Destroy and re-init the rightmost column of chunks
        i32 gx = (cmp.origin.x + FT_PATCH_ROW - 1) % FT_PATCH_ROW;
        for (i32 z = 0; z < FT_PATCH_ROW; z++) {
            i32 gz = (cmp.origin.y + z) % FT_PATCH_ROW;
            FarTerrainPatch& p = cmp.patches[gz * FT_PATCH_ROW + gx];
            p.destroy();
            gridPos.x = (cmp.center.x - FT_PATCH_ROW / 2) * patchWidth;
            gridPos.y = (cmp.center.y + z - FT_PATCH_ROW / 2) * patchWidth;
            p.init(gridPos, cmp.face,
                   0, cmp.sphericalTerrainData, patchWidth,
                   cmp.rpcDispatcher);
        }
        // Shift origin
        cmp.origin.x--;
        // Origin is % FT_PATCH_ROW
        if (cmp.origin.x < 0) cmp.origin.x = FT_PATCH_ROW - 1;
        return;
    }

    // Z shift
    if (newCenter.y > cmp.center.y) { // +Z shift
        std::cout << "+Z Shift\n";
        // Shift center
        cmp.center.y++;
        // Destroy and re-init the leftmost column of chunks
        i32 gz = cmp.origin.y;
        for (int x = 0; x < FT_PATCH_ROW; x++) {
            int gx = (cmp.origin.x + x) % FT_PATCH_ROW;
            FarTerrainPatch& p = cmp.patches[gz * FT_PATCH_ROW + gx];
            p.destroy();
            gridPos.x = (cmp.center.x + x - FT_PATCH_ROW / 2) * patchWidth;
            gridPos.y = (cmp.center.y + FT_PATCH_ROW / 2 - 1) * patchWidth;
            p.init(gridPos, cmp.face,
                   0, cmp.sphericalTerrainData, patchWidth,
                   cmp.rpcDispatcher);
        }
        // Shift origin
        cmp.origin.y++;
        // Origin is % FT_PATCH_ROW
        if (cmp.origin.y >= FT_PATCH_ROW) cmp.origin.y = 0;
    } else if (newCenter.y < cmp.center.y) { // -Z shift
        std::cout << "-Z Shift\n";
        // Shift center
        cmp.center.y--;
        // Destroy and re-init the rightmost column of chunks
        i32 gz = (cmp.origin.y + FT_PATCH_ROW - 1) % FT_PATCH_ROW;
        for (i32 x = 0; x < FT_PATCH_ROW; x++) {
            int gx = (cmp.origin.x + x) % FT_PATCH_ROW;
            FarTerrainPatch& p = cmp.patches[gz * FT_PATCH_ROW + gx];
            p.destroy();
            gridPos.x = (cmp.center.x + x - FT_PATCH_ROW / 2) * patchWidth;
            gridPos.y = (cmp.center.y - FT_PATCH_ROW / 2) * patchWidth;
            p.init(gridPos, cmp.face,
                   0, cmp.sphericalTerrainData, patchWidth,
                   cmp.rpcDispatcher);
        }
        // Shift origin
        cmp.origin.y--;
        // Origin is % FT_PATCH_ROW
        if (cmp.origin.y < 0) cmp.origin.y = FT_PATCH_ROW - 1;
    }
}
