#pragma once
class Entity;

class Component {
public:
    Entity* owner;
    ui32 componentType;
};