#include "stdafx.h"
#include "OrbitComponentUpdater.h"
#include "SpaceSystem.h"

#include "Constants.h"
#include "soaUtils.h"

void OrbitComponentUpdater::update(SpaceSystem* spaceSystem, f64 time) {
    for (auto& it : spaceSystem->m_orbitCT) {
        auto& cmp = it.second;
        if (cmp.parentOrbId) {
            OrbitComponent* pOrbC = &spaceSystem->m_orbitCT.get(cmp.parentOrbId);
            calculatePosition(cmp, time, &spaceSystem->m_namePositionCT.getFromEntity(it.first),
                              pOrbC,
                              &spaceSystem->m_namePositionCT.get(pOrbC->npID));
        } else {
            calculatePosition(cmp, time, &spaceSystem->m_namePositionCT.getFromEntity(it.first));
        }
    }
}

void OrbitComponentUpdater::calculatePosition(OrbitComponent& cmp, f64 time, NamePositionComponent* npComponent,
                                              OrbitComponent* parentOrbComponent /* = nullptr */,
                                              NamePositionComponent* parentNpComponent /* = nullptr */) {

    /// Calculates position as a function of time
    /// http://en.wikipedia.org/wiki/Kepler%27s_laws_of_planetary_motion#Position_as_a_function_of_time
    // 1. Calculate the mean anomaly
    f64 meanAnomaly = (M_2_PI / cmp.t) * time;

    // 2. Solve Kepler's equation to compute eccentric anomaly 
    // using Newton's method
    // http://www.jgiesen.de/kepler/kepler.html
#define ITERATIONS 3
    f64 eccentricAnomaly, F;
    eccentricAnomaly = meanAnomaly;
    F = eccentricAnomaly - cmp.e * sin(meanAnomaly) - meanAnomaly;
    for (int n = 0; n < ITERATIONS; n++) {
        eccentricAnomaly = eccentricAnomaly -
            F / (1.0 - cmp.e * cos(eccentricAnomaly));
        F = eccentricAnomaly -
            cmp.e * sin(eccentricAnomaly) - meanAnomaly;
    }
    // 3. Calculate true anomaly
    f64 v = 2.0 * atan2(sqrt(1.0 - cmp.e) * cos(eccentricAnomaly / 2.0),
                                  sqrt(1.0 + cmp.e) * sin(eccentricAnomaly / 2.0));

    // 4. Calculate radius
    // http://www.stargazing.net/kepler/ellipse.html
    f64 r = cmp.a * (1.0 - cmp.e * cmp.e) / (1.0 + cmp.e * cos(v));
    
    f64 i = 0.0; // Inclination TODO(Ben): This
    f64 o = 0.0; // Ascending node longitude 
    f64 p = 0.0; // Periapsis longitude
    f64 w = cmp.p - cmp.o;

    // Finally calculate position
    f64v3 position;
    f64 cosv = cos(v + p - o);
    f64 sinv = sin(v + p - o);
    f64 coso = cos(o);
    f64 sino = sin(o);
    f64 cosi = cos(i);
    f64 sini = sin(i);
    position.x = r * (coso * cosv - sino * sinv * cosi);
    position.y = r * (sinv * sini);
    position.z = r * (sino * cosv + coso * sinv * cosi);

    // Calculate velocity
    f64 g = sqrt(M_G * KM_PER_M * cmp.parentMass * (2.0 / r - 1.0 / cmp.a)) * KM_PER_M;
    f64 sinwv = sin(w + v);
    cmp.relativeVelocity.x = -g * sinwv * cosi;
    cmp.relativeVelocity.y = g * sinwv * sini;
    cmp.relativeVelocity.z = g * cos(w + v);

    // If this planet has a parent, add parent's position
    if (parentNpComponent) {
        //cmp.relativeVelocity = 
        npComponent->position = position + parentNpComponent->position;
    } else {
        npComponent->position = position;
    }
}
