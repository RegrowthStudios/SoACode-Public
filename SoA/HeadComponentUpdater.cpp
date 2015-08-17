#include "stdafx.h"
#include "HeadComponentUpdater.h"
#include "Constants.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include "GameSystem.h"
#include "soaUtils.h"

void HeadComponentUpdater::rotateFromMouse(GameSystem* gameSystem, vecs::ComponentID cmpID, float dx, float dy, float speed) {
    auto& cmp = gameSystem->head.get(cmpID);
    // Pitch
    cmp.eulerAngles.x += dy * speed;
    cmp.eulerAngles.x = glm::clamp(cmp.eulerAngles.x, -M_PI_2, M_PI_2);
    // Yaw
    cmp.eulerAngles.y += dx * speed;
    f64q qq(f64v3(M_PI_2, -2.0, 3.1));
    f64v3 e = glm::radians(glm::eulerAngles(qq));
    printVec("AAAH ", e);

    // Check if we need to rotate the body
    if (cmp.eulerAngles.y < -M_PI_2) {
        auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPosition);
        f64v3 euler = glm::radians(glm::eulerAngles(vpCmp.orientation));
        euler.y += cmp.eulerAngles.y - (-M_PI_2);
       // if (euler.y < -M_PI_2) euler.y += M_PI;
        vpCmp.orientation = f64q(euler);
        cmp.eulerAngles.y = -M_PI_2;
        std::cout << "R " << euler.y << " ";;
    } else if (cmp.eulerAngles.y > M_PI_2) {
        auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPosition);
        f64v3 euler = glm::radians(glm::eulerAngles(vpCmp.orientation));
        euler.y += cmp.eulerAngles.y - M_PI_2;
       // if (euler.y > M_PI_2) euler.y -= M_PI;
        vpCmp.orientation = f64q(euler);
        cmp.eulerAngles.y = M_PI_2;
        std::cout << "L " << euler.y << " ";;
    }
    cmp.relativeOrientation = f64q(cmp.eulerAngles);
}