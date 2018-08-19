//
// AABBCollidableComponentUpdater.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 30 Aug 2015
// Copyright 2014 Regrowth Studios
// MIT License
//
// Summary:
// Updater for AABB components.
//

#pragma once

#ifndef AABBCollidableComponentUpdater_h__
#define AABBCollidableComponentUpdater_h__

#include <Vorb/ecs/Entity.h>

class GameSystem;
class SpaceSystem;
struct AabbCollidableComponent;

class AABBCollidableComponentUpdater {
public:
    void update(GameSystem* gameSystem, SpaceSystem* spaceSystem);

private:
    void collideWithVoxels(AabbCollidableComponent& cmp, GameSystem* gameSystem, SpaceSystem* spaceSystem);
};

#endif // AABBCollidableComponentUpdater_h__
