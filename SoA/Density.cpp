#include "stdafx.h"
#include "Density.h"
#include "VoxelMatrix.h"

const VoxelMatrix* gMatrix;

// ----------------------------------------------------------------------------

float Sphere(const f32v3& worldPosition, const f32v3& origin, float radius) {
    return glm::length(worldPosition - origin) - radius;
}

// ----------------------------------------------------------------------------

float Cuboid(const f32v3& worldPosition, const f32v3& origin, const f32v3& halfDimensions) {
    const f32v3& local_pos = worldPosition - origin;
    const f32v3& pos = local_pos;

    const f32v3& d = glm::abs(pos) - halfDimensions;
    const float m = glm::max(d.x, glm::max(d.y, d.z));
    return glm::min(m, glm::length(glm::max(d, f32v3(0.f))));
}

// ----------------------------------------------------------------------------

float FractalNoise(
    const int octaves,
    const float frequency,
    const float lacunarity,
    const float persistence,
    const f32v2& position) {
    const float SCALE = 1.f / 128.f;
    f32v2 p = position * SCALE;
    float noise = 0.f;

    float amplitude = 1.f;
    p *= frequency;

    for (int i = 0; i < octaves; i++) {
    //    noise += simplex(p) * amplitude;
        p *= lacunarity;
        amplitude *= persistence;
    }

    // move into [0, 1] range
    return 0.5f + (0.5f * noise);
}

// ----------------------------------------------------------------------------

float Density_Func(const f32v3& worldPosition) {
    i32v3 pos(glm::round(worldPosition));
    float rv = 0.0f;
    if (gMatrix->getColorAndCheckBounds(pos + i32v3(gMatrix->size.x / 2, gMatrix->size.y / 2, gMatrix->size.z / 2)).a != 0) {
        rv += 100.0f;
    } else {
        rv -= 100.0f;
    }
   // return 20.0f;
    
  //  const float MAX_HEIGHT = 20.f;
  //  const float noise = FractalNoise(4, 0.5343f, 2.2324f, 0.68324f, vec2(worldPosition.x, worldPosition.z));
  //  const float terrain = worldPosition.y - (MAX_HEIGHT * noise);

  //  const float cube = Cuboid(worldPosition, vec3(-4., 10.f, -4.f), vec3(12.f));
    const float sphere = Sphere(worldPosition, f32v3(15.f, 2.5f, 1.f), 16.f);

    return sphere + rv;
}