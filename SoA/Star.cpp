#include "stdafx.h"
#include "Star.h"


Star::Star() {
}


Star::~Star() {
}

void Star::update(f64 time) {
    // Call base update
    IOrbitalBody::update(time);
}

void Star::draw(const Camera* camera) {
    throw std::logic_error("The method or operation is not implemented.");
}
