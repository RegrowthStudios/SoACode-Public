#include "stdafx.h"
#include "ECS.h"

#include "ComponentTableBase.h"

vcore::ECS::ECS() : 
    onEntityAdded(this),
    onEntityRemoved(this),
    onComponentAdded(this) {
    // Empty
}

vcore::EntityID vcore::ECS::addEntity() {
    // Generate a new entity
    EntityID id = _genEntity.generate();
    _entities.emplace(id);

    // Signal a newly created entity
    onEntityAdded(id);

    // Return the ID of the newly created entity
    return id;
}
bool vcore::ECS::deleteEntity(EntityID id) {
    // Check for a correct ID
    if (id >= _entities.size()) return false;
    
    // Recycle the ID
    _entities.erase(id);
    _genEntity.recycle(id);

    // Signal an entity must be destroyed
    onEntityRemoved(id);

    return true;
}

void vcore::ECS::addComponentTable(nString name, vcore::ComponentTableBase* table) {
    std::shared_ptr<IDelegate<EntityID>> f(onEntityRemoved.addFunctor([=] (void* sender, EntityID id) {
        table->remove(id);
    }));
    ComponentSubscriber binding(table, f);
    _componentTableBinds.insert(std::make_pair(name, binding));

    NamedComponent nc = std::make_pair(name, table);
    _components.insert(nc);
    onComponentAdded(nc);
}
vcore::ComponentTableBase* vcore::ECS::getComponentTable(nString name) {
    return _components.at(name);
}

vcore::ComponentID vcore::ECS::addComponent(nString name, EntityID id) {
    ComponentTableBase* table = getComponentTable(name);
    if (!table) return ID_GENERATOR_NULL_ID;
    return table->add(id);
}
bool vcore::ECS::deleteComponent(nString name, EntityID id) {
    ComponentTableBase* table = getComponentTable(name);
    if (!table) return false;
    return table->remove(id);
}
