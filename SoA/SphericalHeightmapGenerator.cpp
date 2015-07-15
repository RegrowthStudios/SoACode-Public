#include "stdafx.h"
#include "SphericalHeightmapGenerator.h"

#include "PlanetHeightData.h"
#include "VoxelSpaceConversions.h"
#include "Noise.h"

#define WEIGHT_THRESHOLD 0.001

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

    // For Voxel Position, automatically get tree or flora
    height.flora = getTreeID(height.biome, facePosition, pos);
    // If no tree, try flora
    if (height.flora == FLORA_ID_NONE) {
        height.flora = getFloraID(height.biome, facePosition, pos);
    }
}

void SphericalHeightmapGenerator::generateHeightData(OUT PlanetHeightData& height, const f64v3& normal) const {
    generateHeightData(height, normal * m_genData->radius, normal);
}

//No idea how this works. Something to do with prime numbers, but returns # between -1 and 1
inline f64 pseudoRand(int x, int z) {
    int n = (x & 0xFFFF) + ((z & 0x7FFF) << 16);
    n = (n << 13) ^ n;
    int nn = (n*(n*n * 60493 + z * 19990303) + x * 1376312589) & 0x7fffffff;
    return ((f64)nn / 1073741824.0);
}

FloraID SphericalHeightmapGenerator::getTreeID(const Biome* biome, const VoxelPosition2D& facePosition, const f64v3& worldPos) const {
    // TODO(Ben): Experiment with optimizations with large amounts of flora.
    // TODO(Ben): Sort trees with priority
    for (size_t i = 0; i < biome->trees.size(); i++) {
        const BiomeTree& t = biome->trees[i];
        f64 chance = t.chance.base;
        getNoiseValue(worldPos, t.chance.funcs, nullptr, TerrainOp::ADD, chance);
        f64 roll = pseudoRand((int)facePosition.pos.x, (int)facePosition.pos.y);
        if (roll < chance) {
            return t.id;
        }
    }
    return FLORA_ID_NONE;
}

FloraID SphericalHeightmapGenerator::getFloraID(const Biome* biome, const VoxelPosition2D& facePosition, const f64v3& worldPos) const {
    // TODO(Ben): Experiment with optimizations with large amounts of flora.
    // TODO(Ben): Sort flora with priority
    for (size_t i = 0; i < biome->flora.size(); i++) {
        const BiomeFlora& f = biome->flora[i];
        f64 chance = f.chance.base;
        getNoiseValue(worldPos, f.chance.funcs, nullptr, TerrainOp::ADD, chance);
        f64 roll = pseudoRand((int)facePosition.pos.x, (int)facePosition.pos.y);
        if (roll < chance) {
            return f.id;
        }
    }
    return FLORA_ID_NONE;
}

void getBaseBiomes(const std::vector<BiomeInfluence> baseBiomeInfluenceMap[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH], f64 x, f64 y, OUT std::map<BiomeInfluence, f64>& rvBiomes) {
    int ix = (int)x;
    int iy = (int)y;

    //0 1
    //2 3
    /* Interpolate */
    // Get weights
    f64 fx = x - (f64)ix;
    f64 fy = y - (f64)iy;
    f64 fx1 = 1.0 - fx;
    f64 fy1 = 1.0 - fy;
    f64 w0 = fx1 * fy1;
    f64 w1 = fx * fy1;
    f64 w2 = fx1 * fy;
    f64 w3 = fx * fy;

    // Shorter handles
#define BLIST_0 baseBiomeInfluenceMap[iy][ix]
#define BLIST_1 baseBiomeInfluenceMap[iy][ix + 1]
#define BLIST_2 baseBiomeInfluenceMap[iy + 1][ix]
#define BLIST_3 baseBiomeInfluenceMap[iy + 1][ix + 1]

    // TODO(Ben): Explore padding to ditch ifs?
    /* Construct list of biomes to generate and assign weights from interpolation. */
    // Top Left
    for (auto& b : BLIST_0) {
        auto& it = rvBiomes.find(b);
        if (it == rvBiomes.end()) {
            rvBiomes[b] = w0 * b.weight;
        } else {
            it->second += w0 * b.weight;
        }
    }
    // Top Right
    if (ix < BIOME_MAP_WIDTH - 1) {
        for (auto& b : BLIST_1) {
            auto& it = rvBiomes.find(b);
            if (it == rvBiomes.end()) {
                rvBiomes[b] = w1 * b.weight;
            } else {
                it->second += w1 * b.weight;
            }
        }
    } else {
        for (auto& b : BLIST_0) {
            rvBiomes[b] += w1 * b.weight;
        }
    }
    // Bottom left
    if (iy < BIOME_MAP_WIDTH - 1) {
        for (auto& b : BLIST_2) {
            auto& it = rvBiomes.find(b);
            if (it == rvBiomes.end()) {
                rvBiomes[b] = w2 * b.weight;
            } else {
                it->second += w2 * b.weight;
            }
        }
        // Bottom right
        if (ix < BIOME_MAP_WIDTH - 1) {
            for (auto& b : BLIST_3) {
                auto& it = rvBiomes.find(b);
                if (it == rvBiomes.end()) {
                    rvBiomes[b] = w3 * b.weight;
                } else {
                    it->second += w3 * b.weight;
                }
            }
        } else {
            for (auto& b : BLIST_2) {
                rvBiomes[b] += w3 * b.weight;
            }
        }
    } else {
        for (auto& b : BLIST_0) {
            rvBiomes[b] += w2 * b.weight;
        }
        for (auto& b : BLIST_1) {
            rvBiomes[b] += w3 * b.weight;
        }
    }
}

inline void SphericalHeightmapGenerator::generateHeightData(OUT PlanetHeightData& height, const f64v3& pos, const f64v3& normal) const {
    f64 h = getBaseHeightValue(pos);
    height.height = (f32)(h * VOXELS_PER_M);
    h *= KM_PER_M;
    f64 temperature = getTemperatureValue(pos, normal, h);
    f64 humidity = getHumidityValue(pos, normal, h);
    height.temperature = (ui8)temperature;
    height.humidity = (ui8)humidity;
    height.flora = FLORA_ID_NONE;

    // Base Biome
    f64 biggestWeight = 0.0;
    const Biome* bestBiome = m_genData->baseBiomeLookup[height.humidity][height.temperature];

    std::map<BiomeInfluence, f64> baseBiomes;
    getBaseBiomes(m_genData->baseBiomeInfluenceMap, temperature, humidity, baseBiomes);

    for (auto& bb : baseBiomes) {
        const Biome* biome = bb.first.b;
        f64 baseWeight = bb.first.weight * bb.second;
        // Get base biome terrain
        f64 newHeight = biome->terrainNoise.base + height.height;
        getNoiseValue(pos, biome->terrainNoise.funcs, nullptr, TerrainOp::ADD, newHeight);
        // Mix in height with squared interpolation
        height.height = (f32)((baseWeight * newHeight) + (1.0 - baseWeight) * (f64)height.height);
        // Sub biomes
        recurseChildBiomes(biome, pos, height.height, biggestWeight, bestBiome, baseWeight);
    }
    // Mark biome that is the best
    height.biome = bestBiome;
}

void SphericalHeightmapGenerator::recurseChildBiomes(const Biome* biome, const f64v3& pos, f32& height, f64& biggestWeight, const Biome*& bestBiome, f64 baseWeight) const {
    // Get child noise value
    f64 noiseVal = biome->childNoise.base;
    getNoiseValue(pos, biome->childNoise.funcs, nullptr, TerrainOp::ADD, noiseVal);
    // Sub biomes
    for (auto& child : biome->children) {
        f64 weight = 1.0;
        // Check if biome is present and get weight
        { // Noise
            f64 dx = noiseVal - child->noiseRange.x;
            f64 dy = child->noiseRange.y - noiseVal;
            // See which side we are closer to
            if (ABS(dx) < ABS(dy)) {
                if (dx < 0) {
                    continue;
                }
                dx *= child->noiseScale.x;
                if (dx > 1.0) {
                    dx = 1.0;
                } else {
                    weight *= dx;
                }
            } else {
                if (dy < 0) {
                    continue;
                }
                dy *= child->noiseScale.x;
                if (dy > 1.0) {
                    dy = 1.0;
                } else {
                    weight *= dy;
                }
            }
        }
        { // Height
            f64 dx = height - child->heightRange.x;
            f64 dy = child->heightRange.y - height;
            // See which side we are closer to
            if (ABS(dx) < ABS(dy)) {
                if (dx < 0) {
                    continue;
                }
                dx *= child->heightScale.x;
                if (dx > 1.0) {
                    dx = 1.0;
                } else {
                    weight *= hermite(dx);
                }
            } else {
                if (dy < 0) {
                    continue;
                }
                dy *= child->heightScale.x;
                if (dy > 1.0) {
                    dy = 1.0;
                } else {
                    weight *= hermite(dy);
                }
            }
        }
        // If we reach here, the biome exists.
        f64 newHeight = child->terrainNoise.base + height;
        getNoiseValue(pos, child->terrainNoise.funcs, nullptr, TerrainOp::ADD, newHeight);
        // Biggest weight biome is the next biome
        if (weight >= biggestWeight) {
            biggestWeight = weight;
            bestBiome = child;
        }
        weight = baseWeight * weight * weight;
        // Mix in height with squared interpolation
        height = (f32)((weight * newHeight) + (1.0 - weight) * height);
        // Recurse children
        if (child->children.size() && weight > WEIGHT_THRESHOLD) {
            recurseChildBiomes(child, pos, height, biggestWeight, bestBiome, weight);
        }
    }
}

f64 SphericalHeightmapGenerator::getBaseHeightValue(const f64v3& pos) const {
    f64 genHeight = m_genData->baseTerrainFuncs.base;
    getNoiseValue(pos, m_genData->baseTerrainFuncs.funcs, nullptr, TerrainOp::ADD, genHeight);
    return genHeight;
}

f64 SphericalHeightmapGenerator::getTemperatureValue(const f64v3& pos, const f64v3& normal, f64 height) const {
    f64 genHeight = m_genData->tempTerrainFuncs.base;
    getNoiseValue(pos, m_genData->tempTerrainFuncs.funcs, nullptr, TerrainOp::ADD, genHeight);
    return calculateTemperature(m_genData->tempLatitudeFalloff, computeAngleFromNormal(normal), genHeight - glm::max(0.0, m_genData->tempHeightFalloff * height));
}

f64 SphericalHeightmapGenerator::getHumidityValue(const f64v3& pos, const f64v3& normal, f64 height) const {
    f64 genHeight = m_genData->humTerrainFuncs.base;
    getNoiseValue(pos, m_genData->humTerrainFuncs.funcs, nullptr, TerrainOp::ADD, genHeight);
    return SphericalHeightmapGenerator::calculateHumidity(m_genData->humLatitudeFalloff, computeAngleFromNormal(normal), genHeight - glm::max(0.0, m_genData->humHeightFalloff * height));
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

void SphericalHeightmapGenerator::getNoiseValue(const f64v3& pos,
                                                const Array<TerrainFuncProperties>& funcs,
                                                f64* modifier,
                                                const TerrainOp& op,
                                                f64& height) const {

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
                getNoiseValue(pos, fn.children, nextMod, nextOp, height);
            }
        } else {
            height = doOperation(fn.op, height, h);
        }
    }
}
