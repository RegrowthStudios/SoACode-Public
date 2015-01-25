#include "stdafx.h"
#include "FrustumComponentUpdater.h"

#include "GameSystem.h"

void FrustumComponentUpdater::update(OUT GameSystem* gameSystem) {
    f64q orientation;
    f32v3 up;
    f32v3 dir;
    f32v3 pos;
    for (auto& it : gameSystem->frustum) {
        auto& cmp = it.second;
        
        // Get orientation based on position and head
        if (cmp.voxelPositionComponent) {
            auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPositionComponent);
            orientation = vpCmp.orientation;     
            pos = vpCmp.position;
        } else {
            auto& spCmp = gameSystem->spacePosition.get(cmp.spacePositionComponent);
            orientation = spCmp.orientation;
            pos = spCmp.position;
        }
        if (cmp.headComponent) {
            auto& hCmp = gameSystem->head.get(cmp.headComponent);
            orientation = hCmp.relativeOrientation * orientation;
            pos += hCmp.relativePosition;
        }

        up = orientation * f64v3(0.0, 1.0, 0.0);
        dir = orientation * f64v3(0.0, 0.0, 1.0);

        cmp.frustum.update(pos, dir, up);
    }
}
