///
/// soaUtils.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Feb 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Soa specific generic utilities
///

#pragma once

#ifndef soaUtils_h__
#define soaUtils_h__

#include "Constants.h"

#include <Vorb/graphics/ImageIO.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/types.h>
#include <chrono>
#include <cstdio>
#include <ctime> 
#include <iomanip>
#include <sstream>
#include <string> 

/************************************************************************/
/* Debugging Utilities                                                  */
/************************************************************************/
#define PRINT_VEC_TYPE(TYPE, SYM) \
inline void printVec(const char* desc, const TYPE##v2& vec) { \
    printf("%s <%"#SYM", %"#SYM">\n", desc, vec.x, vec.y); \
} \
inline void printVec(const char* desc, const TYPE##v3& vec) { \
    printf("%s <%"#SYM", %"#SYM", %"#SYM">\n", desc, vec.x, vec.y, vec.z); \
} \
inline void printVec(const char* desc, const TYPE##v4& vec) { \
    printf("%s <%"#SYM", %"#SYM", %"#SYM", %"#SYM">\n", desc, vec.x, vec.y, vec.z, vec.w); \
}
PRINT_VEC_TYPE(f32, f)
PRINT_VEC_TYPE(f64, lf)
PRINT_VEC_TYPE(i16, hd)
PRINT_VEC_TYPE(i32, d)
PRINT_VEC_TYPE(i64, ld)
PRINT_VEC_TYPE(ui16, hu)
PRINT_VEC_TYPE(ui32, u)
PRINT_VEC_TYPE(ui64, lu)
#undef PRINT_VEC_TYPE

/************************************************************************/
/* Miscellaneous Utilities                                              */
/************************************************************************/
/// Gets the closest point on the AABB to the position
/// @param pos: Position to query nearest point in relation to
/// @param aabbPos: Position of the -x,-y,-z corner of the aabb
/// @param aabbDims: Dimensions of the aabb
/// @return the position of the closest point on the aabb
inline f32v3 getClosestPointOnAABB(const f32v3& pos, const f32v3& aabbPos,
                                   const f32v3& aabbDims) {
    return f32v3((pos.x <= aabbPos.x) ? aabbPos.x : ((pos.x > aabbPos.x + aabbDims.x) ?
           (aabbPos.x + aabbDims.x) : pos.x),
           (pos.y <= aabbPos.y) ? aabbPos.y : ((pos.y > aabbPos.y + aabbDims.y) ?
           (aabbPos.y + aabbDims.y) : pos.y),
           (pos.z <= aabbPos.z) ? aabbPos.z : ((pos.z > aabbPos.z + aabbDims.z) ?
           (aabbPos.z + aabbDims.z) : pos.z));
}
inline f64v3 getClosestPointOnAABB(const f64v3& pos, const f64v3& aabbPos,
                                   const f64v3& aabbDims) {
    return f64v3((pos.x <= aabbPos.x) ? aabbPos.x : ((pos.x > aabbPos.x + aabbDims.x) ?
           (aabbPos.x + aabbDims.x) : pos.x),
           (pos.y <= aabbPos.y) ? aabbPos.y : ((pos.y > aabbPos.y + aabbDims.y) ?
           (aabbPos.y + aabbDims.y) : pos.y),
           (pos.z <= aabbPos.z) ? aabbPos.z : ((pos.z > aabbPos.z + aabbDims.z) ?
           (aabbPos.z + aabbDims.z) : pos.z));
}

/// Moves val towards target in increments of step
template <typename T>
inline void stepTowards(T& val, const T& target, const T& step) {
    if (val < target) {
        val += step;
        if (val > target) val = target;
    } else if (val > target) {
        val -= step;
        if (val < target) val = target;
    }
}

/// Gets dot product with self, cheaper than vmath::dot because less copy
inline f32 selfDot(const f32v3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
inline f32 selfDot(const f32v4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
inline f64 selfDot(const f64v3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
inline f64 selfDot(const f64v4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
inline i32 selfDot(const i32v3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
inline i32 selfDot(const i32v4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
inline ui32 selfDot(const ui32v3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
inline ui32 selfDot(const ui32v4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

inline bool dumpFramebufferImage(const nString& rootDir, const ui32v4& viewport) {
    std::vector<ui8v4> pixels;
    int width = (viewport.z - viewport.x);
    int height = (viewport.w - viewport.y);
    pixels.resize(width * height);
    // Read pixels from framebuffer
    glReadPixels(viewport.x, viewport.y, viewport.z, viewport.w, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Use time to get file path
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%X");
    nString path = rootDir + "img-" + ss.str() + ".png";
    std::replace(path.begin(), path.end(), ':', '-');
    // Save screenshot
    if (!vg::ImageIO().save(path, pixels.data(), width, height, vg::ImageIOFormat::RGBA_UI8)) return false;

    printf("Made screenshot %s\n", path.c_str());
    return true;
}

// No idea how this works. Something to do with prime numbers, but returns # between -1 and 1
// It has poor distribution
inline f64 pseudoRand(int x, int z) {
    int n = (x & 0xFFFF) + ((z & 0x7FFF) << 16);
    n = (n << 13) ^ n;
    int nn = (n*(n*n * 60493 + z * 19990303) + x * 1376312589) & 0x7fffffff;
    return ((f64)nn / 1073741824.0);
}

// Thread safe version of intel's fast random number generator
inline ui32 fastRand(ui32 seed) {
    return ((214013u * seed + 2531011u) >> 16) & RAND_MAX;
}

#define FULL_64 0xFFFFFFFFFFFFFFFFu

// Great distribution and only a bit slower than rand()
class FastRandGenerator {
public:
    FastRandGenerator() { m_seed[0] = 214013u; m_seed[1] = 2531011u; };
    template<typename T>
    FastRandGenerator(T seedX) { seed(seedX); }
    template<typename T>
    FastRandGenerator(T seedX, T seedY) { seed(seedX, seedY); }
    template<typename T>
    FastRandGenerator(T seedX, T seedY, T seedZ) { seed(seedX, seedY, seedZ); }

    // Seeds the generator
    // TODO(Ben): Not sure how good this seeding is...
    template<typename T>
    inline void seed(T seed) {
        std::hash<T> h;
        m_seed[0] = ((ui64)h(seed) << 32) | (ui64)seed;
        m_seed[1] = m_seed[0] | (m_seed[0] << 32);
        gen();
    }
    template<typename T>
    inline void seed(T seedX, T seedY) {
        std::hash<T> h;
        ui64 hx = h(seedX);
        ui64 hy = h(seedY);
        m_seed[0] = (ui64)fastRand(hx) | (((ui64)hy + 214013u) << 32);
        m_seed[1] = (ui64)fastRand(hy) | (m_seed[0] << 32);
        gen();
    }
    template<typename T>
    inline void seed(T seedX, T seedY, T seedZ) {
        std::hash<T> h;
        ui64 hx = h(seedX);
        ui64 hy = h(seedY);
        m_seed[0] = (ui64)hx | ((ui64)hy << 32);
        m_seed[1] = ((ui64)hy | ((ui64)hx << 32)) ^ (ui64)seedZ;
        gen();
    }

    // Generates a random 64 bit number
    inline ui64 gen() {
        ui64 x = m_seed[0];
        const ui64 y = m_seed[1];
        m_seed[0] = y;
        x ^= x << 23; // a
        x ^= x >> 17; // b
        x ^= y ^ (y >> 26); // c
        m_seed[1] = x;
        return x + y;
    }
   
    // Generates number between 0 and 1
    inline f64 genlf() {
        ui64 x = m_seed[0];
        const ui64 y = m_seed[1];
        m_seed[0] = y;
        x ^= x << 23; // a
        x ^= x >> 17; // b
        x ^= y ^ (y >> 26); // c
        m_seed[1] = x;
        return (f64)(x + y) / (f64)FULL_64;
    }

private:
    ui64 m_seed[2];
};


// Math stuff //TODO(Ben): Move this to vorb?
// atan2 approximation for doubles for GLSL
// using http://lolengine.net/wiki/doc/maths/remez
inline f64 fastAtan2(f64 y, f64 x) {
    const f64 atan_tbl[] = {
        -3.333333333333333333333333333303396520128e-1,
        1.999999117496509842004185053319506031014e-1,
        -1.428514132711481940637283859690014415584e-1,
        1.110012236849539584126568416131750076191e-1,
        -8.993611617787817334566922323958104463948e-2,
        7.212338962134411520637759523226823838487e-2,
        -5.205055255952184339031830383744136009889e-2,
        2.938542391751121307313459297120064977888e-2,
        -1.079891788348568421355096111489189625479e-2,
        1.858552116405489677124095112269935093498e-3
    };

    /* argument reduction:
    arctan (-x) = -arctan(x);
    arctan (1/x) = 1/2 * pi - arctan (x), when x > 0
    */

    f64 ax = abs(x);
    f64 ay = abs(y);
    f64 t0 = ax > ay ? ax : ay; // max
    f64 t1 = ax < ay ? ax : ay; // min

    f64 a = 1 / t0;
    a *= t1;

    f64 s = a * a;
    f64 p = atan_tbl[9];

    p = fma(fma(fma(fma(fma(fma(fma(fma(fma(fma(p, s,
        atan_tbl[8]), s,
        atan_tbl[7]), s,
        atan_tbl[6]), s,
        atan_tbl[5]), s,
        atan_tbl[4]), s,
        atan_tbl[3]), s,
        atan_tbl[2]), s,
        atan_tbl[1]), s,
        atan_tbl[0]), s*a, a);

    f64 r = ay > ax ? (1.57079632679489661923 - p) : p;

    r = x < 0 ? 3.14159265358979323846 - r : r;
    r = y < 0 ? -r : r;

    return r;
}

/// For logarithmic z-buffer shaders
inline f32 computeZCoef(f32 zFar) {
    return 2.0f / log2(zFar + 1.0f);
}

/// Get distance from a chunk
inline f32 computeDistance2FromChunk(const f64v3& chunkPos, const f64v3& p) {
    f64 dx = ((p.x <= chunkPos.x) ? chunkPos.x : ((p.x > chunkPos.x + CHUNK_WIDTH) ? (chunkPos.x + CHUNK_WIDTH) : p.x));
    f64 dy = ((p.y <= chunkPos.y) ? chunkPos.y : ((p.y > chunkPos.y + CHUNK_WIDTH) ? (chunkPos.y + CHUNK_WIDTH) : p.y));
    f64 dz = ((p.z <= chunkPos.z) ? chunkPos.z : ((p.z > chunkPos.z + CHUNK_WIDTH) ? (chunkPos.z + CHUNK_WIDTH) : p.z));
    dx = dx - p.x;
    dy = dy - p.y;
    dz = dz - p.z;
    // We don't sqrt the distance since sqrt is slow
    return (f32)(dx*dx + dy*dy + dz*dz);
}

#endif // soaUtils_h__
