///
/// ComponentTable.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 10 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Common abstract implementation of ComponentTableBase for any component type
///

#pragma once

#ifndef ComponentTable_h__
#define ComponentTable_h__

#include "ComponentTableBase.h"

namespace vorb {
    namespace core {
        /// Component table that stores a specific component type
        template<typename T>
        class ComponentTable : public ComponentTableBase {
        public:
            typedef std::pair<EntityID, T> ComponentPairing; ///< A pairing of an entity to a component
            typedef std::vector<ComponentPairing> ComponentList; ///< List of components accessed by component IDs

            /// Constructor that requires a blank component for reference
            /// @param defaultData: Blank component data
            ComponentTable(T defaultData) : ComponentTableBase() {
                // Default data goes in the first slot
                _components.emplace_back(ID_GENERATOR_NULL_ID, defaultData);
            }
            /// Default constructor that attempts to use Component::createDefault() as blank component data
            ComponentTable() : ComponentTable(T::createDefault()) {
                // Empty
            }
        
            virtual void update(ComponentID cID) override {
                update(_components[cID].first, cID, get(cID));
            }

            /// Abstract update method for a single component
            /// @param eID: Entity ID of updating component
            /// @param cID: Component ID of updating component
            /// @param component: Component reference
            virtual void update(const EntityID& eID, const ComponentID& cID, T& component) = 0;

            /// Obtain a component from this table
            /// @param cID: Component ID
            /// @return Const component reference
            const T& get(const ComponentID& cID) const {
                return _components[cID].second;
            }
            /// Obtain a component from this table
            /// @param cID: Component ID
            /// @return Component reference
            T& get(const ComponentID& cID) {
                return _components[cID].second;
            }
            /// Obtain an entity's component from this table
            /// @param eID: Entity ID
            /// @return Const component reference
            const T& getFromEntity(const EntityID& eID) const {
                return get(getComponentID(eID));
            }
            /// Obtain an entity's component from this table
            /// @param eID: Entity ID
            /// @return Component reference
            T& getFromEntity(const EntityID& eID) {
                return get(getComponentID(eID));
            }

            /// @return The blank component data
            const T& getDefaultData() const {
                return _components[0].second;
            }

            /// @return Reference to component list for iteration
            operator const ComponentList& () const {
                return _components;
            }
        protected:
            virtual void addComponent(ComponentID cID, EntityID eID) override {
                T val = getDefaultData();
                _components.emplace_back(eID, val);
            }
            virtual void setComponent(ComponentID cID, EntityID eID) override {
                _components[cID].first = eID;
                _components[cID].second = getDefaultData();
            }

            ComponentList _components; ///< A list of (entity ID, Component)
        };
    }
}
namespace vcore = vorb::core;

#endif // ComponentTable_h__