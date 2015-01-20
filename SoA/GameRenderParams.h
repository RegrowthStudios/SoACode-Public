#pragma once

#ifndef GameRenderParams_h__
#define GameRenderParams_h__

#include <Vorb/VorbPreDecl.inl>

class ChunkMesh;
class Camera;

DECL_VG(class, GLProgramManager);

class GameRenderParams {
public:
    void calculateParams(const f64v3& worldCameraPos,
                         const Camera* ChunkCamera, 
                         const std::vector<ChunkMesh*>* ChunkMeshes,
                         bool IsUnderwater);

    f32v3 sunlightDirection;
    f32v3 sunlightColor;  
    float sunlightIntensity;
    f32v3 fogColor;
    float fogEnd;
    float fogStart;
    float lightActive;
    const Camera* chunkCamera;
    const std::vector <ChunkMesh *>* chunkMeshes;
    bool isUnderwater;
    const vg::GLProgramManager* glProgramManager = nullptr;
private:
    void calculateFog(float theta, bool isUnderwater);
    void calculateSunlight(float theta);
};

#endif // GameRenderParams_h__