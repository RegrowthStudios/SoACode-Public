#include "stdafx.h"
#include "SoaEngine.h"

#include "SoaState.h"
#include "GLProgramManager.h"
#include "DebugRenderer.h"
#include "MeshManager.h"

bool SoaEngine::initState(OUT SoaState* state) {
    state->glProgramManager = std::make_unique<vg::GLProgramManager>();
    state->debugRenderer = std::make_unique<DebugRenderer>(state->glProgramManager.get());
    state->meshManager = std::make_unique<MeshManager>(state->glProgramManager.get());
    return true;
}

bool SoaEngine::loadSpaceSystem(OUT SoaState* state, const SpaceSystemLoadData& loadData) {
    state->spaceSystem.init(state->glProgramManager.get());
    state->spaceSystem.addSolarSystem(loadData.filePath);
    return true;
}

bool SoaEngine::loadGameSystem(OUT SoaState* state, const GameSystemLoadData& loadData) {
    return true;
}

void SoaEngine::destroyAll(OUT SoaState* state) {
    destroyGameSystem(state);
    destroySpaceSystem(state);
}

void SoaEngine::destroyGameSystem(OUT SoaState* state) {
    
}

void SoaEngine::destroySpaceSystem(OUT SoaState* state) {
    
}
