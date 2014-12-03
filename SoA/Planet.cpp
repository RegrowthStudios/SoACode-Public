#include "stdafx.h"
#include "Planet.h"

#include "Camera.h"
#include "DebugRenderer.h"
#include "IOManager.h"
#include "utils.h"
#include "Errors.h"

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

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    static DebugRenderer debugRenderer;

    debugRenderer.drawIcosphere(f32v3(0), radius_, f32v4(1.0), 5);

    f32m4 rotationMatrix = f32m4(glm::toMat4(currentOrientation_));

    f32m4 WVP = camera->getProjectionMatrix() * camera->getViewMatrix();

    debugRenderer.render(WVP, f32v3(camera->getPosition()));

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool Planet::loadProperties(const cString filePath) {
    IOManager ioManager; // TODO: Pass in
    PlanetKegProperties properties;
    nString data;
    ioManager.readFileToString(filePath, data);
    if (data.length()) {
        Keg::Error error = Keg::parse(&properties, data.c_str(), "PlanetKegProperties");
        if (error == Keg::Error::NONE) {
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
            fprintf(stderr, "Keg error %d for %s\n", (int)error, filePath);
            return false;
        }
    } else {
        pError("Failed to load planet " + nString(filePath));
        return false;
    }
    return true;
}