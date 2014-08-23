#pragma once
#include "ComponentTableBase.h"

template<typename T>
class ComponentTable : public ComponentTableBase {
public:
    ComponentTable() : ComponentTableBase(sizeof(T)) {}

    T* create() {
        return reinterpret_cast<T*>(createComponent());
    }

    T& get(size_t index) const {
        return reinterpret_cast<T*>(_components)[index];
    }
    T& operator[](size_t index) const {
        return get(index);
    }
};