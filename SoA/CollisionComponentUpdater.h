///
/// CollisionComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates collision components
///

#pragma once

#ifndef CollisionComponentUpdater_h__
#define CollisionComponentUpdater_h__

class GameSystem;

class CollisionComponentUpdater {
public:
    /// Updates collision components
    /// @param gameSystem: Game ECS
    void update(GameSystem* gameSystem);
};

#endif // CollisionComponentUpdater_h__
