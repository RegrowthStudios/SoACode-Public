#include "stdafx.h"
#include "Camera.h"
#include "Errors.h"
#include "FullRenderPipeline.h"
#include "SkyboxRenderer.h"
#include "GameRenderStage.h"

FullRenderPipeline::FullRenderPipeline() : 
    _skyboxRenderer(new SkyboxRenderer),
    _gameRenderStage(nullptr),
    _isInitialized(false) {
    // Empty
}

FullRenderPipeline::~FullRenderPipeline() {
    destroy();
}

void FullRenderPipeline::init(vg::GLProgramManager* glProgramManager, vg::TextureCache* textureCache, Camera* chunkCamera, Camera* worldCamera) {
    _isInitialized = true;

    // Game Render Stage
    _gameRenderStage = new GameRenderStage(glProgramManager, textureCache, chunkCamera, worldCamera);
    _stages.push_back(_gameRenderStage);
}

/// Renders the pipeline
void FullRenderPipeline::render() {

    if (!_isInitialized) {
        pError("FullRenderPipeline::render() failed because init() was never called!");
        return;
    }


 
}

/// Frees all resources
void FullRenderPipeline::destroy() {
    delete _skyboxRenderer;
    // Clear all stages
    for (int i = 0; i < _stages.size(); i++) {
        delete _stages[i];
    }
    std::vector<vg::IRenderStage*>().swap(_stages);
}



