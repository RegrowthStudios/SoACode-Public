#include "stdafx.h"
#include "GameRenderParams.h"

#include <Vorb/utils.h>

#include "Camera.h"
#include "GameManager.h"

void GameRenderParams::calculateParams(const f64v3& worldCameraPos,
                                       const Camera* ChunkCamera,
                                       const VoxelPosition3D& voxPosition,
                                       f64 voxelWorldRadius,
                                       ChunkMeshManager* ChunkMeshmanager,
                                       BlockPack* blocks,
                                       BlockTexturePack* blockTexturePack,
                                       bool IsUnderwater) {
    chunkCamera = ChunkCamera;
    isUnderwater = IsUnderwater;
    this->blocks = blocks;
    this->blockTexturePack = blockTexturePack;

    chunkMeshmanager = ChunkMeshmanager;

    // Calculate fog
    sunlightDirection = glm::normalize(f32v3(0.7, 1.0, 0.5));
    
   // float theta = (f32)glm::dot(VoxelSpaceConversions::voxelToWorld(voxPosition, voxelWorldRadius), f64v3(lightPos));
    f32 theta = 1.0f;
    calculateFog(theta, isUnderwater);
    calculateSunlight(theta);
    sunlightColor = f32v3(1.0f);
}

void GameRenderParams::calculateFog(float theta, bool isUnderwater) {
    
#define FOG_THETA_MULT 100.0f
#define FOG_THETA_OFFSET 50.0f
    glm::mat4 VP;
    //********************************* TODO: PRECOMPILED HEADERS for compilation speed?
    float fogTheta = glm::clamp(theta, 0.0f, 1.0f);
    fogStart = 0;
    if (isUnderwater) {
        float underwaterColor = fogTheta / 2.0f;
        fogEnd = FOG_THETA_OFFSET + fogTheta * FOG_THETA_MULT;
        fogColor[0] = underwaterColor;
        fogColor[1] = underwaterColor;
        fogColor[2] = underwaterColor;
    } else {
        fogEnd = 100000;
        fogColor[0] = 1.0;
        fogColor[1] = 1.0;
        fogColor[2] = 1.0;
    }
}

void GameRenderParams::calculateSunlight(float theta) {
    // Calculate sunlight
    #define AMB_MULT 0.76f
    #define AMB_OFFSET 0.02f
    #define MIN_THETA 0.01f
    #define THETA_MULT 8.0f
    #define SUN_COLOR_MAP_HEIGHT 64.0f
    #define SUN_THETA_OFF 0.06f

    /*   sunlightDirection = glm::normalize(f64v3(f64m4(glm::inverse(GameManager::player->worldRotationMatrix)) *
           f64m4(GameManager::planet->invRotationMatrix) * f64v4(1.0, 0.0, 0.0, 1.0)));
           */
    float sunTheta = MAX(0.0f, theta + SUN_THETA_OFF);
    if (sunTheta > 1) sunTheta = 1;
    sunlightIntensity = sunTheta * AMB_MULT + AMB_OFFSET;
    if (sunlightIntensity > 1.0f) sunlightIntensity = 1.0f;
    float diffVal = 1.0f - sunlightIntensity;

    if (theta < MIN_THETA) {
        diffVal += (theta - MIN_THETA) * THETA_MULT;
        if (diffVal < 0.0f) diffVal = 0.0f;
    }

    int sunHeight = (int)(theta * SUN_COLOR_MAP_HEIGHT);
    if (theta < 0) {
        sunHeight = 0;
    }
    sunlightColor = f32v3(1.0f) * diffVal;
}