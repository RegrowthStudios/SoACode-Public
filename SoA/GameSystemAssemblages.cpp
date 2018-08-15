#include "stdafx.h"
#include "GameSystemAssemblages.h"

#include <Vorb/ecs/ECS.h>

#include "ChunkHandle.h"
#include "ChunkSphereComponentUpdater.h"
#include "GameSystem.h"

vecs::ComponentID GameSystemAssemblages::addFreeMoveInput(GameSystem* gameSystem, vecs::EntityID entity,
                                                                vecs::ComponentID physicsComponent) {
    vecs::ComponentID vmCmpId = gameSystem->addComponent("FreeMove", entity);
    auto& vmCmp = gameSystem->freeMoveInput.get(vmCmpId);
    vmCmp.physicsComponent = physicsComponent;
    return vmCmpId;
}

void GameSystemAssemblages::removeFreeMoveInput(GameSystem* gameSystem, vecs::EntityID entity) {
    gameSystem->deleteComponent("FreeMove", entity);
}

vecs::ComponentID GameSystemAssemblages::addPhysics(GameSystem* gameSystem, vecs::EntityID entity,
                                                     f32 massKg, const f64v3& initialVel,
                                                     vecs::ComponentID spacePositionComponent,
                                                     vecs::ComponentID voxelPositionComponent /*= 0*/) {
    vecs::ComponentID pCmpId = gameSystem->addComponent("Physics", entity);
    auto& pCmp = gameSystem->physics.get(pCmpId);
    pCmp.spacePosition = spacePositionComponent;
    pCmp.voxelPosition = voxelPositionComponent;
    pCmp.velocity = initialVel;
    pCmp.mass = massKg;
    return pCmpId;
}

void GameSystemAssemblages::removePhysics(GameSystem* gameSystem, vecs::EntityID entity) {
    gameSystem->deleteComponent("Physics", entity);
}

vecs::ComponentID GameSystemAssemblages::addSpacePosition(GameSystem* gameSystem, vecs::EntityID entity,
                                                           const f64v3& position, const f64q& orientation,
                                                           vecs::EntityID parentEntity,
                                                           vecs::ComponentID parentGravComponent /* = 0 */,
                                                           vecs::ComponentID parentSphericalTerrainComponent /* = 0 */) {
    vecs::ComponentID spCmpId = gameSystem->addComponent("SpacePosition", entity);
    auto& spCmp = gameSystem->spacePosition.get(spCmpId);
    spCmp.position = position;
    spCmp.orientation = orientation;
    spCmp.parentEntity = parentEntity;
    spCmp.parentGravity = parentGravComponent;
    spCmp.parentSphericalTerrain = parentSphericalTerrainComponent;
    return spCmpId;
}

void GameSystemAssemblages::removeSpacePosition(GameSystem* gameSystem, vecs::EntityID entity) {
    gameSystem->deleteComponent("Space Position", entity);
}

vecs::ComponentID GameSystemAssemblages::addAabbCollidable(GameSystem* gameSystem, vecs::EntityID entity,
                                                                 const f32v3& box VORB_UNUSED, const f32v3& offset VORB_UNUSED) {
    vecs::ComponentID abCmpId = gameSystem->addComponent("AABBCollidable", entity);
    auto& abCmp = gameSystem->aabbCollidable.get(abCmpId);
    abCmp.box = f32v3(1.7f, 3.7f, 1.7f);
    abCmp.offset = f32v3(0.0f);
    return abCmpId;
}

void GameSystemAssemblages::removeAabbCollidable(GameSystem* gameSystem, vecs::EntityID entity) {
    gameSystem->deleteComponent("AABBCollidable", entity);
}

vecs::ComponentID GameSystemAssemblages::addVoxelPosition(GameSystem* gameSystem, vecs::EntityID entity,
                                                                vecs::ComponentID parentVoxelComponent,
                                                                const f64q& orientation,
                                                                const VoxelPosition3D& gridPosition) {
    // We need to transition to the voxels
    vecs::ComponentID vpid = gameSystem->addComponent("VoxelPosition", entity);
    auto& vpcmp = gameSystem->voxelPosition.get(vpid);
    vpcmp.parentVoxel = parentVoxelComponent;
    vpcmp.gridPosition = gridPosition;
    vpcmp.orientation = orientation;
    return vpid;
}

void GameSystemAssemblages::removeVoxelPosition(GameSystem* gameSystem, vecs::EntityID entity) {
    gameSystem->deleteComponent("VoxelPosition", entity);
}

vecs::ComponentID GameSystemAssemblages::addChunkSphere(GameSystem* gameSystem, vecs::EntityID entity,
                                 vecs::ComponentID voxelPosition,
                                 const i32v3& centerPosition,
                                 ui32 radius) {
    vecs::ComponentID id = gameSystem->addComponent("ChunkSphere", entity);
    auto& cmp = gameSystem->chunkSphere.get(id);

    cmp.radius = radius;
    cmp.centerPosition = centerPosition;
    cmp.offset = i32v3(0);
    cmp.voxelPosition = voxelPosition;

    cmp.radius = radius;
    cmp.width = cmp.radius * 2 + 1;
    cmp.layer = cmp.width * cmp.width;
    cmp.size = cmp.layer * cmp.width;
    return id;
}

void GameSystemAssemblages::removeChunkSphere(GameSystem* gameSystem, vecs::EntityID entity) {
    gameSystem->deleteComponent("ChunkSphere", entity);
}

extern vecs::ComponentID GameSystemAssemblages::addFrustumComponent(GameSystem* gameSystem, vecs::EntityID entity,
                                                                     f32 fov, f32 aspectRatio, f32 znear, f32 zfar,
                                                                     vecs::ComponentID spacePosition /* = 0 */,
                                                                     vecs::ComponentID voxelPosition /* = 0 */,
                                                                     vecs::ComponentID head /* = 0 */) {
    vecs::ComponentID fid = gameSystem->addComponent("Frustum", entity);
    auto& fcmp = gameSystem->frustum.get(fid);
    fcmp.frustum.setCamInternals(fov, aspectRatio, znear, zfar);
    fcmp.spacePosition = spacePosition;
    fcmp.voxelPosition = voxelPosition;
    fcmp.head = head;
    return fid;
}

extern void GameSystemAssemblages::removeFrustumComponent(GameSystem* gameSystem, vecs::EntityID entity) {
    gameSystem->deleteComponent("Frustum", entity);
}

extern vecs::ComponentID GameSystemAssemblages::addHeadComponent(GameSystem* gameSystem, vecs::EntityID entity, f64 neckLength) {
    vecs::ComponentID hid = gameSystem->addComponent("Head", entity);
    auto& hcmp = gameSystem->head.get(hid);
    hcmp.neckLength = neckLength;
    return hid;
}

extern void GameSystemAssemblages::removeHeadComponent(GameSystem* gameSystem, vecs::EntityID entity) {
    gameSystem->deleteComponent("Head", entity);
}
