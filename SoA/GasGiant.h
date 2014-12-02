///
/// GasGiant.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines the Gas Giant class
///

#pragma once

#ifndef GasGiant_h__
#define GasGiant_h__

#include "IPlanetaryBody.h"

class GasGiant : public IPlanetaryBody {
public:
    GasGiant();
    ~GasGiant();

    virtual void update(f64 time) override;

    virtual void draw(const Camera* camera) override;

};

#endif // GasGiant_h__