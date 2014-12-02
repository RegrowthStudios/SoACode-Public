#include "stdafx.h"
#include "GasGiant.h"


GasGiant::GasGiant() {
}


GasGiant::~GasGiant() {
}

void GasGiant::update(f64 time) {
    // Call base update
    IOrbitalBody::update(time);
}

void GasGiant::draw(const Camera* camera) {
    throw std::logic_error("The method or operation is not implemented.");
}
