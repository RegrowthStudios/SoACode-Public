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

typedef vcore::ComponentTable<SpaceObject> CTableSpaceObject;
typedef vcore::ComponentTable<SpaceQuadrant> CTableSpaceQuadrant;

#define SPACE_SYSTEM_CT_OBJECT_NAME "Object"
#define SPACE_SYSTEM_CT_QUADRANT_NAME "Quadrant"

class SpaceSystem {
public:
    SpaceSystem();

    vcore::ECS ecs;
    CTableSpaceObject tblObject;
    CTableSpaceQuadrant tblQuadrants;
};

#endif // SpaceSystem_h__