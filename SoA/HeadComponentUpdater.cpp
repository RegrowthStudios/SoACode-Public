#include "stdafx.h"
#include "HeadComponentUpdater.h"
#include "Constants.h"

#include "GameSystem.h"
#include "soaUtils.h"

void HeadComponentUpdater::rotateFromMouse(GameSystem* gameSystem, vecs::ComponentID cmpID, float dx, float dy, float speed) {
    auto& cmp = gameSystem->head.get(cmpID);
    // Pitch
    cmp.eulerAngles.x += dy * speed;
    cmp.eulerAngles.x = vmath::clamp(cmp.eulerAngles.x, -M_PI_2, M_PI_2);
    // Yaw
    cmp.eulerAngles.y += dx * speed;

    f64q qq(f64v3(0.0, (M_PI - 0.01), 0.0));
    f64v3 e = vmath::eulerAngles(qq);
    printVec("AAAH ", e);

    // Check if we need to rotate the body
    if (cmp.eulerAngles.y < 0.0) {
        auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPosition);
        f64v3 euler = vmath::radians(vmath::eulerAngles(vpCmp.orientation));
        euler.y += cmp.eulerAngles.y;
        if (euler.y < 0.0) euler.y += M_2_PI;
        vpCmp.orientation = f64q(euler);
        cmp.eulerAngles.y = 0.0;
        printVec("A ", euler);
    } else if (cmp.eulerAngles.y > M_PI) {
        auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPosition);
        f64v3 euler = vmath::radians(vmath::eulerAngles(vpCmp.orientation));
        euler.y += cmp.eulerAngles.y - M_PI;
        if (euler.y > M_2_PI) euler.y -= M_2_PI;
        vpCmp.orientation = f64q(euler);
        cmp.eulerAngles.y = M_PI;
        printVec("B ", euler);
    }
    cmp.relativeOrientation = f64q(cmp.eulerAngles);
}