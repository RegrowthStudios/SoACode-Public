#include "stdafx.h"
#include "GameSystemFactories.h"

#include <Vorb/ECS.h>

#include "GameSystem.h"

vcore::EntityID GameSystemFactories::createPlayer(OUT GameSystem* gameSystem, const f64v3& spacePosition,
                                                       const f64q& orientation, float massKg, const f64v3& initialVel) {
    vcore::EntityID id = gameSystem->addEntity();

    vcore::ComponentID spCmpId = addSpacePosition(gameSystem, id, spacePosition, orientation);

    addPhysics(gameSystem, id, massKg, initialVel, spCmpId);

    addAabbCollidable(gameSystem, id, f32v3(1.7f, 3.7f, 1.7f), f32v3(0.0f));

    addFreeMoveInput(gameSystem, id);

    return id;
}

void GameSystemFactories::destroyPlayer(OUT GameSystem* gameSystem, vcore::EntityID playerEntity) {
    gameSystem->deleteEntity(playerEntity);
}

extern vcore::ComponentID GameSystemFactories::addFreeMoveInput(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->freeMoveInputCT.add(entity);
}

extern void GameSystemFactories::removeFreeMoveInput(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->freeMoveInputCT.remove(entity);
}

extern vcore::ComponentID GameSystemFactories::addPhysics(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                          float massKg, const f64v3& initialVel,
                                                          vcore::ComponentID spacePositionComponent,
                                                          OPT vcore::ComponentID voxelPositionComponent /*= 0*/) {
    vcore::ComponentID pCmpId = gameSystem->physicsCT.add(entity);
    auto& pCmp = gameSystem->physicsCT.get(pCmpId);
    pCmp.spacePositionComponent = spacePositionComponent;
    pCmp.voxelPositionComponent = voxelPositionComponent;
    pCmp.velocity = initialVel;
    pCmp.mass = massKg;
    return pCmpId;
}

extern void GameSystemFactories::removePhysics(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->physicsCT.remove(entity);
}

extern vcore::ComponentID GameSystemFactories::addSpacePosition(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                                const f64v3& position, const f64q& orientation) {
    vcore::ComponentID spCmpId = gameSystem->spacePositionCT.add(entity);
    auto& spCmp = gameSystem->spacePositionCT.get(spCmpId);
    spCmp.position = position;
    spCmp.orientation = orientation;
    return spCmpId;
}

extern void GameSystemFactories::removeSpacePosition(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->spacePositionCT.remove(entity);
}

extern vcore::ComponentID GameSystemFactories::addAabbCollidable(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                                 const f32v3& box, const f32v3& offset) {
    vcore::ComponentID abCmpId = gameSystem->aabbCollidableCT.add(entity);
    auto& abCmp = gameSystem->aabbCollidableCT.get(abCmpId);
    abCmp.box = f32v3(1.7f, 3.7f, 1.7f);
    abCmp.offset = f32v3(0.0f);
    return abCmpId;
}

extern void GameSystemFactories::removeAabbCollidable(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->aabbCollidableCT.remove(entity);
}

extern vcore::ComponentID GameSystemFactories::addVoxelPosition(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                                const VoxelPosition& position, const f64q& orientation,
                                                                vvox::VoxelPlanetMapData mapData) {
    // We need to transition to the voxels
    vcore::ComponentID vpid = gameSystem->voxelPositionCT.add(it.first);
    auto& vpcmp = gameSystem->voxelPositionCT.get(vpid);
    
}

extern void GameSystemFactories::removeVoxelPosition(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->voxelPositionCT.remove(entity);
}
