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
        
        typedef std::pair<ComponentTableBase*, IDelegate<EntityID>*> ComponentSubscriber;
        typedef std::unordered_map<nString, ComponentSubscriber> ComponentSubscriberSet;
        typedef std::pair<nString, ComponentTableBase*> NamedComponent;
        typedef std::unordered_map<nString, ComponentTableBase*> ComponentSet;

        class ECS {
        public:
            ECS();

            const EntitySet& getEntities() const {
                return _entities;
            }
            size_t getActiveEntityCount() const {
                return _genEntity.getActiveCount();
            }
            const ComponentSet& getComponents() const {
                return _components;
            }

            EntityID addEntity();
            bool deleteEntity(EntityID id);
            void genEntities(const size_t& n, EntityID* ids) {
                for (size_t i = 0; i < n; i++) ids[i] = addEntity();
            }

            ComponentID addComponent(nString name, EntityID id);
            bool deleteComponent(nString name, EntityID id);

            void addComponentTable(nString name, ComponentTableBase* table);
            ComponentTableBase* getComponentTable(nString name);

            Event<EntityID> onEntityAdded;
            Event<EntityID> onEntityRemoved;
            Event<NamedComponent> onComponentAdded;
        private:
            EntitySet _entities;
            IDGenerator<EntityID> _genEntity;

            ComponentSubscriberSet _componentTableBinds;
            ComponentSet _components;
        };
    }
}
namespace vcore = vorb::core;

#endif // ECS_h__