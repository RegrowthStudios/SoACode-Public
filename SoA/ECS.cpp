#include "stdafx.h"
#include "ECS.h"

#include "ComponentTableBase.h"

vcore::ECS::ECS() : 
    evtEntityAddition(this),
    evtEntityRemoval(this) {
    // Empty
}

ui64 vcore::ECS::addEntity() {
    ui64 id;
    if (_usedIDs.size() > 0) { 
        // Add with a recycled ID
        id = _usedIDs.front();
        _usedIDs.pop();
    } else {
        // Add with a unique ID
        id = ++_eid;
        _entities.emplace_back(id);
    }

    // Signal a newly created entity
    _entityCount++;
    evtEntityAddition(id);

    // Return the ID of the newly created entity
    return id;
}
bool vcore::ECS::deleteEntity(ui64 id) {
    if (id >= _entities.size()) return false;
    
    // Signal an entity must be destroyed
    _entityCount--;
    evtEntityRemoval(id);

    // Recycle the ID
    _usedIDs.push(id);

    return true;
}

void vcore::ECS::addComponent(nString name, vcore::ComponentTableBase* table) {
    ComponentBinding binding(table, evtEntityRemoval.addFunctor([=] (void* sender, ui64 id) {
        table->remove(id);
    }));
    _componentTables.insert(std::pair<nString, ComponentBinding>(name, binding));
}
vcore::ComponentTableBase* vcore::ECS::get(nString name) {
    return _componentTables.at(name).first;
}
