#include "stdafx.h"
#include "ECS.h"

#include "ComponentTableBase.h"

vcore::ECS::ECS() : 
    onEntityAddition(this),
    onEntityRemoval(this) {
    // Empty
}

vcore::EntityID vcore::ECS::addEntity() {
    // Generate a new entity
    EntityID id = _genEntity.generate();
    if (id > _entities.size()) _entities.emplace_back(id);

    // Signal a newly created entity
    onEntityAddition(id);

    // Return the ID of the newly created entity
    return id;
}
bool vcore::ECS::deleteEntity(EntityID id) {
    // Check for a correct ID
    if (id >= _entities.size()) return false;
    
    // Recycle the ID
    _genEntity.recycle(id);

    // Signal an entity must be destroyed
    onEntityRemoval(id);

    return true;
}

void vcore::ECS::addComponent(nString name, vcore::ComponentTableBase* table) {
    ComponentBinding binding(table, onEntityRemoval.addFunctor([=] (void* sender, EntityID id) {
        table->remove(id);
    }));
    _componentTables.insert(std::make_pair(name, binding));
}
vcore::ComponentTableBase* vcore::ECS::get(nString name) {
    return _componentTables.at(name).first;
}
