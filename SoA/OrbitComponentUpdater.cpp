#include "stdafx.h"
#include "OrbitComponentUpdater.h"
#include "SpaceSystem.h"

#include "Constants.h"

void OrbitComponentUpdater::update(SpaceSystem* spaceSystem, f64 time) {
    for (auto& it : spaceSystem->m_orbitCT) {
        auto& cmp = it.second;
        if (cmp.parentNpId) {
            calculatePosition(cmp, time, &spaceSystem->m_namePositionCT.getFromEntity(it.first),
                       &spaceSystem->m_namePositionCT.get(cmp.parentNpId));
        } else {
            calculatePosition(cmp, time, &spaceSystem->m_namePositionCT.getFromEntity(it.first));
        }
    }
}


f64 OrbitComponentUpdater::calculateOrbitalSpeed(SpaceSystem* spaceSystem, const OrbitComponent& oCmp,
                                                 const SphericalGravityComponent& sgCmp) {
    auto& npCmp = spaceSystem->m_namePositionCT.get(sgCmp.namePositionComponent);

    f64 distance;
    if (oCmp.parentNpId) {
        auto& pNpCmp = spaceSystem->m_namePositionCT.get(oCmp.parentNpId);
        distance = glm::length(npCmp.position - pNpCmp.position);
    } else {
        distance = glm::length(npCmp.position);
    }

    return sqrt(M_G * sgCmp.mass * (2.0 / distance - 1.0 / oCmp.semiMajor));
}

void OrbitComponentUpdater::calculatePosition(OrbitComponent& cmp, f64 time, NamePositionComponent* npComponent,
                                              NamePositionComponent* parentNpComponent /* = nullptr */) {

    /// Calculates position as a function of time
    /// http://en.wikipedia.org/wiki/Kepler%27s_laws_of_planetary_motion#Position_as_a_function_of_time
    f64 semiMajor3 = cmp.semiMajor * cmp.semiMajor * cmp.semiMajor;
    // 1. Calculate the mean anomoly
    //f64 meanAnomaly = (M_2_PI / cmp.orbitalPeriod) * time;
    f64 meanAnomaly = sqrt((M_G * cmp.totalMass) / semiMajor3) * time;

    // 2. Solve Kepler's equation to compute eccentric anomaly 
    // using Newton's method
    // http://www.jgiesen.de/kepler/kepler.html
#define ITERATIONS 5
    f64 eccentricAnomaly, F;
    eccentricAnomaly = meanAnomaly;
    F = eccentricAnomaly - cmp.eccentricity * sin(meanAnomaly) - meanAnomaly;
    for (int i = 0; i < ITERATIONS; i++) {
        eccentricAnomaly = eccentricAnomaly -
            F / (1.0 - cmp.eccentricity * cos(eccentricAnomaly));
        F = eccentricAnomaly -
            cmp.eccentricity * sin(eccentricAnomaly) - meanAnomaly;
    }
    // 3. Calculate true anomaly
    f64 trueAnomaly = 2.0 * atan2(sqrt(1.0 - cmp.eccentricity) * cos(eccentricAnomaly / 2.0),
                                  sqrt(1.0 + cmp.eccentricity) * sin(eccentricAnomaly / 2.0));

    // 4. Calculate radius
    f64 r = cmp.semiMajor * (1.0 - cmp.eccentricity * cmp.eccentricity) / (1.0 + cmp.eccentricity * cos(trueAnomaly));

    // http://www.stargazing.net/kepler/ellipse.html
    f64 i = M_PI_2; // Inclination TODO(Ben): This
    f64 o = 0.0; // Ascending node longitude http://en.wikipedia.org/wiki/Longitude_of_the_ascending_node
    f64 p = M_PI_2; // Periapsis longitude

    // Finally calculate position
    f64v3 position;
    f64 c = cos(trueAnomaly + p - o);
    f64 s = sin(trueAnomaly + p - o);
    f64 coso = cos(o);
    f64 cosi = cos(i);
    position.x = r * (coso * c - sin(o) * s * cosi);
    position.y = r * (coso * c - coso * s * cosi);
    position.z = r * (s * sin(i));

    // If this planet has a parent, add parent's position
    if (parentNpComponent) {
        npComponent->position = cmp.orientation * position + parentNpComponent->position;
    } else {
        npComponent->position = cmp.orientation * position;
    }
}