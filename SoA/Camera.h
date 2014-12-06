#pragma once
#include "global.h"
#include "Frustum.h"

class Camera
{
public:
    Camera();
    void init(float aspectRatio);
    void offsetPosition(glm::dvec3 offset);
    void offsetPosition(glm::vec3 offset);
    void offsetAngles(float pitchAngle, float yawAngle);
    void update();
    void updateProjection();
    void applyRotation(const f32q& rot);

    // Frustum wrappers
    bool pointInFrustum(const f32v3& pos) const { return _frustum.pointInFrustum(pos); }
    bool sphereInFrustum(const f32v3& pos, float radius) const { return _frustum.sphereInFrustum(pos, radius); }

    //setters
    void setFocalPoint(glm::dvec3 focalPoint) { _focalPoint = focalPoint; _viewChanged = 1; }
    void setPosition(glm::dvec3 position){ _focalPoint = position; _position = position; _focalLength = 0;  _viewChanged = 1; }
    void setDirection(glm::vec3 direction){ _direction = direction; _viewChanged = 1; }
    void setRight(glm::vec3 right){ _right = right; _viewChanged = 1; }
    void setUp(glm::vec3 up){ _up = up; _viewChanged = 1; }
    void setClippingPlane(float zNear, float zFar){ _zNear = zNear; _zFar = zFar; _projectionChanged = 1; }
    void setFieldOfView(float fieldOfView){ _fieldOfView = fieldOfView; _projectionChanged = 1; }
    void setFocalLength(float focalLength) { _focalLength = focalLength; _viewChanged = 1; }
    void setPitchAngle(float pitchAngle) { _pitchAngle = pitchAngle; _viewChanged = 1; }
    void setYawAngle(float yawAngle) { _yawAngle = yawAngle; _viewChanged = 1; }
    void setAspectRatio(float aspectRatio) { _aspectRatio = aspectRatio; _projectionChanged = 1; }
    void setUseAngles(bool useAngles){ _useAngles = useAngles; }

    //getters
    const glm::dvec3& getPosition() const { return _position; }
    const glm::vec3& getDirection() const { return _direction; }
    const glm::vec3& getRight() const { return _right; }
    const glm::vec3& getUp() const { return _up; }

    const glm::mat4& getProjectionMatrix() const { return _projectionMatrix; }
    const glm::mat4& getViewMatrix() const { return _viewMatrix; }

    const float& getNearClip() const { return _zNear; }
    const float& getFarClip() const { return _zFar; }
    const float& getFieldOfView() const { return _fieldOfView; }
    const float& getAspectRatio() const { return _aspectRatio; }
    const float& getFocalLength() const { return _focalLength; }
    const float& getPitchAngle() const { return _pitchAngle; }
    const float& getYawAngle() const { return _yawAngle; }

protected:
    void normalizeAngles();
    void updateView();

    float _zNear, _zFar, _fieldOfView, _focalLength;
    float _pitchAngle, _yawAngle, _aspectRatio;
    bool _viewChanged, _projectionChanged;
    bool _useAngles;

    glm::dvec3 _focalPoint;
    glm::dvec3 _position;
    glm::vec3 _direction;
    glm::vec3 _right;
    glm::vec3 _up;

    glm::mat4 _projectionMatrix;
    glm::mat4 _viewMatrix;

    Frustum _frustum; ///< For frustum culling
};

class CinematicCamera : public Camera
{
public:
    void update();
    void zoomTo(glm::dvec3 targetPos, double time_s, glm::dvec3 endDirection, glm::dvec3 endRight, double endFocalLength);

    bool getIsZooming() const { return _isZooming; }

private:
    bool _isZooming;
    double _mouseSpeed;
    double _zoomDuration;
    double _startTime;
    double _endTime;
    double _startFocalLength;
    double _targetFocalLength;
    
    glm::dvec3 _zoomTargetPos;
    glm::dvec3 _zoomStartPos;
    glm::vec3 _zoomTargetDir;
    glm::vec3 _zoomStartDir;
    glm::vec3 _zoomTargetRight;
    glm::vec3 _zoomStartRight;
};
