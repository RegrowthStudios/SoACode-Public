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
    pos = normal * m_genData->radius;

    f64 h = getBaseHeightValue(pos);
    height.height = (f32)(h * VOXELS_PER_M);
    h *= KM_PER_M;
    height.temperature = (ui8)getTemperatureValue(pos, normal, h);
    height.rainfall = (ui8)getHumidityValue(pos, normal, h);
    height.surfaceBlock = m_genData->surfaceBlock; // TODO(Ben): Naw dis is bad mkay

    // Base Biome
    height.biome = m_genData->baseBiomeLookup[height.rainfall][height.temperature];
    // Sub biomes
    while (height.biome->biomeMap.size()) {
        if (height.biome->biomeMap.size() > BIOME_MAP_WIDTH) { // 2D
            throw new nString("Not implemented");
        } else { // 1D
            throw new nString("Not implemented");
        }
    }
    // TODO(Ben) Custom generation
}

void SphericalHeightmapGenerator::generateHeightData(OUT PlanetHeightData& height, const f64v3& normal) const {
    f64v3 pos = normal * m_genData->radius;
    f64 h = getBaseHeightValue(pos);
    height.height = (f32)(h * VOXELS_PER_M);
    h *= KM_PER_M;
    height.temperature = (ui8)getTemperatureValue(pos, normal, h);
    height.rainfall = (ui8)getHumidityValue(pos, normal, h);
    height.surfaceBlock = m_genData->surfaceBlock;

    // Base Biome
    const Biome* biome;
    biome = m_genData->baseBiomeLookup[height.rainfall][height.temperature];
    // Sub biomes
    while (biome->biomeMap.size()) {
        if (biome->biomeMap.size() > BIOME_MAP_WIDTH) { // 2D
            f64 xVal = biome->xNoise.base + getNoiseValue(pos, biome->xNoise.funcs, nullptr, TerrainOp::ADD);
            int xPos = (int)glm::clamp(xVal, 0.0, 255.0);
            f64 yVal = (height.height - biome->heightScale.x) / biome->heightScale.y;
            int yPos = 255 - (int)glm::clamp(yVal, 0.0, 255.0);
            const Biome* nextBiome = biome->biomeMap[yPos * BIOME_MAP_WIDTH + xPos];
            // If the biome is nullptr, that means we use the current biome.
            if (!nextBiome) break;
            biome = nextBiome;
        } else { // 1D
            throw new nString("Not implemented");
        }
    }
    height.biome = biome;
    // TODO(Ben) Custom generation
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
    f64* nextMod;

    TerrainOp nextOp;

    // NOTE: Make sure this implementation matches NoiseShaderGenerator::addNoiseFunctions()
    for (size_t f = 0; f < funcs.size(); f++) {
        auto& fn = funcs[f];

        f64 h = 0.0f;

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
