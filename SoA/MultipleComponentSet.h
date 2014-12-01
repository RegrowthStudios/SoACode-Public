///
/// MultipleComponentSet.h
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

#ifndef MultipleComponentSet_h__
#define MultipleComponentSet_h__

#include <unordered_set>

#include "Entity.h"
#include "Events.hpp"

namespace vorb {
    namespace core {
        class ComponentTableBase;

        class MultipleComponentSet {
        public:
            typedef std::unordered_set<EntityID> EntityIDSet;

            MultipleComponentSet();
            ~MultipleComponentSet();

            void addRequirement(ComponentTableBase* component);

            EntityIDSet::iterator begin() {
                return _entities.begin();
            }
            EntityIDSet::iterator end() {
                return _entities.end();
            }
            EntityIDSet::const_iterator cbegin() const {
                return _entities.cbegin();
            }
            EntityIDSet::const_iterator cend() const {
                return _entities.cend();
            }

            Event<EntityID> onEntityAdded;
            Event<EntityID> onEntityRemoved;
        private:
            EntityIDSet _entities;
            std::vector<ComponentTableBase*> _tables;
            std::shared_ptr<IDelegate<EntityID>> _fEntityAdded;
            std::shared_ptr<IDelegate<EntityID>> _fEntityRemoved;
        };
    }
}
namespace vcore = vorb::core;

#endif // MultipleComponentSet_h__