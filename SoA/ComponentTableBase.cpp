#include "stdafx.h"
#include "ComponentTableBase.h"

vcore::ComponentTableBase::ComponentTableBase() :
    onEntityAdded(this), 
    onEntityRemoved(this) {
    // Empty
}

vcore::ComponentID vcore::ComponentTableBase::add(EntityID eID) {
    // Check that this entity does not exist
    auto cBind = _components.find(eID);
    if (cBind != _components.end()) {
        char buf[256];
        sprintf(buf, "Entity <0x%08lX> already contains component <0x%08lX>", eID, cBind->second);
        throw std::exception(buf);
    }

    // Generate a new component
    bool shouldPush = false;
    ComponentID id = _genComponent.generate(&shouldPush);
    _components[eID] = id;

    if (shouldPush) {
        // Add a new component
        addComponent(id, eID);
    } else {
        // Recycle an old component
        setComponent(id, eID);
    }
    onEntityAdded(eID);
    return id;
}
bool vcore::ComponentTableBase::remove(EntityID eID) {
    // Find the entity
    auto cBind = _components.find(eID);
    if (cBind == _components.end()) return false;

    // Component is cleared
    _genComponent.recycle(cBind->second);
    setComponent(cBind->second, ID_GENERATOR_NULL_ID);
    _components.erase(cBind);
    onEntityRemoved(eID);

    return true;
}
