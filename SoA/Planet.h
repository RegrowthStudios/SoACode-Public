#pragma once
#include <list>
#include <set>
#include <deque>
#include <queue>

#include "Biome.h"
#include "OpenGLStructs.h"
#include "IOrbitalBody.h"
#include "Texture2d.h"
#include "WorldStructs.h"

#define P_TOP 0
#define P_LEFT 1
#define P_RIGHT 2
#define P_FRONT 3
#define P_BACK 4
#define P_BOTTOM 5

class TreeType;
class PlantType;
class Camera;

class Planet : public IOrbitalBody {
public:
    Planet();
    ~Planet();

    /// Initializes the planet
    /// @param filePath: The path to the planet description
    void init(const cString filePath);

    void update(f64 time) override;

    virtual void draw() override;

private:
    /// Loads the properties of the planet
    bool loadProperties();

    f64q poleAxis_; ///< Axis of rotation
    f64 rotationalSpeed_MS_ = 0.0; ///< Rotational speed about _poleAxis in radians
    f64 currentRotation_ = 0.0; ///< Current rotation about _poleAxis in radians
};
