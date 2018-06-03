#pragma once

#ifndef GameSystemComponentBuilders_h__
#define GameSystemComponentBuilders_h__

#include <Vorb\ecs\ECS.h>

#include "GameSystem.h"
#include "GameSystemComponents.h"
#include "ECSTemplates.h"

class AABBCollidableComponentBuilder: public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;
    virtual void postBuild(vecs::ECS& ecs, vecs::EntityID eID) override;

    AabbCollidableComponent component;
};

class ParkourInputComponentBuilder: public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;
    virtual void postBuild(vecs::ECS& ecs, vecs::EntityID eID) override;
    ParkourInputComponent component;
};

class AttributeComponentBuilder : public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;

    AttributeComponent component;
};

/* TODO(BEN): Implement these component builders
class FreeMoveInputComponentBuilder: public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;

    FreeMoveInputComponent component;
};
*/

class SpacePositionComponentBuilder: public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;

    SpacePositionComponent component;
};

class VoxelPositionComponentBuilder: public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;

    VoxelPositionComponent component;
};

class PhysicsComponentBuilder: public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;
    virtual void postBuild(vecs::ECS& ecs, vecs::EntityID eID) override;

    PhysicsComponent component;
};

class FrustumComponentBuilder: public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;

    FrustumComponent component;
};

class HeadComponentBuilder: public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;
    virtual void postBuild(vecs::ECS& ecs, vecs::EntityID eID) override;

    HeadComponent component;
};

class InventoryComponentBuilder : public ECSComponentBuilder {
public:
    virtual void load(keg::ReadContext& reader, keg::Node node) override;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) override;

    InventoryComponent component;
};

#endif //GameSystemComponentBuilders_h__