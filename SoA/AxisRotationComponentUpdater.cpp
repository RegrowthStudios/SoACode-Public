#include "stdafx.h"
#include "AxisRotationComponentUpdater.h"

#include "SpaceSystem.h"
#include <glm/gtc/quaternion.hpp>

void AxisRotationComponentUpdater::update(SpaceSystem* spaceSystem, f64 time) {
    for (auto& it : spaceSystem->m_axisRotationCT) {
        auto& cmp = it.second;
        // Calculate rotation
     //   cmp.currentRotation = cmp.angularSpeed_RS * time;

        // Calculate the axis rotation quat
        f64v3 eulerAngles(0, cmp.currentRotation, 0);
        f64q rotationQuat = f64q(eulerAngles);

        // Calculate total orientation
        cmp.currentOrientation = cmp.axisOrientation * rotationQuat;
        cmp.invCurrentOrientation = glm::inverse(cmp.currentOrientation);
    }
}
