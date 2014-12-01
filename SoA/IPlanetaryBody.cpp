#include "stdafx.h"
#include "IPlanetaryBody.h"


IPlanetaryBody::IPlanetaryBody() {
}


IPlanetaryBody::~IPlanetaryBody() {
}

void IPlanetaryBody::update(f64 time) {

    // Calculate rotation
    currentRotation_ = angularSpeed_RS_ * time;

    // Calculate the axis rotation quat
    f64v3 eulerAngles(0, currentRotation_, 0);
    f64q rotationQuat = f64q(eulerAngles);

    // Calculate total orientation
    currentOrientation_ = axisOrientation_ * rotationQuat;

    // Updates the base class
    IPlanetaryBody::update(time);
}
