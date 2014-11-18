#include "stdafx.h"
#include "SpaceComponents.h"

SpaceQuadrant SpaceQuadrant::createDefault() {
    SpaceQuadrant v = {};
    v.name = "NULL";
    return v;
}

SpaceObject SpaceObject::createDefault() {
    SpaceObject v = {};
    v.name = "NULL";
    v.quadrant = nullptr;
    return v;
}
