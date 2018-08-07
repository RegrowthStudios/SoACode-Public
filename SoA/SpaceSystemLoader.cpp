#include "stdafx.h"
#include "SpaceSystemLoader.h"

#include "Constants.h"
#include "Errors.h"
#include "SoaOptions.h"
#include "OrbitComponentUpdater.h"
#include "PlanetGenData.h"
#include "PlanetGenLoader.h"
#include "ProgramGenDelegate.h"
#include "SoAState.h"
#include "SpaceSystemAssemblages.h"
#include "SpaceSystemLoadStructs.h"

#include <Vorb/RPC.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/Texture.h>
#include <Vorb/io/Keg.h>
#include <Vorb/ui/GameWindow.h>

void SpaceSystemLoader::init(const SoaState* soaState) {
    m_soaState = soaState;
    m_spaceSystem = soaState->spaceSystem;
    m_ioManager = soaState->systemIoManager;
    m_threadpool = soaState->threadPool;
    m_bodyLoader.init(m_ioManager);
}

void SpaceSystemLoader::loadStarSystem(const nString& path) {
    m_ioManager->setSearchDirectory((path + "/").c_str());

    // Load the path color scheme
    loadPathColors();

    // Load the system
    loadSystemProperties();

    // Set up binary masses
    initBinaries();

    // Set up parent connections and orbits
    initOrbits();
}

// Only used in SoaEngine::loadPathColors
struct PathColorKegProps {
    ui8v4 base = ui8v4(0);
    ui8v4 hover = ui8v4(0);
};
KEG_TYPE_DEF_SAME_NAME(PathColorKegProps, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, PathColorKegProps, base, UI8_V4);
    KEG_TYPE_INIT_ADD_MEMBER(kt, PathColorKegProps, hover, UI8_V4);
}

bool SpaceSystemLoader::loadPathColors() {
    nString data;
    if (!m_ioManager->readFileToString("PathColors.yml", data)) {
        pError("Couldn't find " + m_ioManager->getSearchDirectory().getString() + "/PathColors.yml");
    }

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        fprintf(stderr, "Failed to load %s\n", (m_ioManager->getSearchDirectory().getString() + "/PathColors.yml").c_str());
        context.reader.dispose();
        return false;
    }

    bool goodParse = true;
    auto f = makeFunctor([&](Sender, const nString& name, keg::Node value) {
        PathColorKegProps props;
        keg::Error err = keg::parse((ui8*)&props, value, context, &KEG_GLOBAL_TYPE(PathColorKegProps));
        if (err != keg::Error::NONE) {
            fprintf(stderr, "Failed to parse node %s in PathColors.yml\n", name.c_str());
            goodParse = false;
        }
        f32v4 base = f32v4(props.base) / 255.0f;
        f32v4 hover = f32v4(props.hover) / 255.0f;
        m_spaceSystem->pathColorMap[name] = std::make_pair(base, hover);
    });

    context.reader.forAllInMap(node, f);
    delete f;
    context.reader.dispose();
    return goodParse;
}

bool SpaceSystemLoader::loadSystemProperties() {
    nString data;
    if (!m_ioManager->readFileToString("SystemProperties.yml", data)) {
        pError("Couldn't find " + m_ioManager->getSearchDirectory().getString() + "/SystemProperties.yml");
    }

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        fprintf(stderr, "Failed to load %s\n", (m_ioManager->getSearchDirectory().getString() + "/SystemProperties.yml").c_str());
        context.reader.dispose();
        return false;
    }

    bool goodParse = true;
    auto f = makeFunctor([this, &goodParse, &context](Sender, const nString& name, keg::Node value) {
        // Parse based on the name
        if (name == "description") {
            m_spaceSystem->systemDescription = keg::convert<nString>(value);
        } else if (name == "age") {
            m_spaceSystem->age = keg::convert<f32>(value);
        } else {
            SystemOrbitProperties properties;
            keg::Error err = keg::parse((ui8*)&properties, value, context, &KEG_GLOBAL_TYPE(SystemOrbitProperties));
            if (err != keg::Error::NONE) {
                fprintf(stderr, "Failed to parse node %s in SystemProperties.yml\n", name.c_str());
                goodParse = false;
            }

            // Allocate the body
            SystemBody* body = new SystemBody;
            body->name = name;
            body->parentName = properties.par;
            body->properties = properties;
            if (properties.path.size()) {
                m_bodyLoader.loadBody(m_soaState, properties.path, &properties, body, value);
                m_bodyLookupMap[body->name] = body->entity;
            } else {
                // Make default orbit (used for barycenters)
                SpaceSystemAssemblages::createOrbit(m_spaceSystem, &properties, body, 0.0);
            }
            if (properties.type == SpaceObjectType::BARYCENTER) {
                m_barycenters[name] = body;
            }
            m_systemBodies[name] = body;
        }
    });
    context.reader.forAllInMap(node, f);
    delete f;
    context.reader.dispose();
    return goodParse;
}

void SpaceSystemLoader::initBinaries() {
    for (auto& it : m_barycenters) {
        SystemBody* bary = it.second;

        initBinary(bary);
    }
}

void SpaceSystemLoader::initBinary(SystemBody* bary) {
    // Don't update twice
    if (bary->isBaryCalculated) return;
    bary->isBaryCalculated = true;

    // Need two components or its not a binary
    if (bary->properties.comps.size() != 2) return;

    // A component
    auto bodyA = m_systemBodies.find(std::string(bary->properties.comps[0]));
    if (bodyA == m_systemBodies.end()) return;
    auto& aProps = bodyA->second->properties;

    // B component
    auto bodyB = m_systemBodies.find(std::string(bary->properties.comps[1]));
    if (bodyB == m_systemBodies.end()) return;
    auto& bProps = bodyB->second->properties;

    { // Set orbit parameters relative to A component
        bProps.ref = bodyA->second->name;
        bProps.td = 1.0f;
        bProps.tf = 1.0f;
        bProps.e = aProps.e;
        bProps.i = aProps.i;
        bProps.n = aProps.n;
        bProps.p = aProps.p + 180.0;
        bProps.a = aProps.a;
        auto& oCmp = m_spaceSystem->orbit.getFromEntity(bodyB->second->entity);
        oCmp.e = bProps.e;
        oCmp.i = bProps.i * DEG_TO_RAD;
        oCmp.p = bProps.p * DEG_TO_RAD;
        oCmp.o = bProps.n * DEG_TO_RAD;
        oCmp.startMeanAnomaly = bProps.a * DEG_TO_RAD;
    }

    // Get the A mass
    auto& aSgCmp = m_spaceSystem->sphericalGravity.getFromEntity(bodyA->second->entity);
    f64 massA = aSgCmp.mass;
    // Recurse if child is a non-constructed binary
    if (massA == 0.0) {
        initBinary(bodyA->second);
        massA = aSgCmp.mass;
    }

    // Get the B mass
    auto& bSgCmp = m_spaceSystem->sphericalGravity.getFromEntity(bodyB->second->entity);
    f64 massB = bSgCmp.mass;
    // Recurse if child is a non-constructed binary
    if (massB == 0.0) {
        initBinary(bodyB->second);
        massB = bSgCmp.mass;
    }

    // Set the barycenter mass
    bary->mass = massA + massB;

    auto& barySgCmp = m_spaceSystem->sphericalGravity.getFromEntity(bary->entity);
    barySgCmp.mass = bary->mass;

    { // Calculate A orbit
        SystemBody* body = bodyA->second;
        body->parent = bary;
        bary->children.push_back(body);
        f64 massRatio = massB / (massA + massB);
        calculateOrbit(body->entity,
                       barySgCmp.mass,
                       body, massRatio);
    }

    { // Calculate B orbit
        SystemBody* body = bodyB->second;
        body->parent = bary;
        bary->children.push_back(body);
        f64 massRatio = massA / (massA + massB);
        calculateOrbit(body->entity,
                       barySgCmp.mass,
                       body, massRatio);
    }

    { // Set orbit colors from A component
        auto& oCmp = m_spaceSystem->orbit.getFromEntity(bodyA->second->entity);
        auto& baryOCmp = m_spaceSystem->orbit.getFromEntity(bary->entity);
        baryOCmp.pathColor[0] = oCmp.pathColor[0];
        baryOCmp.pathColor[1] = oCmp.pathColor[1];
    }
}

void recursiveInclinationCalc(OrbitComponentTable& ct, SystemBody* body, f64 inclination) {
    for (auto& c : body->children) {
        OrbitComponent& orbitC = ct.getFromEntity(c->entity);
        orbitC.i += inclination;
        recursiveInclinationCalc(ct, c, orbitC.i);
    }
}

void SpaceSystemLoader::initOrbits() {
    // Set parent connections
    for (auto& it : m_systemBodies) {
        SystemBody* body = it.second;
        const nString& parent = body->parentName;
        if (parent.length()) {
            // Check for parent
            auto p = m_systemBodies.find(parent);
            if (p != m_systemBodies.end()) {
                // Set up parent connection
                body->parent = p->second;
                p->second->children.push_back(body);
            }
        }
    }

    // Child propagation for inclination
    // TODO(Ben): Do this right
    /* for (auto& it : pr.systemBodies) {
    SystemBody* body = it.second;
    if (!body->parent) {
    recursiveInclinationCalc(pr.spaceSystem->m_orbitCT, body,
    pr.spaceSystem->m_orbitCT.getFromEntity(body->entity).i);
    }
    }*/

    // Finally, calculate the orbits
    for (auto& it : m_systemBodies) {
        SystemBody* body = it.second;
        // Calculate the orbit using parent mass
        if (body->parent) {
            calculateOrbit(body->entity,
                           m_spaceSystem->sphericalGravity.getFromEntity(body->parent->entity).mass,
                           body);
        }
    }
}

void SpaceSystemLoader::computeRef(SystemBody* body) {
    if (!body->properties.ref.empty()) {
        OrbitComponent& orbitC = m_spaceSystem->orbit.getFromEntity(body->entity);
        // Find reference body
        auto it = m_systemBodies.find(body->properties.ref);
        if (it != m_systemBodies.end()) {
            SystemBody* ref = it->second;
            // Recursively compute ref if needed
            if (!ref->hasComputedRef) computeRef(ref);
            // Calculate period using reference body
            orbitC.t = ref->properties.t * body->properties.tf / body->properties.td;
            body->properties.t = orbitC.t;
            // Handle trojans
            if (body->properties.trojan == TrojanType::L4) {
                body->properties.a = ref->properties.a + 60.0f;
                orbitC.startMeanAnomaly = body->properties.a * DEG_TO_RAD;
            } else if (body->properties.trojan == TrojanType::L5) {
                body->properties.a = ref->properties.a - 60.0f;
                orbitC.startMeanAnomaly = body->properties.a * DEG_TO_RAD;
            }
        } else {
            fprintf(stderr, "Failed to find ref body %s\n", body->properties.ref.c_str());
        }
    }
    body->hasComputedRef = true;
}

void SpaceSystemLoader::calculateOrbit(vecs::EntityID entity, f64 parentMass,
                               SystemBody* body, f64 binaryMassRatio /* = 0.0 */) {
    OrbitComponent& orbitC = m_spaceSystem->orbit.getFromEntity(entity);

    // If the orbit was already calculated, don't do it again.
    if (orbitC.isCalculated) return;
    orbitC.isCalculated = true;

    // Provide the orbit component with it's parent
    m_spaceSystem->orbit.getFromEntity(body->entity).parentOrbId =
        m_spaceSystem->orbit.getComponentID(body->parent->entity);

    computeRef(body);

    f64 t = orbitC.t;
    auto& sgCmp = m_spaceSystem->sphericalGravity.getFromEntity(entity);
    f64 mass = sgCmp.mass;
    f64 diameter = sgCmp.radius * 2.0;

    if (binaryMassRatio > 0.0) { // Binary orbit
        orbitC.a = pow((t * t) * M_G * parentMass /
                       (4.0 * (M_PI * M_PI)), 1.0 / 3.0) * KM_PER_M * binaryMassRatio;
    } else { // Regular orbit
        // Calculate semi-major axis
        orbitC.a = pow((t * t) * M_G * (mass + parentMass) /
                       (4.0 * (M_PI * M_PI)), 1.0 / 3.0) * KM_PER_M;
    }

    // Calculate semi-minor axis
    orbitC.b = orbitC.a * sqrt(1.0 - orbitC.e * orbitC.e);

    // Set parent pass
    orbitC.parentMass = parentMass;

    // TODO(Ben): Doesn't work right for binaries due to parentMass
    { // Check tidal lock
        f64 ns = log10(0.003 * pow(orbitC.a, 6.0) * pow(diameter + 500.0, 3.0) / (mass * orbitC.parentMass) * (1.0 + (f64)1e20 / (mass + orbitC.parentMass)));
        if (ns < 0) {
            // It is tidally locked so lock the rotational period
            m_spaceSystem->axisRotation.getFromEntity(entity).period = t;
        }
    }

    { // Make the ellipse mesh with stepwise simulation
        OrbitComponentUpdater updater;
        static const int NUM_VERTS = 2880;
        orbitC.verts.resize(NUM_VERTS + 1);
        f64 timePerDeg = orbitC.t / (f64)NUM_VERTS;
        NamePositionComponent& npCmp = m_spaceSystem->namePosition.get(orbitC.npID);
        f64v3 startPos = npCmp.position;
        for (int i = 0; i < NUM_VERTS; i++) {

            if (orbitC.parentOrbId) {
                OrbitComponent* pOrbC = &m_spaceSystem->orbit.get(orbitC.parentOrbId);
                updater.updatePosition(orbitC, i * timePerDeg, &npCmp,
                                       pOrbC,
                                       &m_spaceSystem->namePosition.get(pOrbC->npID));
            } else {
                updater.updatePosition(orbitC, i * timePerDeg, &npCmp);
            }

            OrbitComponent::Vertex vert;
            vert.position = npCmp.position;
            vert.angle = 1.0f - (f32)i / (f32)NUM_VERTS;
            orbitC.verts[i] = vert;
        }
        orbitC.verts.back() = orbitC.verts.front();
        npCmp.position = startPos;
    }
}
