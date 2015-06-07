#pragma once
#include "Frustum.h"

class Camera
{
public:
    Camera();
    void init(float aspectRatio);
    void offsetPosition(glm::dvec3 offset);
    void offsetPosition(glm::vec3 offset);
    void update();
    void updateProjection();
    virtual void applyRotation(const f32q& rot);
    virtual void rotateFromMouse(float dx, float dy, float speed);
    virtual void rollFromMouse(float dx, float speed);

    // Frustum wrappers
    bool pointInFrustum(const f32v3& pos) const { return m_frustum.pointInFrustum(pos); }
    bool sphereInFrustum(const f32v3& pos, float radius) const { return m_frustum.sphereInFrustum(pos, radius); }

    //setters
    void setOrientation(const f64q& orientation);
    void setFocalPoint(glm::dvec3 focalPoint) { m_focalPoint = focalPoint; m_viewChanged = 1; }
    void setPosition(glm::dvec3 position){ m_focalPoint = position; m_position = position; m_focalLength = 0;  m_viewChanged = 1; }
    void setDirection(glm::vec3 direction){ m_direction = direction; m_viewChanged = 1; }
    void setRight(glm::vec3 right){ m_right = right; m_viewChanged = 1; }
    void setUp(glm::vec3 up){ m_up = up; m_viewChanged = 1; }
    void setClippingPlane(float zNear, float zFar){ m_zNear = zNear; m_zFar = zFar; m_projectionChanged = 1; }
    void setFieldOfView(float fieldOfView){ m_fieldOfView = fieldOfView; m_projectionChanged = 1; }
    void setFocalLength(float focalLength) { m_focalLength = focalLength; m_viewChanged = 1; }
    void setAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; m_projectionChanged = 1; }

    // Gets the position of a 3D point on the screen plane
    f32v3 worldToScreenPoint(const f32v3& worldPoint) const;
    f32v3 worldToScreenPointLogZ(const f32v3& worldPoint, f32 zFar) const;
    f32v3 worldToScreenPoint(const f64v3& worldPoint) const;
    f32v3 worldToScreenPointLogZ(const f64v3& worldPoint, f64 zFar) const;
    f32v3 getPickRay(const f32v2& ndcScreenPos) const;

    //getters
    const glm::dvec3& getPosition() const { return m_position; }
    const glm::vec3& getDirection() const { return m_direction; }
    const glm::vec3& getRight() const { return m_right; }
    const glm::vec3& getUp() const { return m_up; }

    const f32m4& getProjectionMatrix() const { return m_projectionMatrix; }
    const f32m4& getViewMatrix() const { return m_viewMatrix; }
    const f32m4& getViewProjectionMatrix() const { return m_viewProjectionMatrix; }

    const f32& getNearClip() const { return m_zNear; }
    const f32& getFarClip() const { return m_zFar; }
    const f32& getFieldOfView() const { return m_fieldOfView; }
    const f32& getAspectRatio() const { return m_aspectRatio; }
    const f64& getFocalLength() const { return m_focalLength; }

    const Frustum& getFrustum() const { return m_frustum; }

protected:
    void normalizeAngles();
    void updateView();

    f32 m_zNear = 0.1f;
    f32 m_zFar = 100000.0f;
    f32 m_fieldOfView = 75.0f;
    f32 m_aspectRatio = 4.0f / 3.0f;
    f64 m_focalLength = 0.0;
    f64 m_maxFocalLength = 10000000000000000000000.0;
    bool m_viewChanged = true;
    bool m_projectionChanged = true;

    f64v3 m_focalPoint = f64v3(0.0);
    f64v3 m_position = f64v3(0.0);
    f32v3 m_direction = f32v3(1.0f, 0.0f, 0.0f);
    f32v3 m_right = f32v3(0.0f, 0.0f, 1.0f);
    f32v3 m_up = f32v3(0.0f, 1.0f, 0.0f);

    f32m4 m_projectionMatrix;
    f32m4 m_viewMatrix;
    f32m4 m_viewProjectionMatrix;

    Frustum m_frustum; ///< For frustum culling
};

class CinematicCamera : public Camera
{
public:
    void update();

    virtual void applyRotation(const f32q& rot) override;
    virtual void rotateFromMouse(float dx, float dy, float speed) override;
    virtual void rollFromMouse(float dx, float speed) override;

    void offsetTargetFocalLength(float offset);

    // Getters
    const f64& getTargetFocalLength() const { return m_targetFocalLength; }
    const f64& getSpeed() const { return m_speed; }

    // Setters
    void setSpeed(f64 speed) { m_speed = speed; }
    void setTarget(const f64v3& targetFocalPoint, const f32v3& targetDirection,
                   const f32v3& targetRight, f64 targetFocalLength);
    void setTargetDirection(const f32v3& targetDirection) { m_targetDirection = targetDirection; }
    void setTargetRight(const f32v3& targetRight) { m_targetRight = targetRight; }
    void setTargetFocalPoint(const f64v3& targetFocalPoint) { m_targetFocalPoint = targetFocalPoint; }
    void setTargetFocalLength(const float& targetFocalLength) { m_targetFocalLength = targetFocalLength; }

private:
    f32v3 m_targetDirection = m_direction; ///< Desired direction
    f32v3 m_targetRight = m_right; ///< Desired right
    f64v3 m_targetFocalPoint = m_focalPoint; ///< Target focal position
    f64 m_targetFocalLength = m_focalLength; ///< Desired focal length
    f64 m_speed = 0.3; ///< The speed of the camera. 1.0 is the highest
};
