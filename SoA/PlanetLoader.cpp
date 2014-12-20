#include "stdafx.h"
#include "PlanetLoader.h"
#include "Keg.h"

enum class TerrainFunction {
    RIDGED_NOISE
};
KEG_ENUM_INIT_BEGIN(TerrainFunction, TerrainFunction, e);
e->addValue("ridgedNoise", TerrainFunction::RIDGED_NOISE);
KEG_ENUM_INIT_END

class GenerationKegProperties {
public:
    TerrainFunction func;
    int octaves = 1;
    float persistence = 1.0f;
    float frequency = 1.0f;
    float low = 0.0f;
    float high = 0.0f;
};
KEG_TYPE_INIT_BEGIN_DEF_VAR(GenerationKegProperties)
KEG_TYPE_INIT_ADD_MEMBER(GenerationKegProperties, I32, octaves);
KEG_TYPE_INIT_ADD_MEMBER(GenerationKegProperties, F32, persistence);
KEG_TYPE_INIT_ADD_MEMBER(GenerationKegProperties, F32, frequency);
KEG_TYPE_INIT_ADD_MEMBER(GenerationKegProperties, F32, low);
KEG_TYPE_INIT_ADD_MEMBER(GenerationKegProperties, F32, high);
KEG_TYPE_INIT_END

PlanetLoader::PlanetLoader() {
}


PlanetLoader::~PlanetLoader() {
}

PlanetGenerationData* PlanetLoader::loadPlanet(const nString& filePath) {

}
