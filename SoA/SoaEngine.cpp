#include "stdafx.h"
#include "SoaEngine.h"

#include "SoaState.h"

bool SoaEngine::loadSpaceSystem(OUT SoaState* state, const SpaceSystemLoadData& loadData) {

}

bool SoaEngine::loadGameSystem(OUT SoaState* state, const GameSystemLoadData& loadData) {

}

void SoaEngine::destroyAll(OUT SoaState* state) {
    destroyGameSystem(state);
    destroySpaceSystem(state);
}

void SoaEngine::destroyGameSystem(OUT SoaState* state) {
    
}

void SoaEngine::destroySpaceSystem(OUT SoaState* state) {
    
}
