#include "stdafx.h"
#include "HeadComponentUpdater.h"
#include "Constants.h"

#include "GameSystem.h"
#include "soaUtils.h"

void HeadComponentUpdater::update(GameSystem* gameSystem VORB_UNUSED) {
    // Empty for now
}

void HeadComponentUpdater::rotateFromMouse(GameSystem* gameSystem, vecs::ComponentID cmpID, float dx, float dy, float speed) {
    // Seems like a race condition
    auto& cmp = gameSystem->head.get(cmpID);

    // Pitch
    cmp.eulerAngles.x += dy * speed;
    cmp.eulerAngles.x = vmath::clamp(cmp.eulerAngles.x, -M_PI_2, M_PI_2);
    // Yaw
    cmp.eulerAngles.y += dx * speed;

    // Check if we need to rotate the body
    if (cmp.eulerAngles.y < -M_PI_2) {
        auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPosition);
        vpCmp.eulerAngles.y += cmp.eulerAngles.y + M_PI_2;
        cmp.eulerAngles.y = -M_PI_2;
    } else if (cmp.eulerAngles.y > M_PI_2) {
        auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPosition);
        vpCmp.eulerAngles.y += cmp.eulerAngles.y - M_PI_2;
        cmp.eulerAngles.y = M_PI_2;
    }

    cmp.relativeOrientation = f64q(cmp.eulerAngles);
}