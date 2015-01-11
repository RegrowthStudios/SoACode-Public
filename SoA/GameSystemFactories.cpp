#include "stdafx.h"
#include "GameSystemFactories.h"

#include <Vorb/ECS.h>

#include "GameSystem.h"

extern vcore::EntityID GameSystemFactories::createPlayer(OUT GameSystem* gameSystem, const f64v3& spacePosition,
                                                       const f64q& orientation, float massKg, const f64v3& initialVel) {
    vcore::EntityID id = gameSystem->addEntity();

    vcore::ComponentID spCmp = gameSystem->spacePositionCT.add(id);
    vcore::ComponentID pCmp = gameSystem->physicsCT.add(id);
    vcore::ComponentID abCmp = gameSystem->aabbCollidableCT.add(id);
    gameSystem->moveInputCT.add(id);

    gameSystem->spacePositionCT.get(spCmp).init(spacePosition, orientation);
    gameSystem->physicsCT.get(pCmp).init(spCmp, massKg, initialVel);
    gameSystem->aabbCollidableCT.get(abCmp).init(f32v3(1.7f, 3.7f, 1.7f),
                                                 f32v3(0.0f));

    return id;
}

void GameSystemFactories::destroyPlayer(OUT GameSystem* gameSystem, vcore::EntityID playerEntity) {
    gameSystem->deleteEntity(playerEntity);
}