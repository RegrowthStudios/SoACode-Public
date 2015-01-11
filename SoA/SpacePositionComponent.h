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

#include <Vorb/Entity.h>

class SpacePositionComponent {
public:
    void init(const f64v3& pos, const f64q& orient) {
        position = pos;
        orientation = orient;
    }

    f64v3 position = f64v3(0.0);
    f64q orientation;
    vcore::ComponentID voxelPositionComponent = 0;
};

#endif // SpacePositionComponent_h__