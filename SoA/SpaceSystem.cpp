#include "stdafx.h"
#include "SpaceSystem.h"

SpaceSystem::SpaceSystem() : vcore::ECS() {
    // Add in component tables
    addComponentTable(SPACE_SYSTEM_CT_OBJECT_NAME, &tblObject);
    addComponentTable(SPACE_SYSTEM_CT_QUADRANT_NAME, &tblQuadrants);
}
