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

KEG_ENUM_INIT_BEGIN(BodyType, BodyType, e);
e->addValue("planet", BodyType::PLANET);
e->addValue("star", BodyType::STAR);
e->addValue("gasGiant", BodyType::GAS_GIANT);
KEG_ENUM_INIT_END

#define M_PER_KM 1000.0

class SystemBodyKegProperties {
public:
    nString name = "";
    nString parent = "";
    nString path = "";
    f64 semiMajor = 0.0;
    f64 period = 0.0;
    f64 startOrbit = 0.0;
};

KEG_TYPE_INIT_BEGIN_DEF_VAR(SystemBodyKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, STRING, parent);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, STRING, path);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, semiMajor);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, period);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, startOrbit);
KEG_TYPE_INIT_END

class PlanetKegProperties {
public:
    f64 diameter;
    f64 density;
    f64 mass;
    f64v3 axis;
    f64 angularSpeed;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(PlanetKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, diameter);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, mass);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_END

class StarKegProperties {
public:
    f64 surfaceTemperature; ///< temperature in kelvin
    f64 diameter;
    f64 density;
    f64 mass;
    f64v3 axis;
    f64 angularSpeed;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(StarKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, surfaceTemperature);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, diameter);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, mass);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F32_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_END

class Binary {
public:

    bool containsBody(const SystemBody* body) {
        for (int i = 0; i < bodies.getLength(); i++) {
            if (bodies[i] == body->name) return true;
        }
        return false;
    }

    nString name;
    Array<const char*> bodies; ///< Temporary due to bug
    f64 mass = 0.0;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(Binary)
KEG_TYPE_INIT_ADD_MEMBER(Binary, STRING, name);
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("bodies", Keg::Value::array(offsetof(Binary, bodies), Keg::BasicType::C_STRING));
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
    nString propFile = filePath + "/SystemProperties.yml";
    loadSystemProperties(propFile.c_str());

    // Set up binary masses
    for (auto& it : m_binaries) {
        f64 mass = 0.0;
        Binary* bin = it.second;

        // Loop through all children
        for (int i = 0; i < bin->bodies.getLength(); i++) {
            // Find the body
            auto& body = m_systemBodies.find(std::string(bin->bodies[i]));
            if (body != m_systemBodies.end()) {
                // Get the mass
                mass += m_sphericalGravityCT.getFromEntity(body->second->entity->id).mass;
            }
        }
        it.second->mass = mass;
    }

    // Set up parent connections and orbits
    for (auto& it : m_systemBodies) {
        SystemBody* body = it.second;
        const nString& parent = body->parentName;
        if (parent.length()) {
            // Check for parent
            auto& p = m_systemBodies.find(parent);
            if (p != m_systemBodies.end()) {
                body->parent = p->second;
                
                // Calculate the orbit using parent mass
                calculateOrbit(body->entity->id,
                               m_sphericalGravityCT.getFromEntity(p->second->entity->id).mass);
            } else {
                auto& b = m_binaries.find(parent);
                if (b != m_binaries.end()) {
                    f64 mass = b->second->mass;
                    // If this body is part of the system, subtract it's mass
                    if (b->second->containsBody(it.second)) {
                        mass -= m_sphericalGravityCT.getFromEntity(body->entity->id).mass;
                    }
                    // Calculate the orbit using parent mass
                    calculateOrbit(body->entity->id, mass);
                }
            }
        }
    }
}

bool SpaceSystem::loadBodyProperties(const nString& filePath, const SystemBodyKegProperties* sysProps, SystemBody* body) {
    
    #define KEG_CHECK if (error != Keg::Error::NONE) { \
       fprintf(stderr, "Keg error %d for %s\n", (int)error, filePath); \
        return false; \
    }
    
    Keg::Error error;
    nString data;
    m_ioManager.readFileToString(filePath.c_str(), data);

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
            addPlanet(sysProps, &properties, body);
        } else if (type == "star") {
            StarKegProperties properties;
            error = Keg::parse(&properties, data.c_str(), "StarKegProperties");
            addStar(sysProps, &properties, body);
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

void SpaceSystem::addPlanet(const SystemBodyKegProperties* sysProps, const PlanetKegProperties* properties, SystemBody* body) {

    body->entity = new vcore::Entity(addEntity());
    const vcore::EntityID& id = body->entity->id;

    vcore::ComponentID npCmp = m_namePositionCT.add(id);
    vcore::ComponentID arCmp = m_axisRotationCT.add(id);
    vcore::ComponentID oCmp = m_orbitCT.add(id);
    vcore::ComponentID stCmp = m_sphericalTerrainCT.add(id);
    vcore::ComponentID sgCmp = m_sphericalGravityCT.add(id);

    f64v3 up(0.0, 1.0, 0.0);
    m_axisRotationCT.get(arCmp).init(properties->angularSpeed,
                                     0.0,
                                     quatBetweenVectors(up, glm::normalize(properties->axis)));

    m_sphericalTerrainCT.get(stCmp).init(properties->diameter / 2.0);

    m_sphericalGravityCT.get(sgCmp).init(properties->diameter / 2.0,
                                         properties->mass);

    //// Calculate mass
    //f64 volume = (4.0 / 3.0) * (M_PI)* pow(radius_, 3.0);
    //mass_ = properties.density * volume;
}

void SpaceSystem::addStar(const SystemBodyKegProperties* sysProps, const StarKegProperties* properties, SystemBody* body) {
    body->entity = new vcore::Entity(addEntity());
    const vcore::EntityID& id = body->entity->id;

    vcore::ComponentID npCmp = m_namePositionCT.add(id);
    vcore::ComponentID arCmp = m_axisRotationCT.add(id);
    vcore::ComponentID oCmp = m_orbitCT.add(id);
    vcore::ComponentID sgCmp = m_sphericalGravityCT.add(id);

    f64v3 up(0.0, 1.0, 0.0);
    m_axisRotationCT.get(arCmp).init(properties->angularSpeed,
                                     0.0,
                                     quatBetweenVectors(up, glm::normalize(properties->axis)));

    m_sphericalGravityCT.get(sgCmp).init(properties->diameter / 2.0,
                                         properties->mass);

    auto& orbitCmp = m_orbitCT.get(oCmp);
    orbitCmp.semiMajor = sysProps->semiMajor;
    orbitCmp.orbitalPeriod = sysProps->period;
    orbitCmp.currentOrbit = sysProps->startOrbit;
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
        // Parse based on the name
        if (name == "description") {
            m_systemDescription = name;
        } else if (name == "Binary") {
            // Binary systems
            Binary* newBinary = new Binary;
            Keg::Error err = Keg::parse((ui8*)newBinary, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Binary));
            if (err != Keg::Error::NONE) {
                fprintf(stderr, "Failed to parse node %s in %s\n", name.c_str(), filePath);
                return false;
            }

            m_binaries[newBinary->name] = newBinary;
        } else {
            // We assume this is just a generic SystemBody
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
            loadBodyProperties(properties.path.c_str(), &properties, body);
            m_systemBodies[name] = body;
        }
    }

    return true;
}

void SpaceSystem::calculateOrbit(vcore::EntityID entity, f64 parentMass) {
    OrbitComponent& orbitC = m_orbitCT.getFromEntity(entity);
    f64 per = orbitC.orbitalPeriod;
    f64 mass = m_sphericalGravityCT.getFromEntity(entity).mass;
    orbitC.semiMinor = pow((per * per) / (4.0 / (M_PI * M_PI)) * M_G *
                        (mass + parentMass), 1.0 / 3.0) / M_PER_KM;
    // Generate the ellipse shape for rendering
    orbitC.generateOrbitEllipse();
}
