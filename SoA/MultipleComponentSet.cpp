#include "stdafx.h"
#include "MultipleComponentSet.h"

#include "ComponentTableBase.h"

vcore::MultipleComponentSet::MultipleComponentSet() :
    onEntityAdded(this),
    onEntityRemoved(this) {
    // Entity added handler
    _fEntityAdded.reset(createDelegate<EntityID>([=] (void* sender, EntityID eID) -> void {
        for (ComponentTableBase* table : this->_tables) {
            // See if a table doesn't contain the entity
            if (table == sender) continue;
            if (table->getComponentID(eID) == ID_GENERATOR_NULL_ID) return;
        }
        this->_entities.insert(eID);
        onEntityAdded(eID);
    }));

    // Entity removed handler
    _fEntityRemoved.reset(createDelegate<EntityID>([=] (void* sender, EntityID eID) -> void {
        // Always remove the entity from this list
        auto entity = this->_entities.find(eID);
        if (entity != _entities.end()) {
            _entities.erase(entity);
            onEntityRemoved(eID);
        }
    }));
}
vcore::MultipleComponentSet::~MultipleComponentSet() {
    // Remove event hooks for last copy of set
    if (_fEntityAdded.unique()) {
        for (ComponentTableBase* table : _tables) {
            table->onEntityAdded.remove(_fEntityAdded.get());
            table->onEntityRemoved.remove(_fEntityRemoved.get());
        }
    }
}

void vcore::MultipleComponentSet::addRequirement(ComponentTableBase* component) {
    // Check for existing requirement
    auto c = std::find(_tables.begin(), _tables.end(), component);
    if (c != _tables.end()) return;

    // Add handlers
    component->onEntityAdded.add(_fEntityAdded.get());
    component->onEntityRemoved.add(_fEntityRemoved.get());
    _tables.push_back(component);
}
