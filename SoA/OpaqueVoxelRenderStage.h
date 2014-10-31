#pragma once

#ifndef OpaqueVoxelRenderStage_h__
#define OpaqueVoxelRenderStage_h__

#include "IRenderStage.h"
#include "ChunkRenderer.h"
class GameRenderParams;
class Camera;
class MeshManager;

class OpaqueVoxelRenderStage : public vg::IRenderStage
{
public:
    OpaqueVoxelRenderStage(Camera* camera, GameRenderParams* gameRenderParams, MeshManager* meshManager);
    ~OpaqueVoxelRenderStage();

    virtual void setState(vg::FrameBuffer* frameBuffer = nullptr) override;

    virtual void draw() override;

    virtual bool isVisible() override;

private:
    GameRenderParams* _gameRenderParams;
    MeshManager* _meshManager;
};

#endif // OpaqueVoxelRenderStage_h__