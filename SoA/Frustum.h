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

    enum Planes {
        RIGHTP = 0, LEFTP, BOTTOMP,
        TOPP, FARP, NEARP
    };

    class Plane {
    public:
        void setNormalAndPoint(const f32v3 &normal, const f32v3 &point);
        void setCoefficients(float a, float b, float c, float d);
        float distance(const f32v3 &p) const;

        f32v3 normal;
        float d;
    };

    /// Sets internal camera properties. Needed for update()
    void setCamInternals(float fov, float aspectRatio, float znear, float zfar);

    /// Updates the frustum with the projection and view matrix
    /// @param MVP: World-View-Projection matrix of camera
    void updateFromWVP(const f32m4& WVP);

    /// Updates the frustum with the geometric information
    void update(const f32v3& position, const f32v3& dir, const f32v3& up);

    /// Checks if a point is in the frustum
    /// @param pos: The position of the point
    /// @return true if it is in the frustum
    bool pointInFrustum(const f32v3& pos) const;

    /// Checks if a sphere is in the frustum
    /// @param pos: Center position of the sphere
    /// @param radius: Radius of the sphere
    /// @return true if it is in the frustum
    bool sphereInFrustum(const f32v3& pos, float radius) const;
private:
    float m_fov = 0.0f; ///< Vertical field of view in degrees
    float m_aspectRatio = 0.0f; ///< Screen aspect ratio
    float m_znear = 0.0f; ///< Near clipping plane
    float m_zfar = 0.0f; ///< Far clipping plane
    float m_nh = 0.0f; ///< Near plane height
    float m_nw = 0.0f; ///< Near plane Width
    float m_fh = 0.0f; ///< Far plane height
    float m_fw = 0.0f; ///< Far plane Width
    Plane m_planes[6]; ///< The actual frustum data
};

#endif // Frustum_h__