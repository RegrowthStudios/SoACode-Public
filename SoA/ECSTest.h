///
/// ECSTest.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 1 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// "Unit test" for ECS module
///

#pragma once

#ifndef ECSTest_h__
#define ECSTest_h__

#include <SDL/SDL.h>

#include "ECSSimpleUpdater.h"
#include "MultiComponentTracker.hpp"
#include "SpaceSystem.h"

namespace test {
    /// Tests ECS features
    void ecs() {
        size_t entityCount, updateCount;
        std::cin >> entityCount;
        std::cin >> updateCount;
        while (entityCount != 0) {
            ui32 ts = SDL_GetTicks();
            {
                // Make ECS
                SpaceSystem space;
                vcore::ECSSimpleUpdater updater;

                // Add multi-component listeners
                vcore::MultiComponentTracker<2> mt;
                mt.addRequirement(space.getComponentTable(SPACE_SYSTEM_CT_OBJECT_NAME));
                mt.addRequirement(space.getComponentTable(SPACE_SYSTEM_CT_QUADRANT_NAME));

                // Build ECS
                vcore::EntityID* entities = new vcore::EntityID[entityCount];
                space.genEntities(entityCount, entities);
                auto tblObject = space.getComponentTable(SPACE_SYSTEM_CT_OBJECT_NAME);
                auto tblQuadrant = space.getComponentTable(SPACE_SYSTEM_CT_QUADRANT_NAME);
                for (size_t i = 0; i < entityCount; i++) {
                    tblObject->add(entities[i]);
                    tblQuadrant->add(entities[i]);
                }

                ts = SDL_GetTicks() - ts;
                printf("Ticks elapsed - Setup    - %d\n", ts);
                ts = SDL_GetTicks();

                // Update ECS
                for (size_t i = 0; i < updateCount; i++) updater.update(&space);

                ts = SDL_GetTicks() - ts;
                printf("Ticks elapsed - Updates  - %d\n", ts);
                printf("Updates: %d\n", space.tblObject.updates + space.tblQuadrants.updates);
                printf("Component Mem Usage: %f MB\n", (entityCount * (sizeof(SpaceObject)+sizeof(SpaceQuadrant))) / 1000000.0f);
                size_t cc = 0;
                for (auto& e : space.getEntities()) {
                    auto comps = mt.getComponents(e.id);
                    printf("\r<0x%08lX> -> <0x%08lX>, <0x%08lX>", e.id, comps[0], comps[1]);
                }
                puts("");
                ts = SDL_GetTicks();
                delete[] entities;
            }
            ts = SDL_GetTicks() - ts;
            printf("Ticks elapsed - Teardown - %d\n", ts);

            std::cin >> entityCount;
            std::cin >> updateCount;
        }

    }
}

#endif // ECSTest_h__