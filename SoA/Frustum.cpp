#include "stdafx.h"
#include "Frustum.h"

#include "Constants.h"

void Frustum::Plane::setNormalAndPoint(const f32v3 &normal, const f32v3 &point) {
    this->normal = glm::normalize(normal);
    d = -(glm::dot(this->normal, point));
}

void Frustum::Plane::setCoefficients(float a, float b, float c, float d) {
    //compute the length of the vector
    float l = glm::length(f32v3(a, b, c));
    // normalize the vector
    normal = f32v3(a / l, b / l, c / l);
    // and divide d by th length as well
    this->d = d / l;
}

float Frustum::Plane::distance(const f32v3 &p) const {
    return (d + glm::dot(normal, p));
}

void Frustum::setCamInternals(float fov, float aspectRatio, float znear, float zfar) {
#define RADIANS_PER_DEGREE M_PI / 180.0f
    // store the information
    m_fov = fov;
    m_aspectRatio = aspectRatio;
    m_znear = znear;
    m_zfar = zfar;
  
    // compute width and height of the near and far plane sections
    float tang = (float)tan(RADIANS_PER_DEGREE * fov * 0.5f);
    m_nh = m_znear * tang;
    m_nw = m_nh * m_aspectRatio;
    m_fh = m_zfar  * tang;
    m_fw = m_fh * m_aspectRatio;
}

void Frustum::updateFromWVP(const f32m4& WVP) {
    m_planes[NEARP].setCoefficients(
        WVP[0][2] + WVP[0][3],
        WVP[1][2] + WVP[1][3],
        WVP[2][2] + WVP[2][3],
        WVP[3][2] + WVP[3][3]);
    m_planes[FARP].setCoefficients(
        -WVP[0][2] + WVP[0][3],
        -WVP[1][2] + WVP[1][3],
        -WVP[2][2] + WVP[2][3],
        -WVP[3][2] + WVP[3][3]);
    m_planes[BOTTOMP].setCoefficients(
        WVP[0][1] + WVP[0][3],
        WVP[1][1] + WVP[1][3],
        WVP[2][1] + WVP[2][3],
        WVP[3][1] + WVP[3][3]);
    m_planes[TOPP].setCoefficients(
        -WVP[0][1] + WVP[0][3],
        -WVP[1][1] + WVP[1][3],
        -WVP[2][1] + WVP[2][3],
        -WVP[3][1] + WVP[3][3]);
    m_planes[LEFTP].setCoefficients(
        WVP[0][0] + WVP[0][3],
        WVP[1][0] + WVP[1][3],
        WVP[2][0] + WVP[2][3],
        WVP[3][0] + WVP[3][3]);
    m_planes[RIGHTP].setCoefficients(
        -WVP[0][0] + WVP[0][3],
        -WVP[1][0] + WVP[1][3],
        -WVP[2][0] + WVP[2][3],
        -WVP[3][0] + WVP[3][3]);
}

void Frustum::update(const f32v3& position, const f32v3& dir, const f32v3& up) {

    f32v3 nc, fc, X, Y, Z;

    // Compute the Z axis of camera
    // This axis points in the opposite direction from 
    // the looking direction
    Z = glm::normalize(position - dir);

    // X axis of camera with given "up" vector and Z axis
    X = glm::normalize(glm::cross(up, Z));

    // The real "up" vector is the cross product of Z and X
    Y = glm::cross(Z, X);

    // compute the centers of the near and far planes
    nc = position - Z * m_znear;
    fc = position - Z * m_zfar;

    m_planes[NEARP].setNormalAndPoint(-Z, nc);
    m_planes[FARP].setNormalAndPoint(Z, fc);

    f32v3 aux, normal;

    aux = glm::normalize((nc + Y * m_nh) - position);
    normal = glm::cross(aux, X);
    m_planes[TOPP].setNormalAndPoint(normal, nc + Y * m_nh);

    aux = glm::normalize((nc - Y * m_nh) - position);
    normal = glm::cross(X, aux);
    m_planes[BOTTOMP].setNormalAndPoint(normal, nc - Y * m_nh);

    aux = glm::normalize((nc - X * m_nw) - position);
    normal = glm::cross(aux, Y);
    m_planes[LEFTP].setNormalAndPoint(normal, nc - X * m_nw);

    aux = glm::normalize((nc + X * m_nw) - position);
    normal = glm::cross(Y, aux);
    m_planes[RIGHTP].setNormalAndPoint(normal, nc + X * m_nw);
}

bool Frustum::pointInFrustum(const f32v3& pos) const {
    for (int p = 0; p < 4; p++) { //*************************************** IGNORING FAR AND NEAR CLIPPING PLANE
        if (m_planes[p].distance(pos) <= 0) return false;
    }
    return true;
}

bool Frustum::sphereInFrustum(const f32v3& pos, float radius) const {
    for (int p = 0; p < 4; p++) { //*************************************** IGNORING FAR AND NEAR CLIPPING PLANE
        if (m_planes[p].distance(pos) <= -radius) return false;
    }
    return true;
}
