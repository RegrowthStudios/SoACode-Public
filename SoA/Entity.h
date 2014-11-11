///
/// Entity.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 9 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef Entity_h__
#define Entity_h__

#include <set>

#define ENTITY_ID_NULL 0

namespace vorb {
    namespace core {
        /// Basically an ID in an ECS
        class Entity {
        public:
            /// It must not be nameless
            /// @param _id: Unique non-zero id
            Entity(ui64 _id);

            const ui64 id; ///< Unique ID
        };

        /// Compares two entities by their IDs
        class EntityComparer {
            bool operator()(const Entity& e1, const Entity& e2) {
                return e1.id < e2.id;
            }
            bool operator()(const Entity* e1, const Entity* e2) {
                return e1->id < e2->id;
            }
        };
        typedef std::vector<ui64> EntityIndexSet;
    }
}
namespace vcore = vorb::core;

#endif // Entity_h__