///
/// Noise.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Jul 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// 
///

#pragma once

#ifndef Noise_h__
#define Noise_h__

#include <Vorb/io/Keg.h>

enum class TerrainStage {
    NOISE,
    SQUARED,
    CUBED,
    RIDGED_NOISE,
    ABS_NOISE,
    SQUARED_NOISE,
    CUBED_NOISE,
    CELLULAR_NOISE,
    CELLULAR_SQUARED_NOISE,
    CELLULAR_CUBED_NOISE,
    CONSTANT,
    PASS_THROUGH
};
KEG_ENUM_DECL(TerrainStage);

enum class TerrainOp {
    ADD = 0,
    SUB,
    MUL,
    DIV
};
KEG_ENUM_DECL(TerrainOp);

struct TerrainFuncProperties {
    TerrainStage func = TerrainStage::NOISE;
    TerrainOp op = TerrainOp::ADD;
    int octaves = 1;
    f64 persistence = 1.0;
    f64 frequency = 1.0;
    f64 low = -1.0;
    f64 high = 1.0;
    f64v2 clamp = f64v2(0.0);
    Array<TerrainFuncProperties> children;
};
KEG_TYPE_DECL(TerrainFuncProperties);

struct NoiseBase {
    f64 base = 0.0f;
    Array<TerrainFuncProperties> funcs;
};
KEG_TYPE_DECL(NoiseBase);

namespace Noise {
    f64v2 cellular(const f64v3& P);

    // Mulit-octave simplex noise
    f64 fractal(const int octaves,
                const f64 persistence,
                const f64 freq,
                const f64 x,
                const f64 y);
    f64 fractal(const int octaves,
                const f64 persistence,
                const f64 freq,
                const f64 x,
                const f64 y,
                const f64 z);
    f64 fractal(const int octaves,
                const f64 persistence,
                const f64 freq,
                const f64 x,
                const f64 y,
                const f64 z,
                const f64 w);

    // Raw Simplex noise - a single noise value.
    f64 raw(const f64 x, const f64 y);
    f64 raw(const f64 x, const f64 y, const f64 z);
    f64 raw(const f64 x, const f64 y, const f64, const f64 w);

    // Scaled Multi-octave Simplex noise
    // The result will be between the two parameters passed.
    inline f64 scaledFractal(const int octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y) {
        return fractal(octaves, persistence, freq, x, y) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
    }
    inline f64 scaledFractal(const int octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z) {
        return fractal(octaves, persistence, freq, x, y, z) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
    }
    inline f64 scaledFractal(const int octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z, const f64 w) {
        return fractal(octaves, persistence, freq, x, y, z, w) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
    }

    // Scaled Raw Simplex noise
    // The result will be between the two parameters passed.
    inline f64 scaledRaw(const f64 loBound, const f64 hiBound, const f64 x, const f64 y) {
        return raw(x, y) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
    }
    inline f64 scaledRaw(const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z) {
        return raw(x, y, z) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
    }
    inline f64 scaledRaw(const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z, const f64 w) {
        return raw(x, y, z, w) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
    }

    // The gradients are the midpoints of the vertices of a cube.
    const f64 grad3[12][3] = {
        { 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 },
        { 1, 0, 1 }, { -1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 },
        { 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }
    };

    // The gradients are the midpoints of the vertices of a hypercube.
    const f64 grad4[32][4] = {
        { 0, 1, 1, 1 }, { 0, 1, 1, -1 }, { 0, 1, -1, 1 }, { 0, 1, -1, -1 },
        { 0, -1, 1, 1 }, { 0, -1, 1, -1 }, { 0, -1, -1, 1 }, { 0, -1, -1, -1 },
        { 1, 0, 1, 1 }, { 1, 0, 1, -1 }, { 1, 0, -1, 1 }, { 1, 0, -1, -1 },
        { -1, 0, 1, 1 }, { -1, 0, 1, -1 }, { -1, 0, -1, 1 }, { -1, 0, -1, -1 },
        { 1, 1, 0, 1 }, { 1, 1, 0, -1 }, { 1, -1, 0, 1 }, { 1, -1, 0, -1 },
        { -1, 1, 0, 1 }, { -1, 1, 0, -1 }, { -1, -1, 0, 1 }, { -1, -1, 0, -1 },
        { 1, 1, 1, 0 }, { 1, 1, -1, 0 }, { 1, -1, 1, 0 }, { 1, -1, -1, 0 },
        { -1, 1, 1, 0 }, { -1, 1, -1, 0 }, { -1, -1, 1, 0 }, { -1, -1, -1, 0 }
    };


    // Permutation table.  The same list is repeated twice.
    const int perm[512] = {
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
        8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
        35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
        134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
        55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
        18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
        250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
        189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43,
        172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
        228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
        107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
        138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,

        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
        8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
        35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
        134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
        55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
        18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
        250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
        189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43,
        172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
        228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
        107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
        138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
    };


    // A lookup table to traverse the simplex around a given point in 4D.
    const int simplex[64][4] = {
        { 0, 1, 2, 3 }, { 0, 1, 3, 2 }, { 0, 0, 0, 0 }, { 0, 2, 3, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 2, 3, 0 },
        { 0, 2, 1, 3 }, { 0, 0, 0, 0 }, { 0, 3, 1, 2 }, { 0, 3, 2, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 3, 2, 0 },
        { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
        { 1, 2, 0, 3 }, { 0, 0, 0, 0 }, { 1, 3, 0, 2 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 2, 3, 0, 1 }, { 2, 3, 1, 0 },
        { 1, 0, 2, 3 }, { 1, 0, 3, 2 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 2, 0, 3, 1 }, { 0, 0, 0, 0 }, { 2, 1, 3, 0 },
        { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
        { 2, 0, 1, 3 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 3, 0, 1, 2 }, { 3, 0, 2, 1 }, { 0, 0, 0, 0 }, { 3, 1, 2, 0 },
        { 2, 1, 0, 3 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 3, 1, 0, 2 }, { 0, 0, 0, 0 }, { 3, 2, 0, 1 }, { 3, 2, 1, 0 }
    };
}

#endif // Noise_h__