///
/// Frustum.h
/// Vorb Engine
///
/// Created by Benjamin Arnold on 24 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// This file implements a frustum used for frustum culling
///

#pragma once

#ifndef Frustum_h__
#define Frustum_h__

class Frustum {
public:
    /// Updates the frustum with the projection and view matrix
    /// @param projectionMatrix: Projection matrix of the camera
    /// @param viewMatrix: View matrix of the camera
    void update(const f64m4& projectionMatrix, const f64m4& viewMatrix);

    /// Checks if a point is in the frustum
    /// @param pos: The position of the point
    /// @return true if it is in the frustum
    bool pointInFrustum(const f32v3& pos);

    /// Checks if a sphere is in the frustum
    /// @param pos: Center position of the sphere
    /// @param radius: Radius of the sphere
    /// @return true if it is in the frustum
    bool sphereInFrustum(const f32v3& pos, float radius);
private:
    double _frustum[6][4] = {}; ///< The actual frustum data
};

#endif // Frustum_h__