#include "stdafx.h"
#include "SystemBodyLoader.h"

#include "SpaceSystemAssemblages.h"
#include "SoAState.h"

#include <Vorb/io/IOManager.h>

void SystemBodyLoader::init(vio::IOManager* iom) {
    m_iom = iom;
    m_planetLoader.init(iom);
}

// TODO: Check why glrpc is currently unused.
bool SystemBodyLoader::loadBody(const SoaState* soaState, const nString& filePath,
                                const SystemOrbitProperties* sysProps, SystemBody* body, keg::Node &value,
                                vcore::RPCManager* glrpc VORB_UNUSED /* = nullptr */) {

#define KEG_CHECK \
    if (error != keg::Error::NONE) { \
        fprintf(stderr, "keg error %d for %s\n", (int)error, filePath.c_str()); \
        goodParse = false; \
        return;  \
                    }

    keg::Error error;
    nString data;
    nString propertiesFile=filePath+"properties.yml";

    m_iom->readFileToString(propertiesFile.c_str(), data);

    keg::ReadContext context;
    context.env=keg::getGlobalEnvironment();

    bool goodParse = true;
    bool foundOne = false;
    auto f = makeFunctor([&](Sender, const nString& type, keg::Node value) {
        if (foundOne) return;

        // Parse based on type
        if (type == "planet") {
            PlanetProperties properties;
            error = keg::parse((ui8*)&properties, value, context, &KEG_GLOBAL_TYPE(PlanetProperties));
            KEG_CHECK;

            // Use planet loader to load terrain and biomes
            if (properties.generation.length()) {
                properties.planetGenData = m_planetLoader.loadPlanetGenData(properties.generation);
            } else {
                properties.planetGenData = nullptr;
                // properties.planetGenData = pr.planetLoader->getRandomGenData(properties.density, pr.glrpc);
                properties.atmosphere = m_planetLoader.getRandomAtmosphere();
            }

            // Set the radius for use later
            if (properties.planetGenData) {
                properties.planetGenData->radius = properties.diameter / 2.0;
            }

            SpaceSystemAssemblages::createPlanet(soaState->spaceSystem, sysProps, &properties, body, soaState->threadPool);
            body->type = SpaceBodyType::PLANET;
        } else if (type == "star") {
            StarProperties properties;
            error = keg::parse((ui8*)&properties, value, context, &KEG_GLOBAL_TYPE(StarProperties));
            KEG_CHECK;
            SpaceSystemAssemblages::createStar(soaState->spaceSystem, sysProps, &properties, body);
            body->type = SpaceBodyType::STAR;
        } else if (type == "gasGiant") {
            GasGiantProperties properties;
            error = keg::parse((ui8*)&properties, value, context, &KEG_GLOBAL_TYPE(GasGiantProperties));
            KEG_CHECK;
            // Get full path for color map
            if (properties.colorMap.size()) {
                vio::Path colorPath;
                if (!m_iom->resolvePath(properties.colorMap, colorPath)) {
                    fprintf(stderr, "Failed to resolve %s\n", properties.colorMap.c_str());
                }
                properties.colorMap = colorPath.getString();
            }
            // Get full path for rings
            if (properties.rings.size()) {
                for (size_t i = 0; i < properties.rings.size(); i++) {
                    auto& r = properties.rings[i];
                    // Resolve the path
                    vio::Path ringPath;
                    if (!m_iom->resolvePath(r.colorLookup, ringPath)) {
                        fprintf(stderr, "Failed to resolve %s\n", r.colorLookup.c_str());
                    }
                    r.colorLookup = ringPath.getString();
                }
            }
            // Create the component
            SpaceSystemAssemblages::createGasGiant(soaState->spaceSystem, sysProps, &properties, body);
            body->type = SpaceBodyType::GAS_GIANT;
        }

        //Only parse the first
        foundOne = true;
    });

    if(data.empty())
    {
        f->invoke(nullptr, spaceObjectTypeName(sysProps->type), value);
    }
    else
    {
        context.reader.init(data.c_str());
        keg::Node node=context.reader.getFirst();

        if(keg::getType(node)!=keg::NodeType::MAP)
        {
            //lets go ahead and give it an entity id even though we dont have properties for it
            body->entity=soaState->spaceSystem->addEntity();

            std::cout<<"Failed to load "+filePath;
            context.reader.dispose();
            return false;
        }

        context.reader.forAllInMap(node, f);
    }

    delete f;
    context.reader.dispose();

    return goodParse;
}
