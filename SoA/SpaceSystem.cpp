#include "stdafx.h"

#include "App.h"
#include "Camera.h"
#include "PlanetLoader.h"
#include "PlanetLoader.h"
#include "SpaceSystem.h"
#include "SphericalTerrainGenerator.h"
#include "SphericalTerrainMeshManager.h"
#include "GLProgramManager.h"

#include <Vorb/IOManager.h>
#include <Vorb/Keg.h>
#include <Vorb/TextureRecycler.hpp>
#include <Vorb/utils.h>

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
    nString parent = "";
    nString path = "";
    f64 eccentricity = 0.0;
    f64 period = 0.0;
    f64 startOrbit = 0.0;
    f64v3 orbitNormal = f64v3(1.0, 0.0, 0.0);
    ui8v4 pathColor = ui8v4(255);
};

KEG_TYPE_INIT_BEGIN_DEF_VAR(SystemBodyKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, STRING, parent);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, STRING, path);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, eccentricity);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, period);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64, startOrbit);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, F64_V3, orbitNormal);
KEG_TYPE_INIT_ADD_MEMBER(SystemBodyKegProperties, UI8_V4, pathColor);
KEG_TYPE_INIT_END

class PlanetKegProperties {
public:
    f64 diameter = 0.0;
    f64 density = 0.0;
    f64 mass = 0.0;
    f64v3 axis;
    f64 angularSpeed = 0.0;
    nString displayName = "";
    nString generation = "";
    PlanetGenData* planetGenData = nullptr;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(PlanetKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, diameter);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, mass);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, STRING, displayName);
KEG_TYPE_INIT_ADD_MEMBER(PlanetKegProperties, STRING, generation);
KEG_TYPE_INIT_END

class StarKegProperties {
public:
    f64 surfaceTemperature = 0.0; ///< temperature in kelvin
    f64 diameter = 0.0;
    f64 density = 0.0;
    f64 mass = 0.0;
    f64v3 axis;
    f64 angularSpeed = 0.0;
    nString displayName = "";
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(StarKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, surfaceTemperature);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, diameter);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, mass);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F32_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_ADD_MEMBER(StarKegProperties, STRING, displayName);
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
    f64 diameter = 0.0;
    f64 density = 0.0;
    f64 mass = 0.0;
    f64v3 axis;
    f64 angularSpeed = 0.0;
    nString displayName = "";
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(GasGiantKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F64, diameter);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F64, density);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F64, mass);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F32_V3, axis);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, F64, angularSpeed);
KEG_TYPE_INIT_ADD_MEMBER(GasGiantKegProperties, STRING, displayName);
KEG_TYPE_INIT_END

SpaceSystem::SpaceSystem() : vcore::ECS() {
    // Add in component tables
    addComponentTable(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, &m_namePositionCT);
    addComponentTable(SPACE_SYSTEM_CT_AXISROTATION_NAME, &m_axisRotationCT);
    addComponentTable(SPACE_SYSTEM_CT_ORBIT_NAME, &m_orbitCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, &m_sphericalTerrainCT);

    m_planetLoader = new PlanetLoader(&m_ioManager);
   
    #define MAX_NORMAL_MAPS 512U
    m_normalMapRecycler = new vg::TextureRecycler((ui32)PATCH_NORMALMAP_WIDTH,
                                                  (ui32)PATCH_NORMALMAP_WIDTH,
                                                  &SamplerState::POINT_CLAMP,
                                                  0,
                                                  vg::TextureInternalFormat::RGB8,
                                                  MAX_NORMAL_MAPS);
}

SpaceSystem::~SpaceSystem() {
    delete m_spriteBatch;
    delete m_spriteFont;
    delete m_planetLoader;
}

void SpaceSystem::init(vg::GLProgramManager* programManager) {
    m_programManager = programManager;
}

void SpaceSystem::update(double time, const f64v3& cameraPos, const Camera* voxelCamera /* = nullptr */) {
    m_mutex.lock();
    for (auto& it : m_axisRotationCT) {
        it.second.update(time);
    }

    // Update Spherical Terrain
    for (auto& it : m_sphericalTerrainCT) {
        it.second.update(cameraPos, &m_namePositionCT.getFromEntity(it.first),
                         &m_axisRotationCT.getFromEntity(it.first));
    }

    m_sphericalVoxelComponentUpdater.update(this, voxelCamera);

    // Update Orbits ( Do this last)
    m_orbitComponentUpdater.update(this, time);

    m_mutex.unlock();
}

void SpaceSystem::glUpdate() {
    for (auto& cmp : m_sphericalTerrainCT) {
        cmp.second.glUpdate();
    }
}

void SpaceSystem::addSolarSystem(const nString& dirPath) {
    m_dirPath = dirPath;
    m_ioManager.setSearchDirectory((m_dirPath + "/").c_str());

    // Load the system
    loadSystemProperties(dirPath);

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
                
                // Provide the orbit component with it's parent
                m_orbitCT.getFromEntity(body->entity->id).parentNpId =
                    m_namePositionCT.getComponentID(body->parent->entity->id);


                // Calculate the orbit using parent mass
                calculateOrbit(body->entity->id,
                               m_sphericalGravityCT.getFromEntity(body->parent->entity->id).mass, false);
            } else {
                auto& b = m_binaries.find(parent);
                if (b != m_binaries.end()) {
                    f64 mass = b->second->mass;
                    // If this body is part of the system, subtract it's mass
                    if (b->second->containsBody(it.second)) {
                        // Calculate the orbit using parent mass
                        calculateOrbit(body->entity->id, mass, true);
                    } else {
                        // Calculate the orbit using parent mass
                        calculateOrbit(body->entity->id, mass, false);
                    }
                }
            }
        }
    }
}

SphericalVoxelComponent* SpaceSystem::enableVoxelsOnTarget(const f64v3& gpos,
                                       vvox::VoxelMapData* startingMapData,
                                       const IOManager* saveFileIom /*= nullptr*/) {
    if (m_targetComponent == 0) return nullptr;

    // Make sure it isn't already enabled
    vcore::ComponentID cid = m_sphericalVoxelCT.getComponentID(m_targetEntity);
    if (cid != 0) return nullptr;

    // It needs a spherical terrain component
    cid = m_sphericalTerrainCT.getComponentID(m_targetEntity);
    if (cid == 0) return nullptr;

    // Add spherical voxel component
    vcore::ComponentID svCmp = m_sphericalVoxelCT.add(m_targetEntity);
    m_sphericalVoxelCT.get(svCmp).init(m_sphericalTerrainCT.get(cid).getSphericalTerrainData(), saveFileIom,
                                       m_sphericalTerrainCT.get(cid).getGenerator(),
                                       gpos, startingMapData);
    return &m_sphericalVoxelCT.get(svCmp);
}

void SpaceSystem::targetBody(const nString& name) {
    auto& it = m_bodyLookupMap.find(name);
    if (it != m_bodyLookupMap.end()) {
        m_targetEntity = it->second;
        m_targetComponent = m_namePositionCT.getComponentID(m_targetEntity);
    }
}

void SpaceSystem::targetBody(vcore::EntityID eid) {
    m_targetEntity = eid;
    m_targetComponent = m_namePositionCT.getComponentID(m_targetEntity);
}

void SpaceSystem::nextTarget() {
    auto& it = m_bodyLookupMap.find(m_namePositionCT.get(m_targetComponent).name);
    it++;
    if (it == m_bodyLookupMap.end()) it = m_bodyLookupMap.begin();
    m_targetEntity = it->second;
    m_targetComponent = m_namePositionCT.getComponentID(m_targetEntity);
}

void SpaceSystem::previousTarget() {
    auto& it = m_bodyLookupMap.find(m_namePositionCT.get(m_targetComponent).name);
    if (it == m_bodyLookupMap.begin()) it = m_bodyLookupMap.end();
    it--;
    m_targetEntity = it->second;
    m_targetComponent = m_namePositionCT.getComponentID(m_targetEntity);
}

f64v3 SpaceSystem::getTargetPosition() {
    m_mutex.lock();
    f64v3 pos = m_namePositionCT.get(m_targetComponent).position;
    m_mutex.unlock();
    return pos;
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
        std::cout << "Failed to load " + filePath;
        return false;
    }

    for (auto& kvp : node) {
        nString type = kvp.first.as<nString>();
        // Parse based on type
        if (type == "planet") {
            PlanetKegProperties properties;
            error = Keg::parse((ui8*)&properties, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(PlanetKegProperties));
            KEG_CHECK;

            // Use planet loader to load terrain and biomes
            if (properties.generation.length()) {
                properties.planetGenData = m_planetLoader->loadPlanet(properties.generation);
            } else {
                properties.planetGenData = m_planetLoader->getDefaultGenData();
            }

            addPlanet(sysProps, &properties, body);
        } else if (type == "star") {
            StarKegProperties properties;
            error = Keg::parse((ui8*)&properties, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(StarKegProperties));
            addStar(sysProps, &properties, body);
            KEG_CHECK;
        } else if (type == "gasGiant") {
            GasGiantKegProperties properties;
            error = Keg::parse((ui8*)&properties, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(GasGiantKegProperties));
            addGasGiant(sysProps, &properties, body);
            KEG_CHECK;
        }

        m_bodyLookupMap[body->name] = body->entity->id;

        //Only parse the first
        break;
    }
    return true;
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

    m_sphericalTerrainCT.get(stCmp).init(properties->diameter / 2.0,
                                         properties->planetGenData,
                                         m_programManager->getProgram("NormalMapGen"),
                                         m_normalMapRecycler);

    m_sphericalGravityCT.get(sgCmp).init(properties->diameter / 2.0,
                                         properties->mass);


    // Set the name
    m_namePositionCT.get(npCmp).name = body->name;

    setOrbitProperties(oCmp, sysProps);
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

    // Set the name
    m_namePositionCT.get(npCmp).name = body->name;

    setOrbitProperties(oCmp, sysProps);
}

void SpaceSystem::addGasGiant(const SystemBodyKegProperties* sysProps, const GasGiantKegProperties* properties, SystemBody* body) {
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

    // Set the name
    m_namePositionCT.get(npCmp).name = body->name;

    setOrbitProperties(oCmp, sysProps);
}

bool SpaceSystem::loadSystemProperties(const nString& dirPath) {
    nString data;
    m_ioManager.readFileToString("SystemProperties.yml", data);
    
    YAML::Node node = YAML::Load(data.c_str());
    if (node.IsNull() || !node.IsMap()) {
        fprintf(stderr, "Failed to load %s\n", (dirPath + "/SystemProperties.yml").c_str());
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
                fprintf(stderr, "Failed to parse node %s in %s\n", name.c_str(), dirPath.c_str());
                return false;
            }

            m_binaries[newBinary->name] = newBinary;
        } else {
            // We assume this is just a generic SystemBody
            SystemBodyKegProperties properties;
            properties.pathColor = ui8v4(rand() % 255, rand() % 255, rand() % 255, 255);
            Keg::Error err = Keg::parse((ui8*)&properties, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(SystemBodyKegProperties));
            if (err != Keg::Error::NONE) {
                fprintf(stderr, "Failed to parse node %s in %s\n", name.c_str(), dirPath.c_str());
                return false;
            }

            if (properties.path.empty()) {
                fprintf(stderr, "Missing path: for node %s in %s\n", name.c_str(), dirPath.c_str());
                return false;
            }
            // Allocate the body
            SystemBody* body = new SystemBody;
            body->name = name;
            body->parentName = properties.parent;
            loadBodyProperties(properties.path, &properties, body);
            m_systemBodies[name] = body;
        }
    }

    return true;
}

void SpaceSystem::calculateOrbit(vcore::EntityID entity, f64 parentMass, bool isBinary) {
    OrbitComponent& orbitC = m_orbitCT.getFromEntity(entity);

    f64 per = orbitC.orbitalPeriod;
    f64 mass = m_sphericalGravityCT.getFromEntity(entity).mass;
    if (isBinary) parentMass -= mass;
    orbitC.semiMajor = pow((per * per) / 4.0 / (M_PI * M_PI) * M_G *
                        (mass + parentMass), 1.0 / 3.0) / M_PER_KM;
    orbitC.semiMinor = orbitC.semiMajor *
        sqrt(1.0 - orbitC.eccentricity * orbitC.eccentricity);
    orbitC.totalMass = mass + parentMass;
    if (isBinary) {
      //  orbitC.r1 = 2.0 * orbitC.semiMajor * (1.0 - orbitC.eccentricity) *
      //      mass / (orbitC.totalMass);
        orbitC.r1 = orbitC.semiMajor;
    } else {
        orbitC.r1 = orbitC.semiMajor * (1.0 - orbitC.eccentricity);
    }
}

void SpaceSystem::setOrbitProperties(vcore::ComponentID cmp, const SystemBodyKegProperties* sysProps) {
    // Set basic properties
    auto& orbitCmp = m_orbitCT.get(cmp);
    orbitCmp.eccentricity = sysProps->eccentricity;
    orbitCmp.orbitalPeriod = sysProps->period;
    orbitCmp.pathColor = sysProps->pathColor;

    // Calculate orbit orientation from normal
    const f64v3 right(1.0, 0.0, 0.0);
    if (right == -sysProps->orbitNormal) {
        f64v3 eulers(0.0, M_PI, 0.0);
        orbitCmp.orientation = f64q(eulers);
    } else if (right != sysProps->orbitNormal) {
        orbitCmp.orientation = quatBetweenVectors(right, sysProps->orbitNormal);
    }
}
