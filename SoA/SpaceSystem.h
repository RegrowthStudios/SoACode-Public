///
/// SpaceSystem.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 9 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Let there be light!
///

#pragma once

#ifndef SpaceSystem_h__
#define SpaceSystem_h__

#include "ComponentTable.hpp"
#include "ECS.h"
#include "SpaceComponents.h"

class CTableSpaceObject : public vcore::ComponentTable<SpaceObject> {
public:
    virtual void update(const vcore::EntityID& eID, const vcore::ComponentID& cID, SpaceObject& component) override {
        printf("For entity <0x%016llX> - updating <0x%016llX> at SpaceObject\n", eID, cID);
    }
};

class CTableSpaceQuadrant : public vcore::ComponentTable<SpaceQuadrant> {
public:
    virtual void update(const vcore::EntityID& eID, const vcore::ComponentID& cID, SpaceQuadrant& component) override {
        printf("For entity <0x%016llX> - updating <0x%016llX> at SpaceQuadrant\n", eID, cID);
    }
};

#define SPACE_SYSTEM_CT_OBJECT_NAME "Object"
#define SPACE_SYSTEM_CT_QUADRANT_NAME "Quadrant"

class SpaceSystem : public vcore::ECS {
public:
    SpaceSystem();

    CTableSpaceObject tblObject;
    CTableSpaceQuadrant tblQuadrants;
};

#endif // SpaceSystem_h__