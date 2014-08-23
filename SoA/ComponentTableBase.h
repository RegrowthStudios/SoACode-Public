#pragma once
#include "Component.h"

class ComponentTableBase {
public:
    ComponentTableBase(size_t compSize);
    ~ComponentTableBase();

    Component* createComponent();

    Component& getComponent(size_t index) const;
    Component& operator[](size_t index) const {
        return getComponent(index);
    }
private:
    ubyte* _components;
    size_t _componentsCount;
    size_t _componentsCapacity;

    std::vector<size_t> _removedComponents;
    size_t _componentSize;
};

