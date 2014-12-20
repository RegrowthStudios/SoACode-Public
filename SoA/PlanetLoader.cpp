#include "stdafx.h"
#include "PlanetLoader.h"

#include <IOManager.h>

enum class TerrainFunction {
    RIDGED_NOISE
};
KEG_ENUM_INIT_BEGIN(TerrainFunction, TerrainFunction, e);
e->addValue("ridgedNoise", TerrainFunction::RIDGED_NOISE);
KEG_ENUM_INIT_END

class TerrainFuncKegProperties {
public:
    TerrainFunction func;
    int octaves = 1;
    float persistence = 1.0f;
    float frequency = 1.0f;
    float low = 0.0f;
    float high = 0.0f;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(TerrainFuncKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, I32, octaves);
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, F32, persistence);
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, F32, frequency);
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, F32, low);
KEG_TYPE_INIT_ADD_MEMBER(TerrainFuncKegProperties, F32, high);
KEG_TYPE_INIT_END

class TerrainFuncs {
public:
    std::vector<TerrainFuncKegProperties> funcs;
    float baseHeight = 0.0f;
};

PlanetLoader::PlanetLoader(IOManager* ioManager) :
    m_iom(ioManager) {
    // Empty
}


PlanetLoader::~PlanetLoader() {
}

PlanetGenData* PlanetLoader::loadPlanet(const nString& filePath) {
    nString data;
    m_iom->readFileToString(filePath.c_str(), data);

    YAML::Node node = YAML::Load(data.c_str());
    if (node.IsNull() || !node.IsMap()) {
        std::cout << "Failed to load " + filePath;
        return false;
    }

    TerrainFuncs baseTerrainFuncs;
    TerrainFuncs tempTerrainFuncs;
    TerrainFuncs humTerrainFuncs;

    for (auto& kvp : node) {
        nString type = kvp.first.as<nString>();
        // Parse based on type
        if (type == "baseHeight") {
            parseTerrainFuncs(&baseTerrainFuncs, kvp.second);
        } else if (type == "temperature") {
            parseTerrainFuncs(&tempTerrainFuncs, kvp.second);
        } else if (type == "humidity") {
            parseTerrainFuncs(&humTerrainFuncs, kvp.second);
        }
    }
}

void PlanetLoader::parseTerrainFuncs(TerrainFuncs* terrainFuncs, YAML::Node& node) {
    if (node.IsNull() || !node.IsMap()) {
        std::cout << "Failed to parse node";
        return;
    }

    #define KEG_CHECK if (error != Keg::Error::NONE) { \
        fprintf(stderr, "Keg error %d\n", (int)error); \
        return; \
    }

    Keg::Error error;

    for (auto& kvp : node) {
        nString type = kvp.first.as<nString>();
        if (type == "add") {
            terrainFuncs->baseHeight += kvp.second.as<float>();
            continue;
        }

        terrainFuncs->funcs.push_back(TerrainFuncKegProperties());
        if (type == "ridgedNoise") {
            terrainFuncs->funcs.back().func = TerrainFunction::RIDGED_NOISE;
        }

        error = Keg::parse((ui8*)&terrainFuncs->funcs.back(), kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(TerrainFuncKegProperties));
        KEG_CHECK;
    }
}
