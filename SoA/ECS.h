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
#include "IDGenerator.h"

namespace vorb {
    namespace core {
        class ComponentTableBase;
        
        class ECS {
        public:
            ECS();

            const EntitySet& getEntities() const {
                return _entities;
            }
            const size_t& getActiveEntityCount() const {
                return _genEntity.getActiveCount();
            }

            EntityID addEntity();
            bool deleteEntity(EntityID id);

            void addComponent(nString name, ComponentTableBase* table);
            ComponentTableBase* get(nString name);

            Event<EntityID> onEntityAddition;
            Event<EntityID> onEntityRemoval;
        private:
            EntitySet _entities;
            IDGenerator<EntityID> _genEntity;

            typedef std::pair<ComponentTableBase*, IDelegate<EntityID>*> ComponentBinding;
            std::unordered_map<nString, ComponentBinding> _componentTables;
        };
    }
}
namespace vcore = vorb::core;

#endif // ECS_h__