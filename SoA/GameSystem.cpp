#include "stdafx.h"
#include "GameSystem.h"

GameSystem::GameSystem() : vecs::ECS() {
    //Add all component tables
    addComponentTable(GAME_SYSTEM_CT_AABBCOLLIDABLE_NAME, &aabbCollidable);
    addComponentTable(GAME_SYSTEM_CT_FREEMOVEINPUT_NAME, &freeMoveInput);
    addComponentTable(GAME_SYSTEM_CT_PARKOURINPUT_NAME, &parkourInput);
    addComponentTable(GAME_SYSTEM_CT_PHYSICS_NAME, &physics);
    addComponentTable(GAME_SYSTEM_CT_SPACEPOSITION_NAME, &spacePosition);
    addComponentTable(GAME_SYSTEM_CT_VOXELPOSITION_NAME, &voxelPosition);
    addComponentTable(GAME_SYSTEM_CT_FRUSTUM_NAME, &frustum);
    addComponentTable(GAME_SYSTEM_CT_HEAD_NAME, &head);
    addComponentTable(GAME_SYSTEM_CT_CHUNKSPHERE_NAME, &chunkSphere);
    addComponentTable(GAME_SYSTEM_CT_ATTRIBUTES_NAME, &attributes);
}

vecs::ComponentID GameSystem::getComponent(const nString& name, vecs::EntityID eID) {
    auto& table = *getComponentTable(name);
    return table.getComponentID(eID);
}