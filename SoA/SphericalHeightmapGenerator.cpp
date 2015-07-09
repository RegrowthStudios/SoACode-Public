#include "stdafx.h"
#include "SphericalHeightmapGenerator.h"

#include "PlanetHeightData.h"
#include "VoxelSpaceConversions.h"
#include "Noise.h"

void SphericalHeightmapGenerator::init(const PlanetGenData* planetGenData) {
    m_genData = planetGenData;
}

void SphericalHeightmapGenerator::generateHeightData(OUT PlanetHeightData& height, const VoxelPosition2D& facePosition) const {
    // Need to convert to world-space
    f32v2 coordMults = f32v2(VoxelSpaceConversions::FACE_TO_WORLD_MULTS[(int)facePosition.face]);
    i32v3 coordMapping = VoxelSpaceConversions::VOXEL_TO_WORLD[(int)facePosition.face];

    f64v3 pos;
    pos[coordMapping.x] = facePosition.pos.x * KM_PER_VOXEL * coordMults.x;
    pos[coordMapping.y] = m_genData->radius * (f64)VoxelSpaceConversions::FACE_Y_MULTS[(int)facePosition.face];
    pos[coordMapping.z] = facePosition.pos.y * KM_PER_VOXEL * coordMults.y;

    f64v3 normal = glm::normalize(pos);

    generateHeightData(height, normal * m_genData->radius, normal);
}

void SphericalHeightmapGenerator::generateHeightData(OUT PlanetHeightData& height, const f64v3& normal) const {
    generateHeightData(height, normal * m_genData->radius, normal);
}

struct BiomeContribution {
    const std::vector<BiomeInfluence>* biomes;
    f64 weight;
};

inline bool biomeContSort(const BiomeContribution& a, const BiomeContribution& b) {
    return a.weight > b.weight;
}

void getBiomes(const BiomeInfluenceMap& biomeMap, f64 x, f64 y, OUT BiomeContribution rvBiomes[4], OUT ui32& numBiomes) {
    int ix = (int)x;
    int iy = (int)y;

    //0 1
    //2 3

    // TODO(Ben): Padding to ditch ifs?
    // Top Left
    rvBiomes[0].biomes = &biomeMap[iy * BIOME_MAP_WIDTH + ix];
    // Top Right
    if (ix < BIOME_MAP_WIDTH - 1) {
        rvBiomes[1].biomes = &biomeMap[iy * BIOME_MAP_WIDTH + ix + 1];
    } else {
        rvBiomes[1].biomes = rvBiomes[0].biomes;
    }
    // Bottom left
    if (iy < BIOME_MAP_WIDTH - 1) {
        rvBiomes[2].biomes = &biomeMap[(iy + 1) * BIOME_MAP_WIDTH + ix];
        // Bottom right
        if (ix < BIOME_MAP_WIDTH - 1) {
            rvBiomes[3].biomes = &biomeMap[(iy + 1) * BIOME_MAP_WIDTH + ix + 1];
        } else {
            rvBiomes[3].biomes = rvBiomes[2].biomes;
        }
    } else {
        rvBiomes[2].biomes = rvBiomes[0].biomes;
        rvBiomes[3].biomes = rvBiomes[1].biomes;
    }

    /* Interpolate */
    // Get weights
    f64 fx = x - (f64)ix;
    f64 fy = y - (f64)iy;
    f64 fx1 = 1.0 - fx;
    f64 fy1 = 1.0 - fy;
    if (rvBiomes[0].biomes->size()) {
        rvBiomes[0].weight = fx1 * fy1;
    } else {
        rvBiomes[0].biomes = nullptr;
    }
    if (rvBiomes[1].biomes->size()) {
        rvBiomes[1].weight = fx * fy1;
    } else {
        rvBiomes[1].biomes = nullptr;
    }
    if (rvBiomes[2].biomes->size()) {
        rvBiomes[2].weight = fx1 * fy;
    } else {
        rvBiomes[2].biomes = nullptr;
    }
    if (rvBiomes[3].biomes->size()) {
        rvBiomes[3].weight = fx * fy;
    } else {
        rvBiomes[3].biomes = nullptr;
    }

    numBiomes = 4;
    // Remove duplicates
    for (int i = 0; i < 3; i++) {
        if (rvBiomes[i].biomes) {
            for (int j = i + 1; j < 4; j++) {
                if (rvBiomes[i].biomes == rvBiomes[j].biomes) {
                    rvBiomes[i].weight += rvBiomes[j].weight;
                    rvBiomes[j].biomes = nullptr;
                }
            }
        } else {
            rvBiomes[i].weight = -1.0;
            --numBiomes;
        }
    }
    // Check last biome for null so we don't have to iterate to 4
    if (!rvBiomes[3].biomes) {
        rvBiomes[3].weight = -1.0;
        --numBiomes;
    }

    // Sort based on weight
    std::sort(rvBiomes, rvBiomes + 4, biomeContSort);
}

inline void SphericalHeightmapGenerator::generateHeightData(OUT PlanetHeightData& height, const f64v3& pos, const f64v3& normal) const {
    f64 h = getBaseHeightValue(pos);
    height.height = (f32)(h * VOXELS_PER_M);
    h *= KM_PER_M;
    f64 temperature = getTemperatureValue(pos, normal, h);
    f64 humidity = getHumidityValue(pos, normal, h);
    height.temperature = (ui8)temperature;
    height.humidity = (ui8)humidity;
    height.surfaceBlock = m_genData->surfaceBlock;

    // Base Biome
    const Biome* biome;
    biome = m_genData->baseBiomeLookup[height.humidity][height.temperature];

    BiomeContribution cornerBiomes[4];
    ui32 numBiomes;
    // Sub biomes
    while (biome->biomeMap.size()) {
        if (biome->biomeMap.size() > BIOME_MAP_WIDTH) { // 2D
            f64 xVal = biome->xNoise.base + getNoiseValue(pos, biome->xNoise.funcs, nullptr, TerrainOp::ADD);
            xVal = glm::clamp(xVal, 0.0, 255.0);
            f64 yVal = ((height.height - biome->heightScale.x) / biome->heightScale.y) * 255.0;
            yVal = glm::clamp(yVal, 0.0, 255.0);

            getBiomes(biome->biomeMap, xVal, yVal, cornerBiomes, numBiomes);
            if (numBiomes == 0) break;
           
        } else { // 1D
            throw new nString("Not implemented");
        }

        for (ui32 i = 0; i < numBiomes; i++) {
            for (size_t j = 0; j < cornerBiomes[i].biomes->size(); j++) {
                const BiomeInfluence& inf = cornerBiomes[i].biomes->operator[](j);
                f64 newHeight = inf.b->terrainNoise.base + getNoiseValue(pos, inf.b->terrainNoise.funcs, nullptr, TerrainOp::ADD);
                const f64& weight = cornerBiomes[i].weight;
                // Add height with squared interpolation
                height.height += (f32)(weight * newHeight);
            }
        }
        // Next biome is the one with the most weight
        biome = cornerBiomes[0].biomes->front().b;
    }
    height.biome = biome;
}

f64 SphericalHeightmapGenerator::getBaseHeightValue(const f64v3& pos) const {
    return m_genData->baseTerrainFuncs.base + getNoiseValue(pos, m_genData->baseTerrainFuncs.funcs, nullptr, TerrainOp::ADD);
}

f64 SphericalHeightmapGenerator::getTemperatureValue(const f64v3& pos, const f64v3& normal, f64 height) const {
    f32 temp = m_genData->tempTerrainFuncs.base + getNoiseValue(pos, m_genData->tempTerrainFuncs.funcs, nullptr, TerrainOp::ADD);
    return calculateTemperature(m_genData->tempLatitudeFalloff, computeAngleFromNormal(normal), temp - glm::max(0.0, m_genData->tempHeightFalloff * height));
}

f64 SphericalHeightmapGenerator::getHumidityValue(const f64v3& pos, const f64v3& normal, f64 height) const {
    f32 hum = m_genData->humTerrainFuncs.base + getNoiseValue(pos, m_genData->humTerrainFuncs.funcs, nullptr, TerrainOp::ADD);
    return SphericalHeightmapGenerator::calculateHumidity(m_genData->humLatitudeFalloff, computeAngleFromNormal(normal), hum - glm::max(0.0, m_genData->humHeightFalloff * height));
}

// Thanks to tetryds for these
f64 SphericalHeightmapGenerator::calculateTemperature(f64 range, f64 angle, f64 baseTemp) {
    f64 tempFalloff = 1.0 - pow(cos(angle), 2.0 * angle);
    f64 temp = baseTemp - tempFalloff * range;
    return glm::clamp(temp, 0.0, 255.0);
}

// Thanks to tetryds for these
f64 SphericalHeightmapGenerator::calculateHumidity(f64 range, f64 angle, f64 baseHum) {
    f64 cos3x = cos(3.0 * angle);
    f64 humFalloff = 1.0 - (-0.25 * angle + 1.0) * (cos3x * cos3x);
    f64 hum = baseHum - humFalloff * range;
    return glm::clamp(hum, 0.0, 255.0);
}

f64 SphericalHeightmapGenerator::computeAngleFromNormal(const f64v3& normal) {
    // Compute angle
    if (abs(normal.y) <= 0.000000001) {
        // Need to do this to fix an equator bug
        return 0.0;
    } else {
        f64v3 equator = glm::normalize(f64v3(normal.x, 0.000000001, normal.z));
        return acos(glm::dot(equator, normal));
    }
}

f64 doOperation(const TerrainOp& op, f64 a, f64 b) {
    switch (op) {
        case TerrainOp::ADD: return a + b;
        case TerrainOp::SUB: return a - b;
        case TerrainOp::MUL: return a * b;
        case TerrainOp::DIV: return a / b;
    }
    return 0.0f;
}

f64 SphericalHeightmapGenerator::getNoiseValue(const f64v3& pos,
                                                const Array<TerrainFuncKegProperties>& funcs,
                                                f64* modifier,
                                                const TerrainOp& op) const {

    f64 rv = 0.0f;
   
    // NOTE: Make sure this implementation matches NoiseShaderGenerator::addNoiseFunctions()
    for (size_t f = 0; f < funcs.size(); ++f) {
        auto& fn = funcs[f];

        f64 h = 0.0f;
        f64* nextMod;
        TerrainOp nextOp;
        // Check if its not a noise function
        if (fn.func == TerrainStage::CONSTANT) {
            nextMod = &h;
            h = fn.low;
            // Apply parent before clamping
            if (modifier) {
                h = doOperation(op, h, *modifier);
            }
            // Optional clamp if both fields are not 0.0f
            if (fn.clamp[0] != 0.0f || fn.clamp[1] != 0.0f) {
                h = glm::clamp(*modifier, (f64)fn.clamp[0], (f64)fn.clamp[1]);
            }
            nextOp = fn.op;
        } else if (fn.func == TerrainStage::PASS_THROUGH) {
            nextMod = modifier;
            // Apply parent before clamping
            if (modifier) {
                h = doOperation(op, *modifier, fn.low);
                // Optional clamp if both fields are not 0.0f
                if (fn.clamp[0] != 0.0f || fn.clamp[1] != 0.0f) {
                    h = glm::clamp(h, (f64)fn.clamp[0], (f64)fn.clamp[1]);
                }
            }
            nextOp = op;
        } else if (fn.func == TerrainStage::SQUARED) {
            nextMod = modifier;
            // Apply parent before clamping
            if (modifier) {
                *modifier = (*modifier) * (*modifier);
                // Optional clamp if both fields are not 0.0f
                if (fn.clamp[0] != 0.0f || fn.clamp[1] != 0.0f) {
                    h = glm::clamp(h, (f64)fn.clamp[0], (f64)fn.clamp[1]);
                }
            }
            nextOp = op;
        } else if (fn.func == TerrainStage::CUBED) {
            nextMod = modifier;
            // Apply parent before clamping
            if (modifier) {
                *modifier = (*modifier) * (*modifier) * (*modifier);
                // Optional clamp if both fields are not 0.0f
                if (fn.clamp[0] != 0.0f || fn.clamp[1] != 0.0f) {
                    h = glm::clamp(h, (f64)fn.clamp[0], (f64)fn.clamp[1]);
                }
            }
            nextOp = op;
        } else { // It's a noise function
            nextMod = &h;
            f64v2 ff;
            f64 tmp;
            f64 total = 0.0;
            f64 maxAmplitude = 0.0;
            f64 amplitude = 1.0;
            f64 frequency = fn.frequency;
            for (int i = 0; i < fn.octaves; i++) {
                // TODO(Ben): Could cut branching
                switch (fn.func) {
                    case TerrainStage::CUBED_NOISE:
                    case TerrainStage::SQUARED_NOISE:
                    case TerrainStage::NOISE:
                        total += Noise::raw(pos.x * frequency, pos.y * frequency, pos.z * frequency) * amplitude;
                        break;
                    case TerrainStage::RIDGED_NOISE:
                        total += ((1.0 - glm::abs(Noise::raw(pos.x * frequency, pos.y * frequency, pos.z * frequency))) * 2.0 - 1.0) * amplitude;
                        break;
                    case TerrainStage::ABS_NOISE:
                        total += glm::abs(Noise::raw(pos.x * frequency, pos.y * frequency, pos.z * frequency)) * amplitude;
                        break;
                    case TerrainStage::CELLULAR_NOISE:
                        ff = Noise::cellular(pos * (f64)frequency);
                        total += (ff.y - ff.x) * amplitude;
                        break;
                    case TerrainStage::CELLULAR_SQUARED_NOISE:
                        ff = Noise::cellular(pos * (f64)frequency);
                        tmp = ff.y - ff.x;
                        total += tmp * tmp * amplitude;
                        break;
                    case TerrainStage::CELLULAR_CUBED_NOISE:
                        ff = Noise::cellular(pos * (f64)frequency);
                        tmp = ff.y - ff.x;
                        total += tmp * tmp * tmp * amplitude;
                        break;
                }
                frequency *= 2.0;
                maxAmplitude += amplitude;
                amplitude *= fn.persistence;
            }
            total = (total / maxAmplitude);
            // Handle any post processes per noise
            switch (fn.func) {
                case TerrainStage::CUBED_NOISE:
                    total = total * total * total;
                    break;
                case TerrainStage::SQUARED_NOISE:
                    total = total * total;
                    break;
                default:
                    break;
            }
            // Conditional scaling. 
            if (fn.low != -1.0f || fn.high != 1.0f) {
                h = total * (fn.high - fn.low) * 0.5 + (fn.high + fn.low) * 0.5;
            } else {
                h = total;
            }
            // Optional clamp if both fields are not 0.0f
            if (fn.clamp[0] != 0.0f || fn.clamp[1] != 0.0f) {
                h = glm::clamp(h, (f64)fn.clamp[0], (f64)fn.clamp[1]);
            }
            // Apply modifier from parent if needed
            if (modifier) {
                h = doOperation(op, h, *modifier);
            }
            nextOp = fn.op;
        }

        if (fn.children.size()) {
            // Early exit for speed
            if (!(nextOp == TerrainOp::MUL && *nextMod == 0.0)) {
                rv += getNoiseValue(pos, fn.children, nextMod, nextOp);
            }
        } else {
            rv = doOperation(fn.op, rv, h);
        }
    }
    return rv;
}
