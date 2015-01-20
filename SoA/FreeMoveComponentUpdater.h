///
/// FreeMoveComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates FreeMoveComponents
///

#pragma once

#ifndef FreeMoveComponentUpdater_h__
#define FreeMoveComponentUpdater_h__

class GameSystem;
class FreeMoveInputComponent;

class FreeMoveComponentUpdater {
public:
    void update(GameSystem* gameSystem);
    static void rotateFromMouse(GameSystem* gameSystem, FreeMoveInputComponent& cmp, float dx, float dy, float speed);
};

#endif // FreeMoveComponentUpdater_h__