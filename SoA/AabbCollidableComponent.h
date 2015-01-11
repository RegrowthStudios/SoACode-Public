///
/// AabbCollidableComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component for AABB
///

#pragma once

#ifndef AabbCollidableComponent_h__
#define AabbCollidableComponent_h__

class AabbCollidableComponent {
public:
    /// Initializes the component
    /// @param boxDims: Dimensions of the box in blocks
    /// @param boxOffset: Offset of the box in blocks. If 0,
    /// then the -x,-y,-z corner is the origin.
    void init(const f32v3& boxDims, const f32v3& boxOffset) {
        box = boxDims;
        offset = boxOffset;
    }

    f32v3 box = f32v3(0.0f); ///< x, y, z widths in blocks
    f32v3 offset = f32v3(0.0f); ///< x, y, z offsets in blocks
};

#endif // AabbCollidableComponent_h__
