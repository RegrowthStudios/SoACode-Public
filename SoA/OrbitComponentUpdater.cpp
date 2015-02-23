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
    f64 meanAnomaly = sqrt((M_G * cmp.totalMass) / semiMajor3) * time;

    // Solve Kepler's equation to compute eccentric anomaly 
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

    // Finally calculate position
    f64v3 position;
    position.x = cmp.semiMajor * (cos(eccentricAnomaly) - cmp.eccentricity);
    position.y = 0.0;
    position.z = cmp.semiMajor * sqrt(1.0 - cmp.eccentricity * cmp.eccentricity) *
        sin(eccentricAnomaly);

    // If this planet has a parent, add parent's position
    if (parentNpComponent) {
        npComponent->position = cmp.orientation * position + parentNpComponent->position;
    } else {
        npComponent->position = cmp.orientation * position;
    }
}