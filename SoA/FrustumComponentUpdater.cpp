#include "stdafx.h"
#include "FrustumComponentUpdater.h"

#include "GameSystem.h"

void FrustumComponentUpdater::update(OUT GameSystem* gameSystem) {
    f64q orientation;
    f32v3 up;
    f32v3 dir;
    const f32v3 pos(0.0f); ///< Always treat as origin for precision
    for (auto& it : gameSystem->frustum) {
        auto& cmp = it.second;
        
        // Get orientation based on position and head
        if (cmp.voxelPosition) {
            auto& vpCmp = gameSystem->voxelPosition.get(cmp.voxelPosition);
            orientation = vpCmp.orientation;     
        } else {
            auto& spCmp = gameSystem->spacePosition.get(cmp.spacePosition);
            orientation = spCmp.orientation;
        }
        if (cmp.head) {
            auto& hCmp = gameSystem->head.get(cmp.head);
            orientation = orientation * hCmp.relativeOrientation;
        }

        up = orientation * f64v3(0.0, 1.0, 0.0);
        dir = orientation * f64v3(0.0, 0.0, 1.0);

        cmp.frustum.update(pos, dir, up);
    }
}
