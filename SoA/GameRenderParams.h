#pragma once

#ifndef GameRenderParams_h__
#define GameRenderParams_h__

#include <Vorb/VorbPreDecl.inl>
#include "VoxelSpaceConversions.h"

class ChunkMesh;
class Camera;
class ChunkMeshManager;
class BlockPack;
class BlockTexturePack;

class GameRenderParams {
public:
    void calculateParams(const f64v3& worldCameraPos,
                         const Camera* ChunkCamera,
                         const VoxelPosition3D& voxPosition,
                         f64 voxelWorldRadius,
                         ChunkMeshManager* ChunkMeshmanager,
                         BlockPack* blocks,
                         BlockTexturePack* blockTexturePack,
                         bool IsUnderwater);

    f32v3 sunlightDirection;
    f32v3 sunlightColor;  
    float sunlightIntensity;
    f32v3 fogColor;
    float fogEnd;
    float fogStart;
    float lightActive;
    const Camera* chunkCamera;
    ChunkMeshManager* chunkMeshmanager;
    BlockPack* blocks;
    BlockTexturePack* blockTexturePack;
    bool isUnderwater;
private:
    void calculateFog(float theta, bool isUnderwater);
    void calculateSunlight(float theta);
};

#endif // GameRenderParams_h__