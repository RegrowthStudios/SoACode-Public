#include "stdafx.h"
#include "OrbitComponentUpdater.h"
#include "SpaceSystem.h"

#include "Constants.h"
#include "soaUtils.h"

void OrbitComponentUpdater::update(SpaceSystem* spaceSystem, f64 time) {
    for (auto& it : spaceSystem->orbit) {
        auto& cmp = it.second;
        if (cmp.parentOrbId) {
            OrbitComponent* pOrbC = &spaceSystem->orbit.get(cmp.parentOrbId);
            updatePosition(cmp, time, &spaceSystem->namePosition.get(cmp.npID),
                              pOrbC,
                              &spaceSystem->namePosition.get(pOrbC->npID));
        } else {
            updatePosition(cmp, time, &spaceSystem->namePosition.get(cmp.npID));
        }
    }
}

void OrbitComponentUpdater::updatePosition(OrbitComponent& cmp, f64 time, NamePositionComponent* npComponent,
                                              OrbitComponent* parentOrbComponent /* = nullptr */,
                                              NamePositionComponent* parentNpComponent /* = nullptr */) {
    if (cmp.a == 0.0) return;
    /// Calculates position as a function of time
    /// http://en.wikipedia.org/wiki/Kepler%27s_laws_of_planetary_motion#Position_as_a_function_of_time

    // 1. Calculate the mean anomaly
    f64 meanAnomaly = (M_2_PI / cmp.t) * time + cmp.startMeanAnomaly;
    cmp.currentMeanAnomaly = (f32)meanAnomaly;

    f64 v = calculateTrueAnomaly(meanAnomaly, cmp.e);
  
    // Calculate radius
    // http://www.stargazing.net/kepler/ellipse.html
    f64 r = cmp.a * (1.0 - cmp.e * cmp.e) / (1.0 + cmp.e * cos(v));
    
    f64 w = cmp.p - cmp.o; ///< Argument of periapsis

    // Calculate position
    f64v3 position;
    f64 cosv = cos(v + cmp.p - cmp.o);
    f64 sinv = sin(v + cmp.p - cmp.o);
    f64 coso = cos(cmp.o);
    f64 sino = sin(cmp.o);
    f64 cosi = cos(cmp.i);
    f64 sini = sin(cmp.i);
    position.x = r * (coso * cosv - sino * sinv * cosi);
    position.y = r * (sinv * sini);
    position.z = r * (sino * cosv + coso * sinv * cosi);

    // Calculate velocity
    f64 g = sqrt(M_G * KM_PER_M * cmp.parentMass * (2.0 / r - 1.0 / cmp.a)) * KM_PER_M;
    f64 sinwv = sin(w + v);
    cmp.relativeVelocity.x = -g * sinwv * cosi;
    cmp.relativeVelocity.y = g * sinwv * sini;
    cmp.relativeVelocity.z = g * cos(w + v);

    // If this planet has a parent, make it parent relative
    if (parentOrbComponent) {
        cmp.velocity = parentOrbComponent->velocity + cmp.relativeVelocity;
        npComponent->position = position + parentNpComponent->position;
    } else {
        cmp.velocity = cmp.relativeVelocity;
        npComponent->position = position;
    }
}

f64 OrbitComponentUpdater::calculateTrueAnomaly(f64 meanAnomaly, f64 e) {
    // 2. Solve Kepler's equation to compute eccentric anomaly 
    // using Newton's method
    // http://www.jgiesen.de/kepler/kepler.html
#define ITERATIONS 3
    f64 E; ///< Eccentric Anomaly
    f64 F;
    E = meanAnomaly;
    for (int n = 0; n < ITERATIONS; n++) {
        F = E - e * sin(E) - meanAnomaly;
        E -= F / (1.0 - e * cos(E));
    }
    // 3. Calculate true anomaly
    return atan2(sqrt(1.0 - e * e) * sin(E), cos(E) - e);
}
