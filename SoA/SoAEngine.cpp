#include "stdafx.h"
#include "SoAEngine.h"

#include "SoAState.h"

bool SoAEngine::loadSpaceSystem(OUT SoAState* state, const SpaceSystemLoadData& loadData) {

}

bool SoAEngine::loadGameSystem(OUT SoAState* state, const GameSystemLoadData& loadData) {

}

void SoAEngine::destroyAll(OUT SoAState* state) {
    destroyGameSystem(state);
    destroySpaceSystem(state);
}

void SoAEngine::destroyGameSystem(OUT SoAState* state) {
    
}

void SoAEngine::destroySpaceSystem(OUT SoAState* state) {
    
}
