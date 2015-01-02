#include "stdafx.h"
#include "macros.h"

#undef UNIT_TEST_BATCH
#define UNIT_TEST_BATCH Vorb_Core_ECS_

#include <Vorb/ECS.h>
#include <Vorb/ComponentTable.hpp>

TEST(Creation) {
    vcore::ECS ecs;
    return true;
}

TEST(MakeEntity) {
    vcore::ECS ecs;
    vcore::EntityID e = ecs.addEntity();
    assert(e == 1);
    return true;
}

struct Component {
public:
    int x = 10;
};
class CTTest : public vcore::ComponentTable<Component> {
    virtual void update(const vcore::EntityID& eID, const vcore::ComponentID& cID, Component& component) {
        component.x = eID;
    }
};

TEST(MakeComponent) {
    vcore::ECS ecs;
    
    CTTest ct;
    assert(ct.getDefaultData().x == 10);
    
    ecs.addComponentTable("C1", &ct);

    auto table = ecs.getComponentTable("C1");
    assert(table);

    return true;
}