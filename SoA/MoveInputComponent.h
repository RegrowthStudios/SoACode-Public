///
/// MoveInputComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component for movement input
///

#pragma once

#ifndef MoveInputComponent_h__
#define MoveInputComponent_h__

class MoveInputComponent {
public:
    bool isWPressed = false; ///< True when moving forward
    bool isSPressed = false; ///< True when moving backward
    bool isAPressed = false; ///< True when moving left
    bool isDPressed = false; ///< True when moving right
    bool isJumping = false; ///< True when attempting to jump
    bool isCrouching = false; ///< True when attempting to crouch
    bool isParkouring = false; ///< True when parkouring
    bool isSprinting = false; ///< True when sprinting
};

#endif // MoveInputComponent_h__