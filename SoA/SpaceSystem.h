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

#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/ecs/ECS.h>

#include "SpaceComponents.h"

class CTableSpaceObject : public vcore::ComponentTable<SpaceObject> {
public:
    ui64 updates = 0;
};

class CTableSpaceQuadrant : public vcore::ComponentTable<SpaceQuadrant> {
public:
    ui64 updates = 0;
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