///
/// AtmosphereComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 31 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Atmosphere component for planets
///

#pragma once

#ifndef AtmosphereComponent_h__
#define AtmosphereComponent_h__

class AtmosphereComponent {
public:
    AtmosphereComponent();
    ~AtmosphereComponent();

    void init();

    void draw();

    void loadProperties(const nString& filePath);

private:
};

#endif // AtmosphereComponent_h__