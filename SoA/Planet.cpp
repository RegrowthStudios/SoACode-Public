#include "stdafx.h"
#include "Planet.h"

#include "Camera.h"
#include "DebugRenderer.h"
#include "IOManager.h"
#include "utils.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

class PlanetKegProperties {
public:
    f64 radius;
    f64 density;
    f64v3 axis;
    f64 angularSpeed;
};

KEG_TYPE_INIT_BEGIN_DEF_VAR(PlanetKegProperties)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("radius", Keg::Value::basic(Keg::BasicType::F64, offsetof(PlanetKegProperties, radius)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("density", Keg::Value::basic(Keg::BasicType::F64, offsetof(PlanetKegProperties, density)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("axis", Keg::Value::basic(Keg::BasicType::F64_V3, offsetof(PlanetKegProperties, axis)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("angularSpeed", Keg::Value::basic(Keg::BasicType::F64, offsetof(PlanetKegProperties, angularSpeed)));
KEG_TYPE_INIT_END

void Planet::init(const cString filePath) {
    /// Load the initial properties
    loadProperties(filePath);
}

void Planet::update(f64 time) {
    // Call base update
    IPlanetaryBody::update(time);
}

void Planet::draw(const Camera* camera) {

    static DebugRenderer debugRenderer;

    debugRenderer.drawIcosphere(f32v3(0), radius_, f32v4(1.0), 6);

    f32m4 rotationMatrix = f32m4(glm::toMat4(currentOrientation_));

    f32m4 WVP = camera->getProjectionMatrix() * camera->getViewMatrix() * rotationMatrix;

    debugRenderer.render(WVP, f32v3(camera->getPosition()));

    throw std::logic_error("The method or operation is not implemented.");
}

bool Planet::loadProperties(const cString filePath) {
    IOManager ioManager; // TODO: Pass in
    PlanetKegProperties properties;
    nString data;
    ioManager.readFileToString(filePath, data);
    if (data.length()) {
        if (Keg::parse(&properties, data.c_str(), "BlockTexture") == Keg::Error::NONE) {
            radius_ = properties.radius;
            // Calculate mass
            f64 volume = (4.0 / 3.0) * (M_PI)* pow(radius_, 3.0);
            mass_ = properties.density * volume;
            // Get angular speed
            angularSpeed_RS_ = properties.angularSpeed;
            // Get rotation quaternion for axis
            properties.axis = glm::normalize(properties.axis);
            f64v3 up(0.0, 1.0, 0.0);
            axisOrientation_ = quatBetweenVectors(up, properties.axis);
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}