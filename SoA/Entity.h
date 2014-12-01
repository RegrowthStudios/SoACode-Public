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

namespace vorb {
    namespace core {
        typedef ui64 EntityID; ///< Numeric ID type for entities
        typedef ui64 ComponentID; ///< Numeric ID type for components
        typedef std::pair<EntityID, ComponentID> ComponentBinding;  ///< Pairing of entities and components

        /// Basically an ID in an ECS
        class Entity {
        public:
            /// It must not be nameless
            /// @param _id: Unique non-zero id
            Entity(EntityID _id) :
                id(_id) {
                // Empty
            }

            const EntityID id; ///< Unique ID
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

        typedef std::vector<Entity> EntitySet; ///< Stores a set of entities
        typedef std::vector<EntityID> EntityIDSet; ///< Stores a set of entity IDs
    }
}
namespace vcore = vorb::core;

#endif // Entity_h__