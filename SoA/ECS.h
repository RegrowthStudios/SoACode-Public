///
/// ECS.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 10 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef ECS_h__
#define ECS_h__

#include "Entity.h"
#include "Events.hpp"


namespace vorb {
    namespace core {
        class ComponentTableBase;
        
        class ECS {
        public:
            ECS();

            const std::vector<Entity>& getEntities() const {
                return _entities;
            }

            const i32& getEntityCount() const {
                return _entityCount;
            }

            ui64 addEntity();
            bool deleteEntity(ui64 id);

            void addComponent(nString name, ComponentTableBase* table);
            ComponentTableBase* get(nString name);

            Event<ui64> evtEntityAddition;
            Event<ui64> evtEntityRemoval;
        private:
            std::vector<Entity> _entities;
            i32 _entityCount = 0;
            std::queue<ui64> _usedIDs;
            ui64 _eid = ENTITY_ID_NULL;

            typedef std::pair<ComponentTableBase*, IDelegate<ui64>*> ComponentBinding;
            std::unordered_map<nString, ComponentBinding> _componentTables;
        };
    }
}
namespace vcore = vorb::core;

#endif // ECS_h__