#include "stdafx.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL/SDL.h>
#include <Vorb/utils.h>

#include "Options.h"

Camera::Camera() {
    // Empty
}

void Camera::init(float aspectRatio) {
    m_aspectRatio = aspectRatio;
}

void Camera::offsetPosition(glm::dvec3 offset) {
    m_position += offset;
    m_viewChanged = 1;
}

void Camera::offsetPosition(glm::vec3 offset) {
    m_position += offset;
    m_viewChanged = 1;

}

void Camera::update() {
    if (m_fieldOfView != graphicsOptions.fov) {
        setFieldOfView(graphicsOptions.fov);
    }

    bool updateFrustum = false;
    if (m_viewChanged) {
        updateView();
        m_viewChanged = false;
        updateFrustum = true;
    }
    if (m_projectionChanged) {
        updateProjection();
        m_projectionChanged = false;
        updateFrustum = true;
    }

    if (updateFrustum) {
        m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
        m_frustum.updateFromWVP(m_viewProjectionMatrix);
    }
}

void Camera::updateView() {
    m_viewMatrix = glm::lookAt(glm::vec3(0.0f), m_direction, m_up);
}

void Camera::updateProjection() {
    m_frustum.setCamInternals(m_fieldOfView, m_aspectRatio, m_zNear, m_zFar);
    m_projectionMatrix = glm::perspective(m_fieldOfView, m_aspectRatio, m_zNear, m_zFar);
}

void Camera::applyRotation(const f32q& rot) {
    m_direction = rot * m_direction;
    m_right = rot * m_right;
    m_up = glm::normalize(glm::cross(m_right, m_direction));
    m_viewChanged = true;
}

void Camera::rotateFromMouse(float dx, float dy, float speed) {
    f32q upQuat = glm::angleAxis(dy * speed, m_right);
    f32q rightQuat = glm::angleAxis(dx * speed, m_up);
 
    applyRotation(upQuat * rightQuat);
}

void Camera::rollFromMouse(float dx, float speed) {
    f32q frontQuat = glm::angleAxis(dx * speed, m_direction);

    applyRotation(frontQuat);
}

void Camera::setOrientation(const f64q& orientation) {
    m_direction = orientation * f64v3(0.0, 0.0, 1.0);
    m_right = orientation * f64v3(1.0, 0.0, 0.0);
    m_up = orientation * f64v3(0.0, 1.0, 0.0);
    m_viewChanged = true;
}

f32v3 Camera::worldToScreenPoint(const f32v3& worldPoint) const {
    // Transform world to clipping coordinates
    f32v4 clipPoint = m_viewProjectionMatrix * f32v4(worldPoint, 1.0f);
    clipPoint.x /= clipPoint.w;
    clipPoint.y /= clipPoint.w;
    clipPoint.z /= clipPoint.w;
    return f32v3((clipPoint.x + 1.0) / 2.0f,
                 (1.0 - clipPoint.y) / 2.0f,
                 clipPoint.z);
}

f32v3 Camera::worldToScreenPoint(const f64v3& worldPoint) const {
    // Transform world to clipping coordinates
    f64v4 clipPoint = f64m4(m_viewProjectionMatrix) * f64v4(worldPoint, 1.0);
    clipPoint.x /= clipPoint.w;
    clipPoint.y /= clipPoint.w;
    clipPoint.z /= clipPoint.w;
    return f32v3((clipPoint.x + 1.0) / 2.0,
                 (1.0 - clipPoint.y) / 2.0,
                 clipPoint.z);
}

f32v3 Camera::getPickRay(const f32v2& ndcScreenPos) const {
    f32v4 clipRay(ndcScreenPos.x, ndcScreenPos.y, - 1.0f, 1.0f);
    f32v4 eyeRay = glm::inverse(m_projectionMatrix) * clipRay;
    eyeRay = f32v4(eyeRay.x, eyeRay.y, -1.0f, 0.0f);
    return glm::normalize(f32v3(glm::inverse(m_viewMatrix) * eyeRay));
}

void CinematicCamera::update()
{
    m_viewChanged = true;
    /// Smooth movement towards target

    if (ABS(m_focalLength - m_targetFocalLength) < 0.1) {
        m_focalLength = m_targetFocalLength;
    } else {
        m_focalLength = lerp(m_focalLength, m_targetFocalLength, m_speed);
    }
    m_focalPoint = lerp(m_focalPoint, m_targetFocalPoint, m_speed);
    m_direction = glm::mix(m_direction, m_targetDirection, m_speed);
    m_right = glm::mix(m_right, m_targetRight, m_speed);
    m_up = glm::normalize(glm::cross(m_right, m_direction));

    m_position = m_focalPoint - f64v3(m_direction) * m_focalLength;

    // Call base class update
    Camera::update();
}


void CinematicCamera::applyRotation(const f32q& rot) {
    m_targetDirection = rot * m_targetDirection;
    m_targetRight = rot * m_targetRight;
   
    m_viewChanged = true;
}

void CinematicCamera::rotateFromMouse(float dx, float dy, float speed) {
    f32q upQuat = glm::angleAxis(dy * speed, m_targetRight);
    f32v3 targetUp = glm::normalize(glm::cross(m_targetRight, m_targetDirection));
    f32q rightQuat = glm::angleAxis(dx * speed, targetUp);

    applyRotation(upQuat * rightQuat);
}

void CinematicCamera::rollFromMouse(float dx, float speed) {
    f32q frontQuat = glm::angleAxis(dx * speed, m_targetDirection);

    applyRotation(frontQuat);
}

void CinematicCamera::offsetTargetFocalLength(float offset) {
    m_targetFocalLength += offset;
    if (m_targetFocalLength < 0.0) {
        m_targetFocalLength = 0.0;
    } else if (m_targetFocalLength > m_maxFocalLength) {
        m_targetFocalLength = m_maxFocalLength;
    }
}

void CinematicCamera::setTarget(const f64v3& targetFocalPoint, const f32v3& targetDirection,
                                const f32v3& targetRight, double targetFocalLength) {
    m_targetFocalPoint = targetFocalPoint;
    m_targetDirection = targetDirection;
    m_targetRight = targetRight;
    m_targetFocalLength = targetFocalLength;
}
