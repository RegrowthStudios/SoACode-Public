///
/// SpaceComponents.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 10 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef SpaceComponents_h__
#define SpaceComponents_h__

class SpaceObject;

#define SPACE_QUADRANT_BOUNDING_MODIFIER 1.1

class SpaceQuadrant {
public:
    static SpaceQuadrant createDefault();

    nString name; ///< Name of quadrant
    f64v3 location; ///< Location in quadrant space
    f64 radius; ///< Bounding radius of all object found within
    std::unordered_map<nString, SpaceObject*> objects; ///< Space objects found within the quadrant
};

class SpaceObject {
public:
    static SpaceObject createDefault();

    nString name; ///< Name of this object
    f64v3 location; ///< Location within the quadrant
    SpaceQuadrant* quadrant; ///< Parent quadrant
};

#endif // SpaceComponents_h__