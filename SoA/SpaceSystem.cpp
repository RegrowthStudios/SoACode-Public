#include "stdafx.h"
#include "SpaceSystem.h"

SpaceSystem::SpaceSystem() :
    tblObject(SpaceObject::createDefault()),
    tblQuadrants(SpaceQuadrant::createDefault()) {
    // Add in component tables
    ecs.addComponent(SPACE_SYSTEM_CT_OBJECT_NAME, &tblObject);
    ecs.addComponent(SPACE_SYSTEM_CT_QUADRANT_NAME, &tblQuadrants);
}
