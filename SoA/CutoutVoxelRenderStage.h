#pragma once

#ifndef CutoutVoxelRenderStage_h__
#define CutoutVoxelRenderStage_h__

#include "IRenderStage.h"

class GameRenderParams;
class Camera;
class MeshManager;

class CutoutVoxelRenderStage : public vg::IRenderStage {
public:
    CutoutVoxelRenderStage(Camera* camera, GameRenderParams* gameRenderParams, MeshManager* meshManager);
    ~CutoutVoxelRenderStage();

    virtual void setState(vg::FrameBuffer* frameBuffer = nullptr) override;

    virtual void draw() override;

    virtual bool isVisible() override;

private:
    GameRenderParams* _gameRenderParams;
    MeshManager* _meshManager;
};

#endif // CutoutVoxelRenderStage_h__

