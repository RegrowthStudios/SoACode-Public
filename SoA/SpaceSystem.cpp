#include "stdafx.h"
#include "SpaceSystem.h"

SpaceSystem::SpaceSystem() {
    // Add in component tables
    ecs.addComponentTable(SPACE_SYSTEM_CT_OBJECT_NAME, &tblObject);
    ecs.addComponentTable(SPACE_SYSTEM_CT_QUADRANT_NAME, &tblQuadrants);
}
