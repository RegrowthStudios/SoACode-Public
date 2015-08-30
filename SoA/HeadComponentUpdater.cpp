#include "stdafx.h"
#include "HeadComponentUpdater.h"
#include "Constants.h"

#include "GameSystem.h"
#include "soaUtils.h"

void HeadComponentUpdater::update(GameSystem* gameSystem) {
    for (auto& it : gameSystem->head) {
        auto& cmp = it.second;
        cmp.relativeOrientation = f64q(cmp.eulerAngles);
    }
}

void HeadComponentUpdater::rotateFromMouse(GameSystem* gameSystem, vecs::ComponentID cmpID, float dx, float dy, float speed) {
    // Seems like a race condition
    auto& cmp = gameSystem->head.get(cmpID);

    // Copy to reduce race conditions
    f64v3 euler = cmp.eulerAngles;
    // Pitch
    euler.x += dy * speed;
    euler.x = vmath::clamp(euler.x, -M_PI_2, M_PI_2);
    // Yaw
    euler.y += dx * speed;

    // Check if we need to rotate the body
    if (euler.y < -M_PI_2) {
        auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPosition);
        vpCmp.eulerAngles.y += euler.y + M_PI_2;
        euler.y = -M_PI_2;
    } else if (euler.y > M_PI_2) {
        auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPosition);
        vpCmp.eulerAngles.y += euler.y - M_PI_2;
        euler.y = M_PI_2;
    }
    
    cmp.eulerAngles = euler;
}