#include "stdafx.h"
#include "GameSystemComponentBuilders.h"

#include <Vorb\io\Keg.h>
#include "GameSystemComponents.h"
#include "Frustum.h"

void AABBCollidableComponentBuilder::load(keg::ReadContext& context, keg::Node node) {
    // Default value
    component.box = f32v3(0.0f);
    component.offset = f32v3(0.0f);

    // Simple read
    keg::parse((ui8*)&component, node, context, &KEG_GLOBAL_TYPE(AabbCollidableComponent));
}
void AABBCollidableComponentBuilder::build(vecs::ECS& ecs, vecs::EntityID eID) {
    ((GameSystem&)ecs).aabbCollidable.get(m_cID) = component;
}

void ParkourInputComponentBuilder::load(keg::ReadContext& context, keg::Node node) {
    // Do nothing
}

void ParkourInputComponentBuilder::build(vecs::ECS& ecs, vecs::EntityID eID) {
    ((GameSystem&)ecs).parkourInput.get(m_cID) = component;
}

void ParkourInputComponentBuilder::postBuild(vecs::ECS& ecs, vecs::EntityID eID) {
    GameSystem& gecs = static_cast<GameSystem&>(ecs);
    auto& cmp = gecs.parkourInput.getFromEntity(eID);
    cmp.physicsComponent = gecs.physics.getComponentID(eID);
    cmp.attributeComponent = gecs.attributes.getComponentID(eID);
    cmp.headComponent = gecs.head.getComponentID(eID);
}

void AttributeComponentBuilder::load(keg::ReadContext& context, keg::Node node) {
    // Simple read
    keg::parse((ui8*)&component, node, context, &KEG_GLOBAL_TYPE(AttributeComponent));
}

void AttributeComponentBuilder::build(vecs::ECS& ecs, vecs::EntityID eID) {
    ((GameSystem&)ecs).attributes.get(m_cID) = component;
}

void SpacePositionComponentBuilder::load(keg::ReadContext& context, keg::Node node) {
    // Default value
    component.position = f64v3(0.0f);
    component.orientation = f64q(0.0, 0.0, 0.0, 1.0);
    component.parentEntity = 0;
    component.parentGravity = 0;
    component.parentSphericalTerrain = 0;
    
    // Simple read
    keg::parse((ui8*)&component, node, context, &KEG_GLOBAL_TYPE(SpacePositionComponent));
}
void SpacePositionComponentBuilder::build(vecs::ECS& ecs, vecs::EntityID eID) {
    ((GameSystem&)ecs).spacePosition.get(m_cID) = component;
}

void VoxelPositionComponentBuilder::load(keg::ReadContext& context, keg::Node node) {
    // Default value
    component.orientation = f64q(0.0, 0.0, 0.0, 1.0);
    VoxelPosition3D pos;
    pos.pos = f64v3(0.0);
    pos.face = FACE_TOP;
    component.gridPosition = pos;
    component.parentVoxel = 0;

    // Simple read
    keg::parse((ui8*)&component, node, context, &KEG_GLOBAL_TYPE(VoxelPositionComponent));
}
void VoxelPositionComponentBuilder::build(vecs::ECS& ecs, vecs::EntityID eID) {
    ((GameSystem&)ecs).voxelPosition.get(m_cID) = component;
}

void PhysicsComponentBuilder::load(keg::ReadContext& context, keg::Node node) {
    // Default value
    component.velocity = f64v3(0.0);
    component.mass = 1.0;
    component.spacePosition = 0;
    component.voxelPosition = 0;

    // Simple read
    keg::parse((ui8*)&component, node, context, &KEG_GLOBAL_TYPE(PhysicsComponent));
}
void PhysicsComponentBuilder::build(vecs::ECS& ecs, vecs::EntityID eID) {
    ((GameSystem&)ecs).physics.get(m_cID) = component;
}

void PhysicsComponentBuilder::postBuild(vecs::ECS& ecs, vecs::EntityID eID) {
    GameSystem& gecs = static_cast<GameSystem&>(ecs);
    auto& cmp = gecs.physics.getFromEntity(eID);
    cmp.voxelPosition = gecs.voxelPosition.getComponentID(eID);
}

void FrustumComponentBuilder::load(keg::ReadContext& context, keg::Node node) {
    // Default value
    Frustum frustum;
    frustum.setCamInternals(1.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    component.frustum = frustum;
    component.spacePosition = 0;
    component.voxelPosition = 0;
    component.head = 0;
    // Simple read
    keg::parse((ui8*)&frustum, node, context, &KEG_GLOBAL_TYPE(FrustumComponent));
    component.frustum = frustum;
}
void FrustumComponentBuilder::build(vecs::ECS& ecs, vecs::EntityID eID) {
    ((GameSystem&)ecs).frustum.get(m_cID) = component;
}

void HeadComponentBuilder::load(keg::ReadContext& context, keg::Node node) {
    // Default value
    component.neckLength = 0.0;
    component.relativeOrientation = f64q(0.0, 0.0, 0.0, 1.0);
    component.relativePosition = f64v3(0.0);

    // Simple read
    keg::parse((ui8*)&component, node, context, &KEG_GLOBAL_TYPE(FrustumComponent));
}
void HeadComponentBuilder::build(vecs::ECS& ecs, vecs::EntityID eID) {
    ((GameSystem&)ecs).head.get(m_cID) = component;
}
void HeadComponentBuilder::postBuild(vecs::ECS& ecs, vecs::EntityID eID) {
    GameSystem& gecs = static_cast<GameSystem&>(ecs);
    auto& cmp = gecs.head.getFromEntity(eID);
    cmp.voxelPosition = gecs.voxelPosition.getComponentID(eID);
}