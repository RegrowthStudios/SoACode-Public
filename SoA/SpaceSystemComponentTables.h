///
/// SpaceSystemComponentTables.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 15 Jan 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// File for component tables that need custom deletion
///

#pragma once

#ifndef SpaceSystemComponentTables_h__
#define SpaceSystemComponentTables_h__

#include <Vorb/ecs/ComponentTable.hpp>
#include "SpaceSystemComponents.h"

class SphericalVoxelComponentTable : public vecs::ComponentTable<SphericalVoxelComponent> {
public:
    virtual void disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) override;
};

class SphericalTerrainComponentTable : public vecs::ComponentTable < SphericalTerrainComponent > {
public:
    virtual void disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) override;
};

class FarTerrainComponentTable : public vecs::ComponentTable < FarTerrainComponent > {
public:
    virtual void disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) override;
};

class OrbitComponentTable : public vecs::ComponentTable < OrbitComponent > {
public:
    virtual void disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) override;
};

#endif // SpaceSystemComponentTables_h__
