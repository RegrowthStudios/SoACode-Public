///
/// ECSSimpleUpdater.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Simplest implementation for an ECS updater
///

#pragma once

#ifndef ECSSimpleUpdater_h__
#define ECSSimpleUpdater_h__

#include "ComponentTableBase.h"
#include "ECS.h"

namespace vorb {
    namespace core {
        /// The simplest component updater implementation
        class ECSSimpleUpdater {
        public:
            /// Update all the components in the ECS in a linear fashion
            /// @param ecs: The ECS
            void update(vcore::ECS* ecs) {
                // Update all the components
                for (const NamedComponent namedComp : ecs->getComponents()) {
                    ComponentTableBase& table = *namedComp.second;
                    for (ComponentBinding bind : table) {
                        table.update(bind.second);
                    }
                }
            }
        };
    }
}
namespace vcore = vorb::core;

#endif // ECSSimpleUpdater_h__