#include "stdafx.h"
#include "SoaEngine.h"

#include "BlockData.h"
#include "BlockPack.h"
#include "ChunkMeshManager.h"
#include "Constants.h"
#include "DebugRenderer.h"
#include "Errors.h"
#include "MeshManager.h"
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

    std::map<nString, Binary*> binaries; ///< Contains all binary systems
    std::map<nString, SystemBody*> systemBodies; ///< Contains all system bodies
    std::map<nString, vecs::EntityID> bodyLookupMap;
};

void SoaEngine::initState(SoaState* state) {
    state->debugRenderer = std::make_unique<DebugRenderer>();
    state->meshManager = std::make_unique<MeshManager>();
    state->chunkMeshManager = std::make_unique<ChunkMeshManager>();
    state->systemIoManager = std::make_unique<vio::IOManager>();
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
    gen.init("NormalMapGen", createShaderSource(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/Generation/NormalMap.vert"),
                  createShaderSource(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/Generation/NormalMap.frag"));

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
    for (auto& it : pr.binaries) {
        f64 mass = 0.0;
        Binary* bin = it.second;

        // Loop through all children
        for (int i = 0; i < bin->bodies.size(); i++) {
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
    if (!pr.ioManager->readFileToString("SystemProperties.yml", data)) {
        pError("Couldn't find " + pr.dirPath +  "/SystemProperties.yml");
    }

    keg::YAMLReader reader;
    reader.init(data.c_str());
    keg::Node node = reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        fprintf(stderr, "Failed to load %s\n", (pr.dirPath + "/SystemProperties.yml").c_str());
        reader.dispose();
        return false;
    }

    bool goodParse = true;
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& name, keg::Node value) {
        // Parse based on the name
        if (name == "description") {
            pr.spaceSystem->systemDescription = name;
        } else if (name == "Binary") {
            // Binary systems
            Binary* newBinary = new Binary;
            keg::Error err = keg::parse((ui8*)newBinary, value, reader, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Binary));
            if (err != keg::Error::NONE) {
                fprintf(stderr, "Failed to parse node %s in %s\n", name.c_str(), pr.dirPath.c_str());
                goodParse = false;
            }

            pr.binaries[newBinary->name] = newBinary;
        } else {
            // We assume this is just a generic SystemBody
            SystemBodyKegProperties properties;
            properties.pathColor = ui8v4(rand() % 255, rand() % 255, rand() % 255, 255);
            keg::Error err = keg::parse((ui8*)&properties, value, reader, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(SystemBodyKegProperties));
            if (err != keg::Error::NONE) {
                fprintf(stderr, "Failed to parse node %s in %s\n", name.c_str(), pr.dirPath.c_str());
                goodParse = false;
            }

            if (properties.path.empty()) {
                fprintf(stderr, "Missing path: for node %s in %s\n", name.c_str(), pr.dirPath.c_str());
                goodParse = false;
            }
            // Allocate the body
            SystemBody* body = new SystemBody;
            body->name = name;
            body->parentName = properties.parent;
            loadBodyProperties(pr, properties.path, &properties, body);
            pr.systemBodies[name] = body;
        }
    });
    reader.forAllInMap(node, f);
    delete f;
    reader.dispose();

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

    keg::YAMLReader reader;
    reader.init(data.c_str());
    keg::Node node = reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        std::cout << "Failed to load " + filePath;
        reader.dispose();
        return false;
    }

    bool goodParse = true;
    bool foundOne = false;
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& type, keg::Node value) {
        if (foundOne) return;

        // Parse based on type
        if (type == "planet") {
            PlanetKegProperties properties;
            error = keg::parse((ui8*)&properties, value, reader, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(PlanetKegProperties));
            KEG_CHECK;

            // Use planet loader to load terrain and biomes
            if (properties.generation.length()) {
                properties.planetGenData = pr.planetLoader->loadPlanet(properties.generation, pr.glrpc);
            } else {
                properties.planetGenData = pr.planetLoader->getDefaultGenData(pr.glrpc);
            }

            // Set the radius for use later
            if (properties.planetGenData) {
                properties.planetGenData->radius = properties.diameter / 2.0;
            }

            SpaceSystemAssemblages::createPlanet(pr.spaceSystem, sysProps, &properties, body);
        } else if (type == "star") {
            StarKegProperties properties;
            error = keg::parse((ui8*)&properties, value, reader, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(StarKegProperties));
            KEG_CHECK;
            SpaceSystemAssemblages::createStar(pr.spaceSystem, sysProps, &properties, body);
        } else if (type == "gasGiant") {
            GasGiantKegProperties properties;
            error = keg::parse((ui8*)&properties, value, reader, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(GasGiantKegProperties));
            KEG_CHECK;
            createGasGiant(pr, sysProps, &properties, body);
        }

        pr.bodyLookupMap[body->name] = body->entity;

        //Only parse the first
        foundOne = true;
    });
    reader.forAllInMap(node, f);
    delete f;
    reader.dispose();

    return goodParse;
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
            vg::BitmapResource b = vg::ImageIO().load(properties->colorMap);
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

void SoaEngine::calculateOrbit(SpaceSystem* spaceSystem, vecs::EntityID entity, f64 parentMass, bool isBinary) {
    OrbitComponent& orbitC = spaceSystem->m_orbitCT.getFromEntity(entity);

    f64 per = orbitC.orbitalPeriod;
    f64 mass = spaceSystem->m_sphericalGravityCT.getFromEntity(entity).mass;
    if (isBinary) parentMass -= mass;
    orbitC.semiMajor = pow((per * per) / 4.0 / (M_PI * M_PI) * M_G *
                           (mass + parentMass), 1.0 / 3.0);
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
    state->spaceSystem.reset();
}
