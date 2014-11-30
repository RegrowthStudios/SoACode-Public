#include "stdafx.h"
#include "Planet.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

void Planet::update(double time) {
    // Call base update
    IOrbitalBody::update(time);
}

void Planet::draw() {
    throw std::logic_error("The method or operation is not implemented.");
}
