///
/// SpaceObject.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 9 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// A notable place in space
///

#pragma once

#ifndef SpaceObject_h__
#define SpaceObject_h__

class SpaceQuadrant;
class SpaceSystem;

class SpaceObject {
public:
    /// Attempt to place this object inside of a quadrant
    /// @param system: The universe...
    /// @param quadrant: Name of quadrant
    /// @return True if a suitable quadrant was found
    bool setQuadrant(SpaceSystem* system, nString quadrant);
private:
    nString _name; ///< Name of this object
    f64v3 _location; ///< Location within the quadrant
    SpaceQuadrant* _quadrant; ///< Parent quadrant
};

#endif // SpaceObject_h__