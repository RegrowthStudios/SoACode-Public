#include "stdafx.h"
#include "ComponentTableBase.h"

#define DEFAULT_COMPONENT_TABLE_CAPACITY 4

ComponentTableBase::ComponentTableBase(size_t compSize) :
_componentSize(0),
_components(nullptr),
_componentsCount(0),
_componentsCapacity(DEFAULT_COMPONENT_TABLE_CAPACITY),
_removedComponents(std::vector<size_t>()) {
    _components = (ubyte*)calloc(_componentsCapacity, _componentSize);
}
ComponentTableBase::~ComponentTableBase() {
    // Free List Memory
    if (_removedComponents.size() > 0) {
        _removedComponents.swap(std::vector<size_t>());
    }

    // Free All The Components
    if (_components) {
        free(_components);
        _componentsCapacity = 0;
        _componentsCount = 0;
    }
}

Component* ComponentTableBase::createComponent() {
    // Where To Get The Component
    i32 index;

    // Check For An Available Component Location
    size_t availableCount = _removedComponents.size();
    if (availableCount > 0) {
        index = _removedComponents[availableCount - 1];
        _removedComponents.pop_back();
    } else {
        // Resize To Fit The New Component
        if (_componentsCount == _componentsCapacity) {
            size_t initialSize = _componentsCapacity * _componentSize;
            _componentsCapacity <<= 1;
            _components = (ubyte*)realloc(_components, initialSize << 1);
            memset(_components + initialSize, 0, initialSize);
        }
        // It's The Last Element
        index = _componentsCount;
    }
    ubyte* componentPtr = _components + index * _componentSize;
    _componentsCount++;
    return reinterpret_cast<Component*>(componentPtr);
}

Component& ComponentTableBase::getComponent(size_t index) const {
    return *reinterpret_cast<Component*>(_components + (index * _componentSize));
}