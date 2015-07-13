#include "stdafx.h"
#include "Noise.h"

#include <Vorb/utils.h>

KEG_TYPE_DEF_SAME_NAME(NoiseBase, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, NoiseBase, base, F32);
    kt.addValue("funcs", keg::Value::array(offsetof(NoiseBase, funcs), keg::Value::custom(0, "TerrainFuncProperties", false)));
}

KEG_ENUM_DEF(TerrainStage, TerrainStage, kt) {
    kt.addValue("noise", TerrainStage::NOISE);
    kt.addValue("squared", TerrainStage::SQUARED);
    kt.addValue("cubed", TerrainStage::CUBED);
    kt.addValue("noise_ridged", TerrainStage::RIDGED_NOISE);
    kt.addValue("noise_abs", TerrainStage::ABS_NOISE);
    kt.addValue("noise_squared", TerrainStage::SQUARED_NOISE);
    kt.addValue("noise_cubed", TerrainStage::CUBED_NOISE);
    kt.addValue("noise_cellular", TerrainStage::CELLULAR_NOISE);
    kt.addValue("noise_cellular_squared", TerrainStage::CELLULAR_SQUARED_NOISE);
    kt.addValue("noise_cellular_cubed", TerrainStage::CELLULAR_CUBED_NOISE);
    kt.addValue("constant", TerrainStage::CONSTANT);
    kt.addValue("passthrough", TerrainStage::PASS_THROUGH);
}

KEG_ENUM_DEF(TerrainOp, TerrainOp, kt) {
    kt.addValue("add", TerrainOp::ADD);
    kt.addValue("sub", TerrainOp::SUB);
    kt.addValue("mul", TerrainOp::MUL);
    kt.addValue("div", TerrainOp::DIV);
}

KEG_TYPE_DEF_SAME_NAME(TerrainFuncProperties, kt) {
    kt.addValue("type", keg::Value::custom(offsetof(TerrainFuncProperties, func), "TerrainStage", true));
    kt.addValue("op", keg::Value::custom(offsetof(TerrainFuncProperties, op), "TerrainOp", true));
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncProperties, octaves, I32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncProperties, persistence, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncProperties, frequency, F32);
    kt.addValue("val", keg::Value::basic(offsetof(TerrainFuncProperties, low), keg::BasicType::F32));
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncProperties, low, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncProperties, high, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, TerrainFuncProperties, clamp, F32_V2);
    kt.addValue("children", keg::Value::array(offsetof(TerrainFuncProperties, children), keg::Value::custom(0, "TerrainFuncProperties", false)));
}


//
// Description : Array and textureless 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 
// Converted to C++ by Ben Arnold

// Permutation polynomial: (34x^2 + x) mod 289
f64v3 permute(const f64v3& x) {
    return glm::mod((34.0 * x + 1.0) * x, 289.0);
}

// TODO(Ben): Fastfloor?
f64v2 Noise::cellular(const f64v3& P) {
#define K 0.142857142857 // 1/7
#define Ko 0.428571428571 // 1/2-K/2
#define K2 0.020408163265306 // 1/(7*7)
#define Kz 0.166666666667 // 1/6
#define Kzo 0.416666666667 // 1/2-1/6*2
#define jitter 1.0 // smaller jitter gives more regular pattern

    f64v3 Pi = glm::mod(glm::floor(P), 289.0);
    f64v3 Pf = glm::fract(P) - 0.5;

    f64v3 Pfx = Pf.x + f64v3(1.0, 0.0, -1.0);
    f64v3 Pfy = Pf.y + f64v3(1.0, 0.0, -1.0);
    f64v3 Pfz = Pf.z + f64v3(1.0, 0.0, -1.0);

    f64v3 p = permute(Pi.x + f64v3(-1.0, 0.0, 1.0));
    f64v3 p1 = permute(p + Pi.y - 1.0);
    f64v3 p2 = permute(p + Pi.y);
    f64v3 p3 = permute(p + Pi.y + 1.0);

    f64v3 p11 = permute(p1 + Pi.z - 1.0);
    f64v3 p12 = permute(p1 + Pi.z);
    f64v3 p13 = permute(p1 + Pi.z + 1.0);

    f64v3 p21 = permute(p2 + Pi.z - 1.0);
    f64v3 p22 = permute(p2 + Pi.z);
    f64v3 p23 = permute(p2 + Pi.z + 1.0);

    f64v3 p31 = permute(p3 + Pi.z - 1.0);
    f64v3 p32 = permute(p3 + Pi.z);
    f64v3 p33 = permute(p3 + Pi.z + 1.0);

    f64v3 ox11 = glm::fract(p11*K) - Ko;
    f64v3 oy11 = glm::mod(glm::floor(p11*K), 7.0)*K - Ko;
    f64v3 oz11 = glm::floor(p11*K2)*Kz - Kzo; // p11 < 289 guaranteed

    f64v3 ox12 = glm::fract(p12*K) - Ko;
    f64v3 oy12 = glm::mod(glm::floor(p12*K), 7.0)*K - Ko;
    f64v3 oz12 = glm::floor(p12*K2)*Kz - Kzo;

    f64v3 ox13 = glm::fract(p13*K) - Ko;
    f64v3 oy13 = glm::mod(glm::floor(p13*K), 7.0)*K - Ko;
    f64v3 oz13 = glm::floor(p13*K2)*Kz - Kzo;

    f64v3 ox21 = glm::fract(p21*K) - Ko;
    f64v3 oy21 = glm::mod(glm::floor(p21*K), 7.0)*K - Ko;
    f64v3 oz21 = glm::floor(p21*K2)*Kz - Kzo;

    f64v3 ox22 = glm::fract(p22*K) - Ko;
    f64v3 oy22 = glm::mod(glm::floor(p22*K), 7.0)*K - Ko;
    f64v3 oz22 = glm::floor(p22*K2)*Kz - Kzo;

    f64v3 ox23 = glm::fract(p23*K) - Ko;
    f64v3 oy23 = glm::mod(glm::floor(p23*K), 7.0)*K - Ko;
    f64v3 oz23 = glm::floor(p23*K2)*Kz - Kzo;

    f64v3 ox31 = glm::fract(p31*K) - Ko;
    f64v3 oy31 = glm::mod(glm::floor(p31*K), 7.0)*K - Ko;
    f64v3 oz31 = glm::floor(p31*K2)*Kz - Kzo;

    f64v3 ox32 = glm::fract(p32*K) - Ko;
    f64v3 oy32 = glm::mod(glm::floor(p32*K), 7.0)*K - Ko;
    f64v3 oz32 = glm::floor(p32*K2)*Kz - Kzo;

    f64v3 ox33 = glm::fract(p33*K) - Ko;
    f64v3 oy33 = glm::mod(glm::floor(p33*K), 7.0)*K - Ko;
    f64v3 oz33 = glm::floor(p33*K2)*Kz - Kzo;

    f64v3 dx11 = Pfx + jitter*ox11;
    f64v3 dy11 = Pfy.x + jitter*oy11;
    f64v3 dz11 = Pfz.x + jitter*oz11;

    f64v3 dx12 = Pfx + jitter*ox12;
    f64v3 dy12 = Pfy.x + jitter*oy12;
    f64v3 dz12 = Pfz.y + jitter*oz12;

    f64v3 dx13 = Pfx + jitter*ox13;
    f64v3 dy13 = Pfy.x + jitter*oy13;
    f64v3 dz13 = Pfz.z + jitter*oz13;

    f64v3 dx21 = Pfx + jitter*ox21;
    f64v3 dy21 = Pfy.y + jitter*oy21;
    f64v3 dz21 = Pfz.x + jitter*oz21;

    f64v3 dx22 = Pfx + jitter*ox22;
    f64v3 dy22 = Pfy.y + jitter*oy22;
    f64v3 dz22 = Pfz.y + jitter*oz22;

    f64v3 dx23 = Pfx + jitter*ox23;
    f64v3 dy23 = Pfy.y + jitter*oy23;
    f64v3 dz23 = Pfz.z + jitter*oz23;

    f64v3 dx31 = Pfx + jitter*ox31;
    f64v3 dy31 = Pfy.z + jitter*oy31;
    f64v3 dz31 = Pfz.x + jitter*oz31;

    f64v3 dx32 = Pfx + jitter*ox32;
    f64v3 dy32 = Pfy.z + jitter*oy32;
    f64v3 dz32 = Pfz.y + jitter*oz32;

    f64v3 dx33 = Pfx + jitter*ox33;
    f64v3 dy33 = Pfy.z + jitter*oy33;
    f64v3 dz33 = Pfz.z + jitter*oz33;

    f64v3 d11 = dx11 * dx11 + dy11 * dy11 + dz11 * dz11;
    f64v3 d12 = dx12 * dx12 + dy12 * dy12 + dz12 * dz12;
    f64v3 d13 = dx13 * dx13 + dy13 * dy13 + dz13 * dz13;
    f64v3 d21 = dx21 * dx21 + dy21 * dy21 + dz21 * dz21;
    f64v3 d22 = dx22 * dx22 + dy22 * dy22 + dz22 * dz22;
    f64v3 d23 = dx23 * dx23 + dy23 * dy23 + dz23 * dz23;
    f64v3 d31 = dx31 * dx31 + dy31 * dy31 + dz31 * dz31;
    f64v3 d32 = dx32 * dx32 + dy32 * dy32 + dz32 * dz32;
    f64v3 d33 = dx33 * dx33 + dy33 * dy33 + dz33 * dz33;

    // Sort out the two smallest distances (F1, F2)
#if 0
    // Cheat and sort out only F1
    f64v3 d1 = glm::min(glm::min(d11, d12), d13);
    f64v3 d2 = glm::min(glm::min(d21, d22), d23);
    f64v3 d3 = glm::min(glm::min(d31, d32), d33);
    f64v3 d = glm::min(glm::min(d1, d2), d3);
    d.x = glm::min(glm::min(d.x, d.y), d.z);
    return glm::sqrt(d.xx); // F1 duplicated, no F2 computed
#else
    // Do it right and sort out both F1 and F2
    f64v3 d1a = glm::min(d11, d12);
    d12 = glm::max(d11, d12);
    d11 = glm::min(d1a, d13); // Smallest now not in d12 or d13
    d13 = glm::max(d1a, d13);
    d12 = glm::min(d12, d13); // 2nd smallest now not in d13
    f64v3 d2a = glm::min(d21, d22);
    d22 = glm::max(d21, d22);
    d21 = glm::min(d2a, d23); // Smallest now not in d22 or d23
    d23 = glm::max(d2a, d23);
    d22 = glm::min(d22, d23); // 2nd smallest now not in d23
    f64v3 d3a = glm::min(d31, d32);
    d32 = glm::max(d31, d32);
    d31 = glm::min(d3a, d33); // Smallest now not in d32 or d33
    d33 = glm::max(d3a, d33);
    d32 = glm::min(d32, d33); // 2nd smallest now not in d33
    f64v3 da = glm::min(d11, d21);
    d21 = glm::max(d11, d21);
    d11 = glm::min(da, d31); // Smallest now in d11
    d31 = glm::max(da, d31); // 2nd smallest now not in d31
    d11 = (d11.x < d11.y) ? d11 : f64v3(d11.y, d11.x, d11.z);
    d11 = (d11.x < d11.z) ? d11 : f64v3(d11.z, d11.y, d11.x);
    d12 = glm::min(d12, d21); // 2nd smallest now not in d21
    d12 = glm::min(d12, d22); // nor in d22
    d12 = glm::min(d12, d31); // nor in d31
    d12 = glm::min(d12, d32); // nor in d32
    d11 = f64v3(d11.x, glm::min(f64v2(d11.y, d11.z), f64v2(d12.x, d12.y))); // nor in d12.yz
    d11.y = glm::min(d11.y, d12.z); // Only two more to go
    d11.y = glm::min(d11.y, d11.z); // Done! (Phew!)
    return glm::sqrt(f64v2(d11.x, d11.y)); // F1, F2
#endif
}

// Multi-octave Simplex noise
// For each octave, a higher frequency/lower amplitude function will be added to the original.
// The higher the persistence [0-1], the more of each succeeding octave will be added.

//SOURCE
// http://www.6by9.net/simplex-noise-for-c-and-python/


#define offsetfmult 1.45

inline int fastfloor(const f64 x) { return x > 0 ? (int)x : (int)x - 1; }

inline f64 dot(const int* g, const f64 x, const f64 y) { return g[0] * x + g[1] * y; }
inline f64 dot(const int* g, const f64 x, const f64 y, const f64 z) { return g[0] * x + g[1] * y + g[2] * z; }
inline f64 dot(const int* g, const f64 x, const f64 y, const f64 z, const f64 w) { return g[0] * x + g[1] * y + g[2] * z + g[3] * w; }

f64 Noise::fractal(const int octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y) {
    f64 total = 0.0;
    f64 frequency = freq;
    f64 amplitude = 1.0;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0.0;

    for (int i = 0; i < octaves; i++) {
        total += raw(x * frequency, y * frequency) * amplitude;

        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 Noise::fractal(const int octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y, const f64 z) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0.0;

    for (int i = 0; i < octaves; i++) {
        total += raw(x * frequency, y * frequency, z * frequency) * amplitude;

        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 Noise::fractal(const int octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y, const f64 z, const f64 w) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    for (int i = 0; i < octaves; i++) {
        total += raw(x * frequency, y * frequency, z * frequency, w * frequency) * amplitude;

        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

// 2D raw Simplex noise
f64 Noise::raw(const f64 x, const f64 y) {
    // Noise contributions from the three corners
    f64 n0, n1, n2;

    // Skew the input space to determine which simplex cell we're in
    f64 F2 = 0.5 * (sqrtf(3.0) - 1.0);
    // Hairy factor for 2D
    f64 s = (x + y) * F2;
    int i = fastFloor(x + s);
    int j = fastFloor(y + s);

    f64 G2 = (3.0 - sqrtf(3.0)) / 6.0;
    f64 t = (i + j) * G2;
    // Unskew the cell origin back to (x,y) space
    f64 X0 = i - t;
    f64 Y0 = j - t;
    // The x,y distances from the cell origin
    f64 x0 = x - X0;
    f64 y0 = y - Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if (x0>y0) { i1 = 1; j1 = 0; } // lower triangle, XY order: (0,0)->(1,0)->(1,1)
    else { i1 = 0; j1 = 1; } // upper triangle, YX order: (0,0)->(0,1)->(1,1)

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    f64 x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    f64 y1 = y0 - j1 + G2;
    f64 x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
    f64 y2 = y0 - 1.0 + 2.0 * G2;

    // Work out the hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = perm[ii + perm[jj]] % 12;
    int gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
    int gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

    // Calculate the contribution from the three corners
    f64 t0 = 0.5 - x0*x0 - y0*y0;
    if (t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
    }

    f64 t1 = 0.5 - x1*x1 - y1*y1;
    if (t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
    }

    f64 t2 = 0.5 - x2*x2 - y2*y2;
    if (t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0 * (n0 + n1 + n2);
}

// 3D raw Simplex noise
f64 Noise::raw(const f64 x, const f64 y, const f64 z) {
    f64 n0, n1, n2, n3; // Noise contributions from the four corners

    // Skew the input space to determine which simplex cell we're in
    const f64 F3 = 1.0 / 3.0;
    f64 s = (x + y + z)*F3; // Very nice and simple skew factor for 3D
    int i = fastFloor(x + s);
    int j = fastFloor(y + s);
    int k = fastFloor(z + s);

    const f64 G3 = 1.0 / 6.0; // Very nice and simple unskew factor, too
    f64 t = (i + j + k)*G3;
    f64 X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    f64 Y0 = j - t;
    f64 Z0 = k - t;
    f64 x0 = x - X0; // The x,y,z distances from the cell origin
    f64 y0 = y - Y0;
    f64 z0 = z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

    if (x0 >= y0) {
        if (y0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // X Y Z order
        else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; } // X Z Y order
        else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; } // Z X Y order
    } else { // x0<y0
        if (y0<z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; } // Z Y X order
        else if (x0<z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; } // Y Z X order
        else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // Y X Z order
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    f64 x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    f64 y1 = y0 - j1 + G3;
    f64 z1 = z0 - k1 + G3;
    f64 x2 = x0 - i2 + 2.0*G3; // Offsets for third corner in (x,y,z) coords
    f64 y2 = y0 - j2 + 2.0*G3;
    f64 z2 = z0 - k2 + 2.0*G3;
    f64 x3 = x0 - 1.0 + 3.0*G3; // Offsets for last corner in (x,y,z) coords
    f64 y3 = y0 - 1.0 + 3.0*G3;
    f64 z3 = z0 - 1.0 + 3.0*G3;

    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = perm[ii + perm[jj + perm[kk]]] % 12;
    int gi1 = perm[ii + i1 + perm[jj + j1 + perm[kk + k1]]] % 12;
    int gi2 = perm[ii + i2 + perm[jj + j2 + perm[kk + k2]]] % 12;
    int gi3 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1]]] % 12;

    // Calculate the contribution from the four corners
    f64 t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if (t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
    }

    f64 t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if (t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
    }

    f64 t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if (t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
    }

    f64 t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if (t3<0) n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0 * (n0 + n1 + n2 + n3);
}

// 4D raw Simplex noise
f64 Noise::raw(const f64 x, const f64 y, const f64 z, const f64 w) {
    // The skewing and unskewing factors are hairy again for the 4D case
    f64 F4 = (sqrtf(5.0) - 1.0) / 4.0;
    f64 G4 = (5.0 - sqrtf(5.0)) / 20.0;
    f64 n0, n1, n2, n3, n4; // Noise contributions from the five corners

    // Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
    f64 s = (x + y + z + w) * F4; // Factor for 4D skewing
    int i = fastFloor(x + s);
    int j = fastFloor(y + s);
    int k = fastFloor(z + s);
    int l = fastFloor(w + s);
    f64 t = (i + j + k + l) * G4; // Factor for 4D unskewing
    f64 X0 = i - t; // Unskew the cell origin back to (x,y,z,w) space
    f64 Y0 = j - t;
    f64 Z0 = k - t;
    f64 W0 = l - t;

    f64 x0 = x - X0; // The x,y,z,w distances from the cell origin
    f64 y0 = y - Y0;
    f64 z0 = z - Z0;
    f64 w0 = w - W0;

    // For the 4D case, the simplex is a 4D shape I won't even try to describe.
    // To find out which of the 24 possible simplices we're in, we need to
    // determine the magnitude ordering of x0, y0, z0 and w0.
    // The method below is a good way of finding the ordering of x,y,z,w and
    // then find the correct traversal order for the simplex we're in.
    // First, six pair-wise comparisons are performed between each possible pair
    // of the four coordinates, and the results are used to add up binary bits
    // for an integer index.
    int c1 = (x0 > y0) ? 32 : 0;
    int c2 = (x0 > z0) ? 16 : 0;
    int c3 = (y0 > z0) ? 8 : 0;
    int c4 = (x0 > w0) ? 4 : 0;
    int c5 = (y0 > w0) ? 2 : 0;
    int c6 = (z0 > w0) ? 1 : 0;
    int c = c1 + c2 + c3 + c4 + c5 + c6;

    int i1, j1, k1, l1; // The integer offsets for the second simplex corner
    int i2, j2, k2, l2; // The integer offsets for the third simplex corner
    int i3, j3, k3, l3; // The integer offsets for the fourth simplex corner

    // simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
    // Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
    // impossible. Only the 24 indices which have non-zero entries make any sense.
    // We use a thresholding to set the coordinates in turn from the largest magnitude.
    // The number 3 in the "simplex" array is at the position of the largest coordinate.
    i1 = simplex[c][0] >= 3 ? 1 : 0;
    j1 = simplex[c][1] >= 3 ? 1 : 0;
    k1 = simplex[c][2] >= 3 ? 1 : 0;
    l1 = simplex[c][3] >= 3 ? 1 : 0;
    // The number 2 in the "simplex" array is at the second largest coordinate.
    i2 = simplex[c][0] >= 2 ? 1 : 0;
    j2 = simplex[c][1] >= 2 ? 1 : 0;
    k2 = simplex[c][2] >= 2 ? 1 : 0;
    l2 = simplex[c][3] >= 2 ? 1 : 0;
    // The number 1 in the "simplex" array is at the second smallest coordinate.
    i3 = simplex[c][0] >= 1 ? 1 : 0;
    j3 = simplex[c][1] >= 1 ? 1 : 0;
    k3 = simplex[c][2] >= 1 ? 1 : 0;
    l3 = simplex[c][3] >= 1 ? 1 : 0;
    // The fifth corner has all coordinate offsets = 1, so no need to look that up.

    f64 x1 = x0 - i1 + G4; // Offsets for second corner in (x,y,z,w) coords
    f64 y1 = y0 - j1 + G4;
    f64 z1 = z0 - k1 + G4;
    f64 w1 = w0 - l1 + G4;
    f64 x2 = x0 - i2 + 2.0*G4; // Offsets for third corner in (x,y,z,w) coords
    f64 y2 = y0 - j2 + 2.0*G4;
    f64 z2 = z0 - k2 + 2.0*G4;
    f64 w2 = w0 - l2 + 2.0*G4;
    f64 x3 = x0 - i3 + 3.0*G4; // Offsets for fourth corner in (x,y,z,w) coords
    f64 y3 = y0 - j3 + 3.0*G4;
    f64 z3 = z0 - k3 + 3.0*G4;
    f64 w3 = w0 - l3 + 3.0*G4;
    f64 x4 = x0 - 1.0 + 4.0*G4; // Offsets for last corner in (x,y,z,w) coords
    f64 y4 = y0 - 1.0 + 4.0*G4;
    f64 z4 = z0 - 1.0 + 4.0*G4;
    f64 w4 = w0 - 1.0 + 4.0*G4;

    // Work out the hashed gradient indices of the five simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int ll = l & 255;
    int gi0 = perm[ii + perm[jj + perm[kk + perm[ll]]]] % 32;
    int gi1 = perm[ii + i1 + perm[jj + j1 + perm[kk + k1 + perm[ll + l1]]]] % 32;
    int gi2 = perm[ii + i2 + perm[jj + j2 + perm[kk + k2 + perm[ll + l2]]]] % 32;
    int gi3 = perm[ii + i3 + perm[jj + j3 + perm[kk + k3 + perm[ll + l3]]]] % 32;
    int gi4 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1 + perm[ll + 1]]]] % 32;

    // Calculate the contribution from the five corners
    f64 t0 = 0.6 - x0*x0 - y0*y0 - z0*z0 - w0*w0;
    if (t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad4[gi0], x0, y0, z0, w0);
    }

    f64 t1 = 0.6 - x1*x1 - y1*y1 - z1*z1 - w1*w1;
    if (t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad4[gi1], x1, y1, z1, w1);
    }

    f64 t2 = 0.6 - x2*x2 - y2*y2 - z2*z2 - w2*w2;
    if (t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad4[gi2], x2, y2, z2, w2);
    }

    f64 t3 = 0.6 - x3*x3 - y3*y3 - z3*z3 - w3*w3;
    if (t3<0) n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad4[gi3], x3, y3, z3, w3);
    }

    f64 t4 = 0.6 - x4*x4 - y4*y4 - z4*z4 - w4*w4;
    if (t4<0) n4 = 0.0;
    else {
        t4 *= t4;
        n4 = t4 * t4 * dot(grad4[gi4], x4, y4, z4, w4);
    }

    // Sum up and scale the result to cover the range [-1,1]
    return 27.0 * (n0 + n1 + n2 + n3 + n4);
}
