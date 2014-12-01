///
/// ECSSimpleUpdater.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef ECSSimpleUpdater_h__
#define ECSSimpleUpdater_h__

#include "ComponentTableBase.h"
#include "ECS.h"

namespace vorb {
    namespace core {
        class ECSSimpleUpdater {
        public:
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