#include "stdafx.h"
#include "GameSystemFactories.h"

#include <Vorb/ECS.h>

#include "GameSystem.h"

vcore::EntityID GameSystemFactories::createPlayer(OUT GameSystem* gameSystem, const f64v3& spacePosition,
                                                       const f64q& orientation, float massKg, const f64v3& initialVel) {
    vcore::EntityID id = gameSystem->addEntity();

    vcore::ComponentID spCmpId = gameSystem->spacePositionCT.add(id);
    auto& spCmp = gameSystem->spacePositionCT.get(spCmpId);
    spCmp.position = spacePosition;
    spCmp.orientation = orientation;

    vcore::ComponentID pCmpId = gameSystem->physicsCT.add(id);
    auto& pCmp = gameSystem->physicsCT.get(pCmpId);
    pCmp.spacePositionComponent = spCmpId;
    pCmp.velocity = initialVel;
    pCmp.mass = massKg;

    vcore::ComponentID abCmpId = gameSystem->aabbCollidableCT.add(id);
    auto& abCmp = gameSystem->aabbCollidableCT.get(abCmpId);
    abCmp.box = f32v3(1.7f, 3.7f, 1.7f);
    abCmp.offset = f32v3(0.0f);

    gameSystem->moveInputCT.add(id);

    return id;
}

void GameSystemFactories::destroyPlayer(OUT GameSystem* gameSystem, vcore::EntityID playerEntity) {
    gameSystem->deleteEntity(playerEntity);
}