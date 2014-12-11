#include "stdafx.h"
#include "Camera.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <SDL/SDL.h>

#include "Options.h"
#include "utils.h"

#ifndef M_PI
#define M_PI 3.1415926f
#endif


static const float maxPitch = 89.0f; //must be less than 90 to avoid gimbal lock

static inline float RadiansToDegrees(float radians) {
    return radians * 180.0f / (float)M_PI;
}

Camera::Camera() : _position(0.0),
                _direction(0.0, 0.0, 1.0),
                _right(-1.0, 0.0, 0.0),
                _up(0.0, 1.0, 0.0),
                _fieldOfView(75.0f),
                _zNear(0.01f),
                _zFar(100000.0f),
                _aspectRatio(4.0f / 3.0f),
                _viewChanged(true),
                _projectionChanged(true),
                _useAngles(1)
{
}


void Camera::init(float aspectRatio)
{
    _aspectRatio = aspectRatio;
}

void Camera::offsetPosition(glm::dvec3 offset)
{
    _position += offset;
    _viewChanged = 1;
}

void Camera::offsetPosition(glm::vec3 offset)
{
    _position += offset;
    _viewChanged = 1;
}

void Camera::offsetAngles(float pitchAngle, float yawAngle)
{
    _pitchAngle += pitchAngle;
    _yawAngle += yawAngle;
    normalizeAngles();
    _viewChanged = 1;
}

void Camera::normalizeAngles() {
    _yawAngle = fmodf(_yawAngle, 360.0f);
    //fmodf can return negative values, but this will make them all positive
    if (_yawAngle < 0.0f){
        _yawAngle += 360.0f;
    }
    if (_pitchAngle > maxPitch){
        _pitchAngle = maxPitch;
    }
    else if (_pitchAngle < -maxPitch){
        _pitchAngle = -maxPitch;
    }
}

void Camera::update()
{
    if (_fieldOfView != graphicsOptions.fov){
        setFieldOfView(graphicsOptions.fov);
    }

    bool updateFrustum = false;
    if (_viewChanged){
        updateView();
        _viewChanged = false;
        updateFrustum = true;
    }
    if (_projectionChanged){
        updateProjection();
        _projectionChanged = false;
        updateFrustum = true;
    }
    if (updateFrustum) {
        _frustum.update(_projectionMatrix, _viewMatrix);
    }
}

void Camera::updateView()
{
    //if we are using basic pitch and yaw
    if (_useAngles){
        glm::mat4 orientation;

        orientation = glm::rotate(orientation, _pitchAngle, glm::vec3(1, 0, 0));
        orientation = glm::rotate(orientation, _yawAngle, glm::vec3(0, 1, 0));

        glm::mat4 invOrientation = glm::inverse(orientation);

        glm::vec4 direction = invOrientation * glm::vec4(0, 0, -1, 1);
        _direction = glm::vec3(direction);

        glm::vec4 right = invOrientation * glm::vec4(1, 0, 0, 1);
        _right = glm::vec3(right);

        glm::vec4 up = invOrientation * glm::vec4(0, 1, 0, 1);
        _up = glm::vec3(up);

        _viewMatrix = orientation * glm::translate(glm::mat4(), _focalLength*_direction);
    }
    else{ //if the vectors are set explicitly
        _viewMatrix = glm::lookAt(glm::vec3(0.0f), _direction, _up);
    }
}

void Camera::updateProjection()
{
    _projectionMatrix = glm::perspective(_fieldOfView, _aspectRatio, _zNear, _zFar);
}

void Camera::applyRotation(const f32q& rot) {
    _direction = rot * _direction;
    _right = rot * _right;
    _up = glm::normalize(glm::cross(_right, _direction));
    _viewChanged = true;
}

void Camera::rotateFromMouse(float dx, float dy, float speed) {
    f32q upQuat = glm::angleAxis(dy * speed, _right);
    f32q rightQuat = glm::angleAxis(dx * speed, _up);
 
    applyRotation(upQuat * rightQuat);
}

void Camera::yawFromMouse(float dx, float speed) {
    f32q frontQuat = glm::angleAxis(dx * speed, _direction);

    applyRotation(frontQuat);
}

void CinematicCamera::update()
{
    if (_isZooming){ //manual movement
        float t = (SDL_GetTicks() - _startTime);
        if (t >= _zoomDuration){
            _focalPoint = _zoomTargetPos;
            _focalLength = _targetFocalLength;
            _direction = _zoomTargetDir;
            _right = _zoomTargetRight;
            _isZooming = 0;
        }else{
            t /= _zoomDuration;
            double hermite_t = (3.0f * t * t) - (2.0f * t * t * t);
           
            _focalPoint = INTERPOLATE(hermite_t, _zoomStartPos, _zoomTargetPos);

            _focalLength = INTERPOLATE(hermite_t, _startFocalLength, _targetFocalLength);
            _direction = glm::normalize(INTERPOLATE((float)hermite_t, _zoomStartDir, _zoomTargetDir));
            _right = glm::normalize(INTERPOLATE((float)hermite_t, _zoomStartRight, _zoomTargetRight));
        }
        _up = glm::normalize(glm::cross(_right, _direction));
        _viewChanged = true;
    }
    _position = _focalPoint - glm::dvec3(_direction*_focalLength);

    // Call base class update
    Camera::update();
}

void CinematicCamera::zoomTo(glm::dvec3 targetPos, double time_s, glm::dvec3 endDirection, glm::dvec3 endRight, double endFocalLength)
{
        //the integral of sin(x) from 0 to pi is 2
        _useAngles = 0; //don't want to use original angles
        _isZooming = 1;
        _zoomStartPos = _position;
        _zoomTargetPos = targetPos;
        _targetFocalLength = endFocalLength;
        _startFocalLength = _focalLength;
        _startTime = SDL_GetTicks();
        _endTime = SDL_GetTicks() + time_s*1000.0;
        _zoomDuration = _endTime - _startTime;
        if (_zoomDuration == 0) _isZooming = 0;
        // Make sure the vectors are orthogonal
        f64v3 up = glm::normalize(glm::cross(endDirection, endRight));
        endRight = glm::normalize(glm::cross(endDirection, up));

        _zoomTargetDir = endDirection;
        _zoomStartDir = _direction;
        _zoomTargetRight = endRight;
        _zoomStartRight = _right;
        /*float t = time / duration;
        float hermite_t = (3.0f * t * t) - (2.0f * t * t * t);
        positionNow = interpolate(hermite_t, positionA, positionB);*/
}


