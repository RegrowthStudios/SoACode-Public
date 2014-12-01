///
/// ComponentTableBase.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 9 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef ComponentTableBase_h__
#define ComponentTableBase_h__

#include "Entity.h"
#include "Events.hpp"
#include "IDGenerator.h"

namespace vorb {
    namespace core {
        typedef std::unordered_map<EntityID, ComponentID> ComponentBindingSet;

        class ComponentTableBase {
        public:
            ComponentTableBase();

            void onEntityRemoval(void* sender, EntityID id) {
                remove(id);
            }

            void add(EntityID eID);
            bool remove(EntityID eID);

            virtual void addComponent(ComponentID cID, EntityID eID) = 0;
            virtual void setComponent(ComponentID cID, EntityID eID) = 0;

            ComponentBindingSet::iterator begin() {
                return _components.begin();
            }
            ComponentBindingSet::iterator end() {
                return _components.end();
            }
            ComponentBindingSet::const_iterator cbegin() const {
                return _components.cbegin();
            }
            ComponentBindingSet::const_iterator cend() const {
                return _components.cend();
            }

            const ComponentID& getComponentID(EntityID eID) const {
                auto comp = _components.find(eID);
                if (comp == _components.end()) return BAD_ID;
                return comp->second;
            }

            size_t getComponentCount() const {
                return _components.size(); // This should be equal to _genComponent.getActiveCount()
            }

            Event<EntityID> onEntityAdded;
            Event<EntityID> onEntityRemoved;
        private:
            static const ComponentID BAD_ID = ID_GENERATOR_NULL_ID;
            ComponentBindingSet _components;
            IDGenerator<ComponentID> _genComponent;
        };
    }
}
namespace vcore = vorb::core;

#endif // ComponentTableBase_h__

