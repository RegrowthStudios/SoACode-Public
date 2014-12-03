///
/// ECS.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 10 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// The main ECS class
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
        
        typedef std::pair<nString, ComponentTableBase*> NamedComponent; ///< A component table paired with its name
        typedef std::unordered_map<nString, ComponentTableBase*> ComponentSet; ///< Dictionary of NamedComponents

        /// Entity Component System
        class ECS {
        public:
            /// Default constructor which initializes events
            ECS();

            /// @return Set of entities for iteration
            const EntitySet& getEntities() const {
                return _entities;
            }
            /// @return Number of entities that are active
            size_t getActiveEntityCount() const {
                return _genEntity.getActiveCount();
            }
            /// @return The dictionary of NamedComponents
            const ComponentSet& getComponents() const {
                return _components;
            }

            /// @return The ID of a newly generated entity
            EntityID addEntity();
            /// Delete an entity from this ECS
            /// @param id: The entity's ID
            /// @return True if an entity was deleted
            bool deleteEntity(EntityID id);
            /// Generate a chunk of entities
            /// @param n: Number of entities to generate
            /// @param ids: Pointer to output array of entities
            void genEntities(const size_t& n, EntityID* ids) {
                for (size_t i = 0; i < n; i++) ids[i] = addEntity();
            }

            /// Add a component to an entity
            /// @param name: Friendly name of component
            /// @param id: Component owner entity
            /// @return ID of generated component
            ComponentID addComponent(nString name, EntityID id);
            /// Remove a component from an entity
            /// @param name: Friendly name of component
            /// @param id: Component owner entity
            /// @return True if a component was deleted
            bool deleteComponent(nString name, EntityID id);

            /// Add a component table to be referenced by a special name
            /// @param name: Friendly name of component
            /// @param table: Component table
            void addComponentTable(nString name, ComponentTableBase* table);
            /// Obtain a component table by its name
            /// @param name: Friendly name of component
            /// @return The component table
            ComponentTableBase* getComponentTable(nString name);

            Event<EntityID> onEntityAdded; ///< Called when an entity is added to this system
            Event<EntityID> onEntityRemoved; ///< Called when an entity is removed from this system
            Event<NamedComponent> onComponentAdded; ///< Called when a component table is added to this system
        private:
            typedef std::pair<ComponentTableBase*, std::shared_ptr<IDelegate<EntityID>>> ComponentSubscriber;
            typedef std::unordered_map<nString, ComponentSubscriber> ComponentSubscriberSet;

            EntitySet _entities; ///< List of entities
            IDGenerator<EntityID> _genEntity; ///< Unique ID generator for entities
            ComponentSet _components; ///< List of component tables

            ComponentSubscriberSet _componentTableBinds; ///< List of function hooks
        };
    }
}
namespace vcore = vorb::core;

#endif // ECS_h__