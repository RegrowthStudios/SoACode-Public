#include "stdafx.h"
#include "AxisRotationComponent.h"

#include <glm/gtc/quaternion.hpp>

void AxisRotationComponent::init(f64 AngularSpeed_RS, f64 CurrentRotation, f64q AxisOrientation) {
    angularSpeed_RS = AngularSpeed_RS;
    currentRotation = CurrentRotation;
    invCurrentOrientation = glm::inverse(currentOrientation);
    axisOrientation = AxisOrientation;
}