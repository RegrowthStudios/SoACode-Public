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

enum class BodyType {
    NONE,
    PLANET,
    STAR,
    GAS_GIANT
};

KEG_ENUM_INIT_BEGIN(BodyType, BodyType, e);
e->addValue("planet", BodyType::PLANET);
e->addValue("star", BodyType::STAR);
e->addValue("gasGiant", BodyType::GAS_GIANT);
KEG_ENUM_INIT_END

class SystemBodyKegProperties {
public:
    nString name = "";
    nString parent = "";
    nString path = "";
    f64 semiMajor = 0.0;
    f32v3 semiMajorDir;
    f64 semiMinor = 0.0;
    f32v3 semiMinorDir;
};

KEG_TYPE_INIT_BEGIN_DEF_VAR(SystemBodyKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, STRING, parent);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, STRING, path);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F32_V3, semiMajorDir);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, semiMajor);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F32_V3, semiMinorDir);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, semiMinor);
KEG_TYPE_INIT_END

class PlanetKegProperties {
public:
    f64 radius;
    f64 density;
    f64v3 axis;
    f64 angularSpeed;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(PlanetKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, radius);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_END

class StarKegProperties {
public:
    f64 surfaceTemperature;
    f64 radius;
    f64 density;
    f64v3 axis;
    f64 angularSpeed;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(StarKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, surfaceTemperature);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, radius);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F32_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_END

class GasGiantKegProperties {
public:

};
KEG_TYPE_INIT_BEGIN_DEF_VAR(GasGiantKegProperties)
KEG_TYPE_INIT_END

SpaceSystem::SpaceSystem() : vcore::ECS() {
    // Add in component tables
    addComponentTable(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, &m_namePositionCT);
    addComponentTable(SPACE_SYSTEM_CT_AXISROTATION_NAME, &m_axisRotationCT);
    addComponentTable(SPACE_SYSTEM_CT_ORBIT_NAME, &m_orbitCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, &m_sphericalTerrainCT);
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

void SpaceSystem::addSolarSystem(const nString& filePath) {
    // Load the properties file
    loadSystemProperties(filePath.c_str());

    // Set up parent connections
    for (auto& it : m_systemBodies) {
        const nString& parent = it.second->parentName;
        if (parent.length()) {
            auto& p = m_systemBodies.find(parent);
            if (p != m_systemBodies.end()) {
                it.second->parent = p->second;
            }
        }
    }
}

bool SpaceSystem::loadBodyProperties(const nString& filePath, SystemBody* body) {
    
    #define KEG_CHECK if (error != Keg::Error::NONE) { \
       fprintf(stderr, "Keg error %d for %s\n", (int)error, filePath); \
        return false; \
    }
    
    Keg::Error error;
    nString data;
    m_ioManager.readFileToString(filePath, data);

    YAML::Node node = YAML::Load(data.c_str());
    if (node.IsNull() || !node.IsMap()) {
        return false;
    }

    for (auto& kvp : node) {
        nString type = kvp.first.as<nString>();
        // Parse based on type
        if (type == "planet") {
            PlanetKegProperties properties;
            error = Keg::parse(&properties, data.c_str(), "PlanetKegProperties");
            KEG_CHECK;
            addPlanet(&properties, body);
        } else if (type == "star") {
            StarKegProperties properties;
            error = Keg::parse(&properties, data.c_str(), "StarKegProperties");
            addStar(&properties, body);
            KEG_CHECK;
        } else if (type == "gasGiant") {
            GasGiantKegProperties properties;
            error = Keg::parse(&properties, data.c_str(), "GasGiantKegProperties");
            KEG_CHECK;
        }

        //Only parse the first
        break;
    }

}

void SpaceSystem::addPlanet(const PlanetKegProperties* properties, SystemBody* body) {

    vcore::Entity newEntity = addEntity();
    body->entity = newEntity;

    vcore::ComponentID npCmp = m_namePositionCT.add(newEntity.id);
    vcore::ComponentID arCmp = m_axisRotationCT.add(newEntity.id);
    vcore::ComponentID oCmp = m_orbitCT.add(newEntity.id);
    vcore::ComponentID stCmp = m_sphericalTerrainCT.add(newEntity.id);

    f64v3 up(0.0, 1.0, 0.0);
    m_axisRotationCT.get(arCmp).init(properties->angularSpeed,
                                     0.0,
                                     quatBetweenVectors(up, glm::normalize(properties->axis)));

    m_sphericalTerrainCT.get(stCmp).init(properties->radius);

    //// Calculate mass
    //f64 volume = (4.0 / 3.0) * (M_PI)* pow(radius_, 3.0);
    //mass_ = properties.density * volume;
}

void SpaceSystem::addStar(const StarKegProperties* properties, SystemBody* body) {
    vcore::Entity newEntity = addEntity();
    body->entity = newEntity;

    vcore::ComponentID npCmp = m_namePositionCT.add(newEntity.id);
    vcore::ComponentID arCmp = m_axisRotationCT.add(newEntity.id);
    vcore::ComponentID oCmp = m_orbitCT.add(newEntity.id);

    f64v3 up(0.0, 1.0, 0.0);
    m_axisRotationCT.get(arCmp).init(properties->angularSpeed,
                                     0.0,
                                     quatBetweenVectors(up, glm::normalize(properties->axis));
}

bool SpaceSystem::loadSystemProperties(const cString filePath) {
    nString data;
    m_ioManager.readFileToString(filePath, data);
    
    YAML::Node node = YAML::Load(data.c_str());
    if (node.IsNull() || !node.IsMap()) {
        return false;
    }

    for (auto& kvp : node) {
        nString name = kvp.first.as<nString>();
        if (name == "description") {
            m_systemDescription = name;
        } else {
            SystemBodyKegProperties properties;
            Keg::Error err = Keg::parse((ui8*)&properties, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(SystemBodyKegProperties));
            if (err != Keg::Error::NONE) {
                fprintf(stderr, "Failed to parse node %s in %s\n", name.c_str(), filePath);
                return false;
            }

            if (properties.path.empty()) {
                fprintf(stderr, "Missing path: for node %s in %s\n", name.c_str(), filePath);
                return false;
            }
            // Allocate the body
            SystemBody* body = new SystemBody;
            body->name = name;
            body->parentName = properties.parent;
            loadBodyProperties(properties.path.c_str(), body);
            m_systemBodies[name] = body;
        }
    }

    return true;
}