///
/// ComponentTable.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 10 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef ComponentTable_h__
#define ComponentTable_h__

#include "ComponentTableBase.h"

namespace vorb {
    namespace core {
        template<typename T>
        class ComponentTable : public ComponentTableBase {
        public:
            typedef std::vector<std::pair<EntityID, T>> ComponentList; ///< List of components accessed by component IDs

            ComponentTable(T defaultData) : ComponentTableBase() {
                // Default data goes in the first slot
                _components.emplace_back(ID_GENERATOR_NULL_ID, defaultData);
            }
        
            virtual void addComponent(ComponentID cID, EntityID eID) override {
                T val = getDefaultData();
                _components.emplace_back(eID, val);
            }
            virtual void setComponent(ComponentID cID, EntityID eID) override {
                _components[cID].first = eID;
                _components[cID].second = getDefaultData();
            }
            
            const T& get(const ComponentID& cID) const {
                return _components[cID];
            }
            T& get(const ComponentID& cID) {
                return _components[cID];
            }
            const T& getFromEntity(const EntityID& eID) const {
                return get(getComponentID(eID));
            }
            T& getFromEntity(const EntityID& eID) {
                return get(getComponentID(eID));
            }

            const T& getDefaultData() const {
                return _components[0].second;
            }

            operator const ComponentList& () const {
                return _components;
            }
        protected:
            ComponentList _components;
        };
    }
}
namespace vcore = vorb::core;

#endif // ComponentTable_h__