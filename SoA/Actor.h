#pragma once
#include "stdafx.h"

#include "Item.h"


class Actor {
public:
    Actor();
    ~Actor();

    f32v3 boundingBox;
    f32v3 velocity;
    f64v3 facePosition;
    f64v3 gridPosition;
    f64v3 headPosition;
    f64v3 worldPosition;
    f64v3 solarPosition;
    std::vector<Item*> inventory;
};
