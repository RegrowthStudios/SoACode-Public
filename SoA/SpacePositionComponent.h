///
/// SpacePositionComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Position in space
///

#pragma once

#ifndef SpacePositionComponent_h__
#define SpacePositionComponent_h__

class SpacePositionComponent {
public:
    f64v3 position = f64v3(0.0);
    f64q orientation;
};

#endif // SpacePositionComponent_h__