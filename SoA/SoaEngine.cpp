#include "stdafx.h"
#include "SoaEngine.h"

#include "BlockData.h"
#include "BlockPack.h"
#include "ChunkMeshManager.h"
#include "Constants.h"
#include "DebugRenderer.h"
#include "Errors.h"
#include "MeshManager.h"
#include "Options.h"
#include "OrbitComponentUpdater.h"
#include "PlanetData.h"
#include "PlanetLoader.h"
#include "ProgramGenDelegate.h"
#include "SoaState.h"
#include "SpaceSystemAssemblages.h"
#include "SpaceSystemLoadStructs.h"

#include <Vorb/RPC.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/io/keg.h>

#define M_PER_KM 1000.0

struct SpaceSystemLoadParams {
    SpaceSystem* spaceSystem = nullptr;
    vio::IOManager* ioManager = nullptr;
    nString dirPath;
    PlanetLoader* planetLoader = nullptr;
    vcore::RPCManager* glrpc = nullptr;

    std::map<nString, SystemBody*> barycenters; ///< Contains all barycenter objects
    std::map<nString, SystemBody*> systemBodies; ///< Contains all system bodies
    std::map<nString, vecs::EntityID> bodyLookupMap;
};

void SoaEngine::initState(SoaState* state) {
    state->debugRenderer = std::make_unique<DebugRenderer>();
    state->meshManager = std::make_unique<MeshManager>();
    state->chunkMeshManager = std::make_unique<ChunkMeshManager>();
    state->systemIoManager = std::make_unique<vio::IOManager>();
    state->texturePathResolver.init("Textures/TexturePacks/" + graphicsOptions.defaultTexturePack + "/",
                                    "Textures/TexturePacks/" + graphicsOptions.currTexturePack + "/");
   
}
// TODO: A vorb helper would be nice.
vg::ShaderSource createShaderSource(const vg::ShaderType& stage, const vio::IOManager& iom, const cString path, const cString defines = nullptr) {
    vg::ShaderSource src;
    src.stage = stage;
    if (defines) src.sources.push_back(defines);
    const cString code = iom.readFileToString(path);
    src.sources.push_back(code);
    return src;
}

bool SoaEngine::loadSpaceSystem(SoaState* state, const SpaceSystemLoadData& loadData, vcore::RPCManager* glrpc /* = nullptr */) {

    AutoDelegatePool pool;
    vpath path = "SoASpace.log";
    vfile file;
    path.asFile(&file);

    state->spaceSystem = std::make_unique<SpaceSystem>();
    state->planetLoader = std::make_unique<PlanetLoader>(state->systemIoManager.get());

    vfstream fs = file.open(vio::FileOpenFlags::READ_WRITE_CREATE);
    pool.addAutoHook(state->spaceSystem->onEntityAdded, [=] (Sender, vecs::EntityID eid) {
        fs.write("Entity added: %d\n", eid);
    });
    for (auto namedTable : state->spaceSystem->getComponents()) {
        auto table = state->spaceSystem->getComponentTable(namedTable.first);
        pool.addAutoHook(table->onEntityAdded, [=] (Sender, vecs::ComponentID cid, vecs::EntityID eid) {
            fs.write("Component \"%s\" added: %d -> Entity %d\n", namedTable.first.c_str(), cid, eid);
        });
    }

    // Load normal map gen shader
    ProgramGenDelegate gen;
    vio::IOManager iom;
    gen.initFromFile("NormalMapGen", "Shaders/Generation/NormalMap.vert", "Shaders/Generation/NormalMap.frag", &iom);

    if (glrpc) {
        glrpc->invoke(&gen.rpc, true);
    } else {
        gen.invoke(nullptr, nullptr);
    }

    if (gen.program == nullptr) {
        std::cerr << "Failed to load shader NormalMapGen with error: " << gen.errorMessage;
        return false;
    }
    /// Manage the program with a unique ptr
    state->spaceSystem->normalMapGenProgram = std::unique_ptr<vg::GLProgram>(gen.program);

    // Load system
    SpaceSystemLoadParams spaceSystemLoadParams;
    spaceSystemLoadParams.glrpc = glrpc;
    spaceSystemLoadParams.dirPath = loadData.filePath;
    spaceSystemLoadParams.spaceSystem = state->spaceSystem.get();
    spaceSystemLoadParams.ioManager = state->systemIoManager.get();
    spaceSystemLoadParams.planetLoader = state->planetLoader.get();

    addStarSystem(spaceSystemLoadParams);

    pool.dispose();
    return true;
}

bool SoaEngine::loadGameSystem(SoaState* state, const GameSystemLoadData& loadData) {
    // TODO(Ben): Implement
    state->gameSystem = std::make_unique<GameSystem>();
    return true;
}

void SoaEngine::setPlanetBlocks(SoaState* state) {
    SpaceSystem* ss = state->spaceSystem.get();
    for (auto& it : ss->m_sphericalTerrainCT) {
        auto& cmp = it.second;
        PlanetBlockInitInfo& blockInfo = cmp.planetGenData->blockInfo;
        // TODO(Ben): Biomes too!
        if (cmp.planetGenData) {
            // Set all block layers
            for (size_t i = 0; i < blockInfo.blockLayerNames.size(); i++) {
                ui16 blockID = Blocks[blockInfo.blockLayerNames[i]].ID;
                cmp.planetGenData->blockLayers[i].block = blockID;
            }
            // Clear memory
            std::vector<nString>().swap(blockInfo.blockLayerNames);
            // Set liquid block
            if (blockInfo.liquidBlockName.length()) {
                cmp.planetGenData->liquidBlock = Blocks[blockInfo.liquidBlockName].ID;
                nString().swap(blockInfo.liquidBlockName); // clear memory
            }
            // Set surface block
            if (blockInfo.surfaceBlockName.length()) {
                cmp.planetGenData->surfaceBlock = Blocks[blockInfo.surfaceBlockName].ID;
                nString().swap(blockInfo.surfaceBlockName); // clear memory
            }
        }
    }
}

void SoaEngine::destroyAll(SoaState* state) {
    state->debugRenderer.reset();
    state->meshManager.reset();
    state->systemIoManager.reset();
    destroyGameSystem(state);
    destroySpaceSystem(state);
}

void SoaEngine::destroyGameSystem(SoaState* state) {
    state->gameSystem.reset();
}

void SoaEngine::addStarSystem(SpaceSystemLoadParams& pr) {
    pr.ioManager->setSearchDirectory((pr.dirPath + "/").c_str());

    // Load the system
    loadSystemProperties(pr);

    // Set up binary masses
    initBinaries(pr);

    // Set up parent connections and orbits
    initOrbits(pr);
}

bool SoaEngine::loadSystemProperties(SpaceSystemLoadParams& pr) {
    nString data;
    if (!pr.ioManager->readFileToString("SystemProperties.yml", data)) {
        pError("Couldn't find " + pr.dirPath +  "/SystemProperties.yml");
    }

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        fprintf(stderr, "Failed to load %s\n", (pr.dirPath + "/SystemProperties.yml").c_str());
        context.reader.dispose();
        return false;
    }

    bool goodParse = true;
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& name, keg::Node value) {
        // Parse based on the name
        if (name == "description") {
            pr.spaceSystem->systemDescription = name;
        } else {
            SystemBodyKegProperties properties;
            keg::Error err = keg::parse((ui8*)&properties, value, context, &KEG_GLOBAL_TYPE(SystemBodyKegProperties));
            if (err != keg::Error::NONE) {
                fprintf(stderr, "Failed to parse node %s in %s\n", name.c_str(), pr.dirPath.c_str());
                goodParse = false;
            }

            // Allocate the body
            SystemBody* body = new SystemBody;
            body->name = name;
            body->parentName = properties.par;
            body->properties = properties;
            if (properties.path.size()) {
                loadBodyProperties(pr, properties.path, &properties, body);
            } else {
                SpaceSystemAssemblages::createOrbit(pr.spaceSystem, &properties, body);
            }
            if (properties.type == ObjectType::BARYCENTER) {
                pr.barycenters[name] = body;
            }
            pr.systemBodies[name] = body;
        }
    });
    context.reader.forAllInMap(node, f);
    delete f;
    context.reader.dispose();
    return goodParse;
}

bool SoaEngine::loadBodyProperties(SpaceSystemLoadParams& pr, const nString& filePath,
                                   const SystemBodyKegProperties* sysProps, SystemBody* body) {

#define KEG_CHECK \
    if (error != keg::Error::NONE) { \
        fprintf(stderr, "keg error %d for %s\n", (int)error, filePath); \
        goodParse = false; \
        return;  \
        }

    keg::Error error;
    nString data;
    pr.ioManager->readFileToString(filePath.c_str(), data);

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to load " + filePath;
        context.reader.dispose();
        return false;
    }

    bool goodParse = true;
    bool foundOne = false;
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& type, keg::Node value) {
        if (foundOne) return;

        // Parse based on type
        if (type == "planet") {
            PlanetKegProperties properties;
            error = keg::parse((ui8*)&properties, value, context, &KEG_GLOBAL_TYPE(PlanetKegProperties));
            KEG_CHECK;

            // Use planet loader to load terrain and biomes
            if (properties.generation.length()) {
                properties.planetGenData = pr.planetLoader->loadPlanet(properties.generation, pr.glrpc);
            } else {
                properties.planetGenData = pr.planetLoader->getRandomGenData(pr.glrpc);
            }

            // Set the radius for use later
            if (properties.planetGenData) {
                properties.planetGenData->radius = properties.diameter / 2.0;
            }

            SpaceSystemAssemblages::createPlanet(pr.spaceSystem, sysProps, &properties, body);
            body->type = BodyType::PLANET;
        } else if (type == "star") {
            StarKegProperties properties;
            error = keg::parse((ui8*)&properties, value, context, &KEG_GLOBAL_TYPE(StarKegProperties));
            KEG_CHECK;
            SpaceSystemAssemblages::createStar(pr.spaceSystem, sysProps, &properties, body);
            body->type = BodyType::STAR;
        } else if (type == "gasGiant") {
            GasGiantKegProperties properties;
            error = keg::parse((ui8*)&properties, value, context, &KEG_GLOBAL_TYPE(GasGiantKegProperties));
            KEG_CHECK;
            createGasGiant(pr, sysProps, &properties, body);
            body->type = BodyType::GAS_GIANT;
        }

        pr.bodyLookupMap[body->name] = body->entity;

        //Only parse the first
        foundOne = true;
    });
    context.reader.forAllInMap(node, f);
    delete f;
    context.reader.dispose();

    return goodParse;
}

void SoaEngine::initBinaries(SpaceSystemLoadParams& pr) {
    for (auto& it : pr.barycenters) {
        f64 mass = 0.0;
        SystemBody* bary = it.second;

        if (bary->properties.comps.size() != 2) return;
        // A component
        auto& bodyA = pr.systemBodies.find(std::string(bary->properties.comps[0]));
        if (bodyA == pr.systemBodies.end()) return;
        auto& aProps = bodyA->second->properties;
        // Get the mass
        mass += pr.spaceSystem->m_sphericalGravityCT.getFromEntity(bodyA->second->entity).mass;

        // B component
        auto& bodyB = pr.systemBodies.find(std::string(bary->properties.comps[1]));
        if (bodyB == pr.systemBodies.end()) return;
        auto& bProps = bodyB->second->properties;
        // Get the mass
        mass += pr.spaceSystem->m_sphericalGravityCT.getFromEntity(bodyB->second->entity).mass;
        // Use orbit parameters relative to A component
        bProps.ref = bodyA->second->name;
        bProps.td = 1.0f;
        bProps.tf = 1.0f;
        bProps.e = aProps.e;
        bProps.i = aProps.i;
        bProps.n = aProps.n;
        bProps.p = aProps.p + 180.0;
        bProps.a = aProps.a;
        auto& oCmp = pr.spaceSystem->m_orbitCT.getFromEntity(bodyB->second->entity);
        oCmp.e = bProps.e;
        oCmp.i = bProps.i * DEG_TO_RAD;
        oCmp.p = bProps.p * DEG_TO_RAD;
        oCmp.o = bProps.n * DEG_TO_RAD;
        oCmp.startTrueAnomaly = bProps.a * DEG_TO_RAD;
        
        // Set the barycenter mass
        it.second->mass = mass;
    }
}

void SoaEngine::initOrbits(SpaceSystemLoadParams& pr) {
    for (auto& it : pr.systemBodies) {
        SystemBody* body = it.second;
        const nString& parent = body->parentName;
        if (parent.length()) {
            // Check for parent
            auto& p = pr.systemBodies.find(parent);
            if (p != pr.systemBodies.end()) {
                body->parent = p->second;

                // Provide the orbit component with it's parent
                pr.spaceSystem->m_orbitCT.getFromEntity(body->entity).parentOrbId =
                    pr.spaceSystem->m_orbitCT.getComponentID(body->parent->entity);

                // Calculate the orbit using parent mass
                calculateOrbit(pr, body->entity,
                                pr.spaceSystem->m_sphericalGravityCT.getFromEntity(body->parent->entity).mass,
                                body, false);
              
            }
        }
    }
}

void SoaEngine::createGasGiant(SpaceSystemLoadParams& pr,
                               const SystemBodyKegProperties* sysProps,
                               const GasGiantKegProperties* properties,
                               SystemBody* body) {

    // Load the texture
    VGTexture colorMap = 0;
    if (properties->colorMap.size()) {
        vio::Path colorPath;
        if (!pr.ioManager->resolvePath(properties->colorMap, colorPath)) {
            fprintf(stderr, "Failed to resolve %s\n", properties->colorMap.c_str());
            return;
        }
        if (pr.glrpc) {
            vcore::RPC rpc;
            rpc.data.f = makeFunctor<Sender, void*>([&](Sender s, void* userData) {
                vg::BitmapResource b = vg::ImageIO().load(colorPath); 
                if (b.data) {
                    colorMap = vg::GpuMemory::uploadTexture(b.bytesUI8,
                                                            b.width, b.height,
                                                            &vg::SamplerState::LINEAR_CLAMP);
                    vg::ImageIO().free(b);
                } else {
                    fprintf(stderr, "Failed to load %s\n", properties->colorMap.c_str());
                }
            });
            pr.glrpc->invoke(&rpc, true);
        } else {
            vg::BitmapResource b = vg::ImageIO().load(colorPath);
            if (b.data) {
                colorMap = vg::GpuMemory::uploadTexture(b.bytesUI8,
                                                        b.width, b.height,
                                                        &vg::SamplerState::LINEAR_CLAMP);
                vg::ImageIO().free(b);
                
            } else {
                fprintf(stderr, "Failed to load %s\n", properties->colorMap.c_str());
                return;
            }
        }
    }
    SpaceSystemAssemblages::createGasGiant(pr.spaceSystem, sysProps, properties, body, colorMap);
}

void SoaEngine::calculateOrbit(SpaceSystemLoadParams& pr, vecs::EntityID entity, f64 parentMass,
                               const SystemBody* body, bool isBinary) {
    SpaceSystem* spaceSystem = pr.spaceSystem;
    OrbitComponent& orbitC = spaceSystem->m_orbitCT.getFromEntity(entity);

    if (!body->properties.ref.empty()) {
        auto it = pr.systemBodies.find(body->properties.ref);
        if (it != pr.systemBodies.end()) {
            SystemBody* ref = it->second;
            orbitC.t = ref->properties.t * body->properties.tf / body->properties.td;
        } else {
            fprintf(stderr, "Failed to find ref body %s\n", body->properties.ref.c_str());
        }
    }

    f64 per = orbitC.t;
    f64 mass = spaceSystem->m_sphericalGravityCT.getFromEntity(entity).mass;
    if (isBinary) parentMass -= mass;
    orbitC.a = pow((per * per) / 4.0 / (M_PI * M_PI) * M_G *
                           (mass + parentMass), 1.0 / 3.0) * KM_PER_M;
    orbitC.b = orbitC.a *
        sqrt(1.0 - orbitC.e * orbitC.e);
    orbitC.parentMass = parentMass;
    if (isBinary) {
        //  orbitC.r1 = 2.0 * orbitC.semiMajor * (1.0 - orbitC.eccentricity) *
        //      mass / (orbitC.totalMass);
        orbitC.r1 = orbitC.a;
    } else {
        orbitC.r1 = orbitC.a * (1.0 - orbitC.e);
    }

    { // Make the ellipse with stepwise simulation
        OrbitComponentUpdater updater;
        static const int NUM_VERTS = 2880;
        orbitC.verts.resize(NUM_VERTS + 1);
        f64 timePerDeg = orbitC.t / (f64)NUM_VERTS;
        NamePositionComponent& npCmp = spaceSystem->m_namePositionCT.get(orbitC.npID);
        f64v3 startPos = npCmp.position;
        for (int i = 0; i < NUM_VERTS; i++) {

            if (orbitC.parentOrbId) {
                OrbitComponent* pOrbC = &spaceSystem->m_orbitCT.get(orbitC.parentOrbId);
                updater.updatePosition(orbitC, i * timePerDeg, &npCmp,
                               pOrbC,
                               &spaceSystem->m_namePositionCT.get(pOrbC->npID));
            } else {
                updater.updatePosition(orbitC, i * timePerDeg, &npCmp);
            }

            orbitC.verts[i] = npCmp.position;
        }
        orbitC.verts.back() = orbitC.verts.front();
        npCmp.position = startPos;
    }
}

void SoaEngine::destroySpaceSystem(SoaState* state) {
    state->spaceSystem.reset();
}
