//
// HeadComponentUpdater.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 15 Aug 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// Updater for Head Components.
//

#pragma once

#ifndef HeadComponentUpdater_h__
#define HeadComponentUpdater_h__

#include <Vorb/ecs/Entity.h>

class GameSystem;
class HeadComponentUpdater {
public:
    void update(GameSystem* gameSystem);
    static void rotateFromMouse(GameSystem* gameSystem, vecs::ComponentID cmpID, float dx, float dy, float speed);
};

#endif // HeadComponentUpdater_h__
