#include "stdafx.h"
#include "GameSystemAssemblages.h"

#include <Vorb/ecs/ECS.h>

#include "GameSystem.h"

vcore::EntityID GameSystemAssemblages::createPlayer(OUT GameSystem* gameSystem, const f64v3& spacePosition,
                                                    const f64q& orientation, f32 massKg, const f64v3& initialVel,
                                                    f32 fov, f32 aspectRatio, f32 znear, f32 zfar) {
    vcore::EntityID id = gameSystem->addEntity();

    vcore::ComponentID spCmpId = addSpacePosition(gameSystem, id, spacePosition, orientation);

    vcore::ComponentID pyCmpId = addPhysics(gameSystem, id, massKg, initialVel, spCmpId);

    addAabbCollidable(gameSystem, id, f32v3(1.7f, 3.7f, 1.7f), f32v3(0.0f));

    addFreeMoveInput(gameSystem, id, pyCmpId);

    vcore::ComponentID hCmpId = addHeadComponent(gameSystem, id, 0.1f);

    addFrustumComponent(gameSystem, id, fov, aspectRatio, znear, zfar, spCmpId, 0, hCmpId);

    return id;
}

void GameSystemAssemblages::destroyPlayer(OUT GameSystem* gameSystem, vcore::EntityID playerEntity) {
    gameSystem->deleteEntity(playerEntity);
}

vcore::ComponentID GameSystemAssemblages::addFreeMoveInput(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                                vcore::ComponentID physicsComponent) {
    vcore::ComponentID vmCmpId = gameSystem->addComponent("FreeMove", entity);
    auto& vmCmp = gameSystem->freeMoveInput.get(vmCmpId);
    vmCmp.physicsComponent = physicsComponent;
    return vmCmpId;
}

void GameSystemAssemblages::removeFreeMoveInput(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->deleteComponent("FreeMove", entity);
}

vcore::ComponentID GameSystemAssemblages::addPhysics(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                     f32 massKg, const f64v3& initialVel,
                                                     vcore::ComponentID spacePositionComponent,
                                                     OPT vcore::ComponentID voxelPositionComponent /*= 0*/) {
    vcore::ComponentID pCmpId = gameSystem->addComponent("Physics", entity);
    auto& pCmp = gameSystem->physics.get(pCmpId);
    pCmp.spacePositionComponent = spacePositionComponent;
    pCmp.voxelPositionComponent = voxelPositionComponent;
    pCmp.velocity = initialVel;
    pCmp.mass = massKg;
    return pCmpId;
}

void GameSystemAssemblages::removePhysics(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->deleteComponent("Physics", entity);
}

vcore::ComponentID GameSystemAssemblages::addSpacePosition(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                                const f64v3& position, const f64q& orientation) {
    vcore::ComponentID spCmpId = gameSystem->addComponent("SpacePosition", entity);
    auto& spCmp = gameSystem->spacePosition.get(spCmpId);
    spCmp.position = position;
    spCmp.orientation = orientation;
    return spCmpId;
}

void GameSystemAssemblages::removeSpacePosition(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->deleteComponent("Space Position", entity);
}

vcore::ComponentID GameSystemAssemblages::addAabbCollidable(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                                 const f32v3& box, const f32v3& offset) {
    vcore::ComponentID abCmpId = gameSystem->addComponent("AABBCollidable", entity);
    auto& abCmp = gameSystem->aabbCollidable.get(abCmpId);
    abCmp.box = f32v3(1.7f, 3.7f, 1.7f);
    abCmp.offset = f32v3(0.0f);
    return abCmpId;
}

void GameSystemAssemblages::removeAabbCollidable(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->deleteComponent("AABBCollidable", entity);
}

vcore::ComponentID GameSystemAssemblages::addVoxelPosition(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                                vcore::ComponentID parentVoxelComponent,
                                                                const f64q& orientation,
                                                                const VoxelPosition3D& gridPosition) {
    // We need to transition to the voxels
    vcore::ComponentID vpid = gameSystem->addComponent("VoxelPosition", entity);
    auto& vpcmp = gameSystem->voxelPosition.get(vpid);
    vpcmp.parentVoxelComponent = parentVoxelComponent;
    vpcmp.gridPosition = gridPosition;
    vpcmp.orientation = orientation;
    return vpid;
}

void GameSystemAssemblages::removeVoxelPosition(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->deleteComponent("VoxelPosition", entity);
}

extern vcore::ComponentID GameSystemAssemblages::addFrustumComponent(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                                     f32 fov, f32 aspectRatio, f32 znear, f32 zfar,
                                                                     vcore::ComponentID spacePosition /* = 0 */,
                                                                     vcore::ComponentID voxelPosition /* = 0 */,
                                                                     vcore::ComponentID head /* = 0 */) {
    vcore::ComponentID fid = gameSystem->addComponent("Frustum", entity);
    auto& fcmp = gameSystem->frustum.get(fid);
    fcmp.frustum.setCamInternals(fov, aspectRatio, znear, zfar);
    fcmp.spacePositionComponent = spacePosition;
    fcmp.voxelPositionComponent = voxelPosition;
    fcmp.headComponent = head;
    return fid;
}

extern void GameSystemAssemblages::removeFrustumComponent(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->deleteComponent("Frustum", entity);
}

extern vcore::ComponentID GameSystemAssemblages::addHeadComponent(OUT GameSystem* gameSystem, vcore::EntityID entity, f64 neckLength) {
    vcore::ComponentID hid = gameSystem->addComponent("Head", entity);
    auto& hcmp = gameSystem->head.get(hid);
    hcmp.neckLength = neckLength;
    return hid;
}

extern void GameSystemAssemblages::removeHeadComponent(OUT GameSystem* gameSystem, vcore::EntityID entity) {
    gameSystem->deleteComponent("Head", entity);
}
