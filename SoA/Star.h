///
/// Star.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines the Star class
///

#pragma once

#ifndef Star_h__
#define Star_h__

#include "IPlanetaryBody.h"

class Star : public IPlanetaryBody {
public:
    Star();
    ~Star();

    virtual void update(f64 time) override;

    virtual void draw(const Camera* camera) override;
private:
    f64 surfaceTemperature_K_ = 0.0f; ///< Surface temperature in kelvin
};

#endif // Star_h__