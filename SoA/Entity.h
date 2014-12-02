///
/// Entity.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 9 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Basic ECS types
///

#pragma once

#ifndef Entity_h__
#define Entity_h__

#include <unordered_set>

namespace vorb {
    namespace core {
        typedef ui32 EntityID; ///< Numeric ID type for entities
        typedef ui32 ComponentID; ///< Numeric ID type for components
        typedef std::pair<EntityID, ComponentID> ComponentBinding;  ///< Pairing of entities and components
        typedef std::unordered_map<EntityID, ComponentID> ComponentBindingSet; ///< Set of entity-component pairings

        /// Basically an ID in an ECS
        class Entity {
        public:
            /// It must not be nameless
            /// @param _id: Unique non-zero id
            Entity(const EntityID& _id) :
                id(_id) {
                // Empty
            }

            const EntityID id; ///< Unique ID

            bool operator== (const Entity& e) const {
                return id == e.id;
            }
        };

        typedef std::unordered_set<Entity> EntitySet; ///< A set of entities
        typedef std::unordered_set<EntityID> EntityIDSet; ///< A set of entity IDs
    }
}
namespace vcore = vorb::core;

/// Entity hash implementation
template<> struct std::hash<vcore::Entity> {
public:
    size_t operator() (const vcore::Entity& e) const {
        return _internal(e.id);
    }
private:
    std::hash<vcore::EntityID> _internal;
};


#endif // Entity_h__