#include "stdafx.h"
#include "ComponentTableBase.h"

template<typename SortedSet1, typename SortedSet2>
void vorb::core::commonEntities(std::vector<ui64>& rv, SortedSet1* c1, SortedSet2* c2) {
    auto& end = std::set_intersection(c1->begin(), c1->end(), c2->begin(), c2->end(), rv.begin());
    rv.resize(end - rv.begin());
}

void vorb::core::commonEntities(std::vector<ui64>& rv, i32 count, ...) {
    std::vector<ui64> tmp1;

    std::vector<ui64>* v1;
    std::vector<ui64>* v2;
    std::vector<ui64>* buf = nullptr;
    if (count % 2 == 0) {
        v1 = &rv;
        v2 = &tmp1;
    } else {
        v1 = &tmp1;
        v2 = &rv;
    }

    va_list args;
    va_start(args, count);

    // Get the first table and resize elements to fit inside
    ComponentTableBase* tFirst = va_arg(args, ComponentTableBase*);
    rv.resize(tFirst->getEntityCount());
    tmp1.resize(tFirst->getEntityCount());

    // Grab intersection of first two tables
    ComponentTableBase* t = va_arg(args, ComponentTableBase*);
    commonEntities(*v1, tFirst, t);

    // Intersect on remaining tables
    for (i32 i = 2; i < count; i++) {
        buf = v1; v1 = v2; v2 = buf;
        t = va_arg(args, ComponentTableBase*);
        commonEntities(*v1, v2, t);
    }
    va_end(args);
}

void vorb::core::ComponentTableBase::add(ui64 eID) {
    EntityIndexSet::iterator it = _entities.begin();
    while (it != _entities.end()) {
        if (*it > eID) break;
        it++;
    }
    _entities.insert(it, eID);
    addComponent(eID);
}
bool vorb::core::ComponentTableBase::remove(ui64 eID) {
    EntityIndexSet::iterator it = _entities.begin();
    while (it != _entities.end()) {
        if (*it == eID) break;
        it++;
    }
    if (it == _entities.end()) return false;

    _entities.erase(it);
    removeComponent(eID);
    return true;
}


