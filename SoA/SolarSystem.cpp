#include "stdafx.h"
#include "SolarSystem.h"

#include "IOrbitalBody.h"

SolarSystem::SolarSystem() {
    // Empty
}

SolarSystem::~SolarSystem() {
    // Empty
}

void SolarSystem::init(const nString& dirPath) {
    // Set the directory path
    dirPath_ = dirPath;
    // Load the system properties
    loadProperties(dirPath_.c_str());
}

void SolarSystem::update(f64 time) {
    // Update all orbital bodies
    for (auto& body : orbitalBodies_) {
        body->update(time);
    }
}

bool SolarSystem::loadProperties(const cString filePath) {

    return true;
}
