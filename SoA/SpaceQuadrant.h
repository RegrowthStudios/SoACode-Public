///
/// SpaceQuadrant.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 9 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// A large "closed-system" region of space
///

#pragma once

#ifndef SpaceQuadrant_h__
#define SpaceQuadrant_h__

class SpaceObject;

#define SPACE_QUADRANT_BOUNDING_MODIFIER 1.1

class SpaceQuadrant {
public:

private:
    nString _name; ///< Name of quadrant
    f64v3 _location; ///< Location in quadrant space
    f64 _radius; ///< Bounding radius of all object found within
    std::unordered_map<nString, SpaceObject*> _objects; ///< Space objects found within the quadrant
};

#endif // SpaceQuadrant_h__