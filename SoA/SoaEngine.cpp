#include "stdafx.h"
#include "SoaEngine.h"

#include "DebugRenderer.h"
#include "GLProgramManager.h"
#include "MeshManager.h"
#include "PlanetLoader.h"
#include "SoaState.h"
#include "SpaceSystemFactories.h"
#include "SpaceSystemLoadStructs.h"

#include <Vorb/io/Keg.h>
#include <yaml-cpp/yaml.h>

#define M_PER_KM 1000.0

struct SpaceSystemLoadParams {
    SpaceSystem* spaceSystem = nullptr;
    vio::IOManager* ioManager = nullptr;
    nString dirPath;
    PlanetLoader* planetLoader = nullptr;

    std::map<nString, Binary*> binaries; ///< Contains all binary systems
    std::map<nString, SystemBody*> systemBodies; ///< Contains all system bodies
    std::map<nString, vcore::EntityID> bodyLookupMap;
};

bool SoaEngine::initState(OUT SoaState* state) {
    state->glProgramManager = std::make_unique<vg::GLProgramManager>();
    state->debugRenderer = std::make_unique<DebugRenderer>(state->glProgramManager.get());
    state->meshManager = std::make_unique<MeshManager>(state->glProgramManager.get());
    state->systemIoManager = std::make_unique<vio::IOManager>();
    state->planetLoader = std::make_unique<PlanetLoader>(state->systemIoManager.get());

    return true;
}

bool SoaEngine::loadSpaceSystem(OUT SoaState* state, const SpaceSystemLoadData& loadData) {
    AutoDelegatePool pool;
    vpath path = "SoASpace.log";
    vfile file;
    path.asFile(&file);
    vfstream fs = file.open(vio::FileOpenFlags::READ_WRITE_CREATE);
    pool.addAutoHook(&state->spaceSystem.onEntityAdded, [=] (Sender, vcore::EntityID eid) {
        fs.write("Entity added: %d\n", eid);
    });
    for (auto namedTable : state->spaceSystem.getComponents()) {
        auto table = state->spaceSystem.getComponentTable(namedTable.first);
        pool.addAutoHook(&table->onEntityAdded, [=] (Sender, vcore::ComponentID cid, vcore::EntityID eid) {
            fs.write("Component \"%s\" added: %d -> Entity %d\n", namedTable.first.c_str(), cid, eid);
        });
    }

    // TODO(Ben): This is temporary
    state->spaceSystem.init(state->glProgramManager.get());

    SpaceSystemLoadParams spaceSystemLoadParams;
    spaceSystemLoadParams.dirPath = loadData.filePath;
    spaceSystemLoadParams.spaceSystem = &state->spaceSystem;
    spaceSystemLoadParams.ioManager = state->systemIoManager.get();
    spaceSystemLoadParams.planetLoader = state->planetLoader.get();

    addSolarSystem(spaceSystemLoadParams);

    pool.dispose();
    return true;
}

bool SoaEngine::loadGameSystem(OUT SoaState* state, const GameSystemLoadData& loadData) {
    return true;
}

void SoaEngine::destroyAll(OUT SoaState* state) {
    destroyGameSystem(state);
    destroySpaceSystem(state);
}

void SoaEngine::destroyGameSystem(OUT SoaState* state) {

}

void SoaEngine::addSolarSystem(SpaceSystemLoadParams& pr) {
    pr.ioManager->setSearchDirectory((pr.dirPath + "/").c_str());

    // Load the system
    loadSystemProperties(pr);

    // Set up binary masses
    for (auto& it : pr.binaries) {
        f64 mass = 0.0;
        Binary* bin = it.second;

        // Loop through all children
        for (int i = 0; i < bin->bodies.getLength(); i++) {
            // Find the body
            auto& body = pr.systemBodies.find(std::string(bin->bodies[i]));
            if (body != pr.systemBodies.end()) {
                // Get the mass
                mass += pr.spaceSystem->m_sphericalGravityCT.getFromEntity(body->second->entity).mass;
            }
        }
        it.second->mass = mass;
    }

    // Set up parent connections and orbits
    for (auto& it : pr.systemBodies) {
        SystemBody* body = it.second;
        const nString& parent = body->parentName;
        if (parent.length()) {
            // Check for parent
            auto& p = pr.systemBodies.find(parent);
            if (p != pr.systemBodies.end()) {
                body->parent = p->second;

                // Provide the orbit component with it's parent
                pr.spaceSystem->m_orbitCT.getFromEntity(body->entity).parentNpId =
                    pr.spaceSystem->m_namePositionCT.getComponentID(body->parent->entity);


                // Calculate the orbit using parent mass
                calculateOrbit(pr.spaceSystem, body->entity,
                               pr.spaceSystem->m_sphericalGravityCT.getFromEntity(body->parent->entity).mass, false);
            } else {
                auto& b = pr.binaries.find(parent);
                if (b != pr.binaries.end()) {
                    f64 mass = b->second->mass;
                    // If this body is part of the system, subtract it's mass
                    if (b->second->containsBody(it.second)) {
                        // Calculate the orbit using parent mass
                        calculateOrbit(pr.spaceSystem, body->entity, mass, true);
                    } else {
                        // Calculate the orbit using parent mass
                        calculateOrbit(pr.spaceSystem, body->entity, mass, false);
                    }
                }
            }
        }
    }
}

bool SoaEngine::loadSystemProperties(SpaceSystemLoadParams& pr) {
    nString data;
    pr.ioManager->readFileToString("SystemProperties.yml", data);

    YAML::Node node = YAML::Load(data.c_str());
    if (node.IsNull() || !node.IsMap()) {
        fprintf(stderr, "Failed to load %s\n", (pr.dirPath + "/SystemProperties.yml").c_str());
        return false;
    }

    for (auto& kvp : node) {
        nString name = kvp.first.as<nString>();
        // Parse based on the name
        if (name == "description") {
            pr.spaceSystem->systemDescription = name;
        } else if (name == "Binary") {
            // Binary systems
            Binary* newBinary = new Binary;
            Keg::Error err = Keg::parse((ui8*)newBinary, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Binary));
            if (err != Keg::Error::NONE) {
                fprintf(stderr, "Failed to parse node %s in %s\n", name.c_str(), pr.dirPath.c_str());
                return false;
            }

            pr.binaries[newBinary->name] = newBinary;
        } else {
            // We assume this is just a generic SystemBody
            SystemBodyKegProperties properties;
            properties.pathColor = ui8v4(rand() % 255, rand() % 255, rand() % 255, 255);
            Keg::Error err = Keg::parse((ui8*)&properties, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(SystemBodyKegProperties));
            if (err != Keg::Error::NONE) {
                fprintf(stderr, "Failed to parse node %s in %s\n", name.c_str(), pr.dirPath.c_str());
                return false;
            }

            if (properties.path.empty()) {
                fprintf(stderr, "Missing path: for node %s in %s\n", name.c_str(), pr.dirPath.c_str());
                return false;
            }
            // Allocate the body
            SystemBody* body = new SystemBody;
            body->name = name;
            body->parentName = properties.parent;
            loadBodyProperties(pr, properties.path, &properties, body);
            pr.systemBodies[name] = body;
        }
    }

    return true;
}

bool SoaEngine::loadBodyProperties(SpaceSystemLoadParams& pr, const nString& filePath, const SystemBodyKegProperties* sysProps, SystemBody* body) {

#define KEG_CHECK if (error != Keg::Error::NONE) { \
       fprintf(stderr, "Keg error %d for %s\n", (int)error, filePath); \
        return false; \
    }

    Keg::Error error;
    nString data;
    pr.ioManager->readFileToString(filePath.c_str(), data);

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
                properties.planetGenData = pr.planetLoader->loadPlanet(properties.generation);
            } else {
                properties.planetGenData = pr.planetLoader->getDefaultGenData();
            }

            SpaceSystemFactories::createPlanet(pr.spaceSystem, sysProps, &properties, body);
        } else if (type == "star") {
            StarKegProperties properties;
            error = Keg::parse((ui8*)&properties, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(StarKegProperties));
            SpaceSystemFactories::createStar(pr.spaceSystem, sysProps, &properties, body);
            KEG_CHECK;
        } else if (type == "gasGiant") {
            GasGiantKegProperties properties;
            error = Keg::parse((ui8*)&properties, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(GasGiantKegProperties));
            SpaceSystemFactories::createGasGiant(pr.spaceSystem, sysProps, &properties, body);
            KEG_CHECK;
        }

        pr.bodyLookupMap[body->name] = body->entity;

        //Only parse the first
        break;
    }
    return true;
}

void SoaEngine::calculateOrbit(SpaceSystem* spaceSystem, vcore::EntityID entity, f64 parentMass, bool isBinary) {
    OrbitComponent& orbitC = spaceSystem->m_orbitCT.getFromEntity(entity);

    f64 per = orbitC.orbitalPeriod;
    f64 mass = spaceSystem->m_sphericalGravityCT.getFromEntity(entity).mass;
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

void SoaEngine::destroySpaceSystem(OUT SoaState* state) {

}
