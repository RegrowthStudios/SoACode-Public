///
/// SolarSystem.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines a solar system
///

#pragma once

#ifndef SolarSystem_h__
#define SolarSystem_h__

#include <vector>

class IOrbitalBody;

class SolarSystem {
public:
    SolarSystem();
    ~SolarSystem();
    
    /// Initializes the solar system
    /// @param dirPath: Path to the system directory
    void init(const nString& dirPath);

    /// Updates the solar system
    /// @param time: Time since the beginning in seconds
    void update(f64 time);

private:
    /// Loads the properties file
    /// @param filePath: path to the properties file
    /// @return true on success
    bool loadProperties(const cString filePath);

    nString name_; ///< Name of the system
    nString dirPath_; /// Path to the system directory

    std::vector <IOrbitalBody*> orbitalBodies_;
};

#endif // SolarSystem_h__