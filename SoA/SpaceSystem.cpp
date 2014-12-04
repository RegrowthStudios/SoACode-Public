#include "stdafx.h"

#include "IOManager.h"
#include "Keg.h"
#include "SpaceSystem.h"
#include "utils.h"
#include "Camera.h"
#include "DebugRenderer.h"

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

SpaceSystem::SpaceSystem() : vcore::ECS() {
    // Add in component tables
    addComponentTable(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, &m_namePositionCT);
    addComponentTable(SPACE_SYSTEM_CT_AXISROTATION_NAME, &m_axisRotationCT);
    addComponentTable(SPACE_SYSTEM_CT_ORBIT_NAME, &m_orbitCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, &m_sphericalTerrainCT);
}

void SpaceSystem::addPlanet(const nString& filePath) {

    PlanetKegProperties properties;
    loadPlanetProperties(filePath.c_str(), properties);

    vcore::Entity newEntity = addEntity();

    vcore::ComponentID npCmp = m_namePositionCT.add(newEntity.id);
    vcore::ComponentID arCmp = m_axisRotationCT.add(newEntity.id);
    vcore::ComponentID oCmp = m_orbitCT.add(newEntity.id);
    vcore::ComponentID stCmp = m_sphericalTerrainCT.add(newEntity.id);

    f64v3 up(0.0, 1.0, 0.0);
    m_axisRotationCT.get(arCmp).init(properties.angularSpeed,
                                     0.0,
                                     quatBetweenVectors(up, properties.axis));

    m_sphericalTerrainCT.get(stCmp).init(properties.radius);
    
    //// Calculate mass
    //f64 volume = (4.0 / 3.0) * (M_PI)* pow(radius_, 3.0);
    //mass_ = properties.density * volume;
}

void SpaceSystem::update(double time) {
    for (auto& cmp : m_axisRotationCT.getComponentList()) {
        cmp.second.update(time);
    }
}

void SpaceSystem::draw(const Camera* camera) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    static DebugRenderer debugRenderer;

    for (auto& entity : getEntities()) {

        float radius = m_sphericalTerrainCT.getFromEntity(entity.id).radius;

        debugRenderer.drawIcosphere(f32v3(0), radius, f32v4(1.0), 5);

        AxisRotationComponent& axisRotComp = m_axisRotationCT.getFromEntity(entity.id);

        f32m4 rotationMatrix = f32m4(glm::toMat4(axisRotComp.currentOrientation));

        f32m4 WVP = camera->getProjectionMatrix() * camera->getViewMatrix();

        debugRenderer.render(WVP, f32v3(camera->getPosition()), rotationMatrix);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool SpaceSystem::loadPlanetProperties(const cString filePath, PlanetKegProperties& result) {
    IOManager ioManager; // TODO: Pass in

    nString data;
    ioManager.readFileToString(filePath, data);
    if (data.length()) {
        Keg::Error error = Keg::parse(&result, data.c_str(), "PlanetKegProperties");
        if (error != Keg::Error::NONE) {
            fprintf(stderr, "Keg error %d for %s\n", (int)error, filePath);
            return false;
        }
    } else {
        fprintf(stderr, "Failed to load planet %s" ,filePath);
        return false;
    }
    return true;
}
