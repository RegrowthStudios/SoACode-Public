///
/// SphericalVoxelComponentTable.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 15 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component table for SphericalVoxelComponent
///

#pragma once

#ifndef SphericalVoxelComponentTable_h__
#define SphericalVoxelComponentTable_h__

#include <Vorb/ecs/ComponentTable.hpp>
#include "SpaceSystemComponents.h"

class SphericalVoxelComponentTable : public vecs::ComponentTable<SphericalVoxelComponent> {
public:
    virtual void disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) override;
};



#endif // SphericalVoxelComponentTable_h__