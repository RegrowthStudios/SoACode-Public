#include "stdafx.h"
#include "SoaController.h"

#include <Vorb/ecs/Entity.h>

#include "App.h"
#include "GameSystemAssemblages.h"
#include "GameSystemUpdater.h"
#include "SoaState.h"
#include "SoaOptions.h"
#include "SoaEngine.h"
#include "OrbitComponentUpdater.h"

SoaController::~SoaController() {
    // Empty
}

void initCreativeInventory(vecs::EntityID eid, SoaState* state) {
    auto& invCmp = state->gameSystem->inventory.getFromEntity(eid);
    const std::vector<Block>& blocks = state->blocks.getBlockList();
    // Skip first two blocks
    for (int i = 2; i < blocks.size(); i++) {
        if (!state->items.hasItem(i)) {
            ItemData d;
            d.blockID = i;
            d.maxCount = UINT_MAX;
            d.name = blocks[i].name;
            d.type = ItemType::BLOCK;
            // Add new item to stack
            ItemStack stack;
            stack.id = state->items.append(d);
            stack.count = UINT_MAX;
            stack.pack = &state->items;
            invCmp.items.push_back(stack);
        }
    }
}

void SoaController::startGame(SoaState* state) {
    // Load game ECS
    SoaEngine::loadGameSystem(state);

    GameSystem* gameSystem = state->gameSystem;
    SpaceSystem* spaceSystem = state->spaceSystem;

    // TODO(Ben): Client only
    ClientState& clientState = state->clientState;
    if (clientState.isNewGame) {
        // Create the player entity and make the initial planet his parent
        if (clientState.startingPlanet) {
            clientState.playerEntity = state->templateLib.build(*gameSystem, "Player");

            auto& spacePos = gameSystem->spacePosition.getFromEntity(clientState.playerEntity);
            spacePos.position = clientState.startSpacePos;
            spacePos.parentEntity = clientState.startingPlanet;
            spacePos.parentGravity = spaceSystem->sphericalGravity.getComponentID(clientState.startingPlanet);
            spacePos.parentSphericalTerrain = spaceSystem->sphericalTerrain.getComponentID(clientState.startingPlanet);
        
            auto& physics = gameSystem->physics.getFromEntity(clientState.playerEntity);
            physics.spacePosition = gameSystem->spacePosition.getComponentID(clientState.playerEntity);
        } else {
            clientState.playerEntity = state->templateLib.build(*gameSystem, "Player");

            auto& spacePos = gameSystem->spacePosition.getFromEntity(clientState.playerEntity);
            spacePos.position = state->clientState.startSpacePos;
        }

        // TODO(Ben): Temporary
        initCreativeInventory(clientState.playerEntity, state);
    } else {
        // TODO(Ben): This
    }
}

f64v3 SoaController::getEntityEyeVoxelPosition(SoaState* state, vecs::EntityID eid) {
    auto& hCmp = state->gameSystem->head.getFromEntity(eid);
    auto& vpCmp = state->gameSystem->voxelPosition.get(hCmp.voxelPosition);
    return vpCmp.gridPosition.pos + hCmp.relativePosition;
}

f64v3 SoaController::getEntityViewVoxelDirection(SoaState* state, vecs::EntityID eid) {
    auto& hCmp = state->gameSystem->head.getFromEntity(eid);
    auto& vpCmp = state->gameSystem->voxelPosition.get(hCmp.voxelPosition);
    f64v3 v(0.0, 0.0, 1.0);
    return v * hCmp.relativeOrientation * vpCmp.orientation;
}