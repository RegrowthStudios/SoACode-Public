#pragma once
#include <list>
#include <set>
#include <deque>
#include <queue>

#include "Biome.h"
#include "OpenGLStructs.h"
#include "IPlanetaryBody.h"
#include "Texture2d.h"
#include "WorldStructs.h"

#define P_TOP 0
#define P_LEFT 1
#define P_RIGHT 2
#define P_FRONT 3
#define P_BACK 4
#define P_BOTTOM 5

class Camera;

class Planet : public IPlanetaryBody {
public:
    Planet();
    ~Planet();

    /// Initializes the planet
    /// @param filePath: The path to the planet properties
    void init(const cString filePath);

    /// Updates the planet
    /// @time: Time in sec since the beginning of this game
    void update(f64 time) override;

    /// Draws the planet
    virtual void draw() override;

private:
    /// Loads the properties of the planet
    /// @param filePath: The path to the planet properties
    /// @return true on success
    bool loadProperties(const cString filePath);
};
