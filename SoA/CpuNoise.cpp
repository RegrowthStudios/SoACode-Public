#include "stdafx.h"
#include "CpuNoise.h"

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

f32v3 mod289(f32v3 x) {
    return x - glm::floor(x * (1.0f / 289.0f)) * 289.0f;
}

f32v4 mod289(f32v4 x) {
    return x - glm::floor(x * (1.0f / 289.0f)) * 289.0f;
}

f32v4 permute(f32v4 x) {
    return mod289(((x*34.0f) + 1.0f)*x);
}

f32v4 taylorInvSqrt(f32v4 r) {
    return 1.79284291400159f - 0.85373472095314f * r;
}

float CpuNoise::rawAshimaSimplex3D(f32v3 v) {
    const f32v2  C = f32v2(1.0f / 6.0f, 1.0f / 3.0f);
    const f32v4  D = f32v4(0.0f, 0.5f, 1.0f, 2.0f);

    // First corner
    f32v3 cyyy(C.y, C.y, C.y);
    f32v3 cxxx(C.x, C.x, C.x);
    f32v3 i = glm::floor(v + glm::dot(v, cyyy));
    f32v3 x0 = v - i + glm::dot(i, cxxx);

    // Other corners
    f32v3 g = glm::step(f32v3(x0.y, x0.z, x0.x), x0);
    f32v3 l = 1.0f - g;
    f32v3 lzxy(l.z, l.x, l.y);
    f32v3 i1 = glm::min(g, lzxy);
    f32v3 i2 = glm::max(g, lzxy);

    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    f32v3 x1 = x0 - i1 + cxxx;
    f32v3 x2 = x0 - i2 + cyyy; // 2.0*C.x = 1/3 = C.y
    f32v3 x3 = x0 - f32v3(D.y, D.y, D.y);      // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i);
    f32v4 p = permute(permute(permute(
        i.z + f32v4(0.0, i1.z, i2.z, 1.0))
        + i.y + f32v4(0.0, i1.y, i2.y, 1.0))
        + i.x + f32v4(0.0, i1.x, i2.x, 1.0));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857f; // 1.0/7.0
    f32v3  ns = n_ * f32v3(D.w, D.y, D.z) - f32v3(D.x, D.z, D.z);

    f32v4 j = p - 49.0f * glm::floor(p * ns.z * ns.z);  //  mod(p,7*7)

    f32v4 x_ = glm::floor(j * ns.z);
    f32v4 y_ = glm::floor(j - 7.0f * x_);    // mod(j,N)

    f32v4 nsyyyy(ns.y, ns.y, ns.y, ns.y);
    f32v4 x = x_ *ns.x + nsyyyy;
    f32v4 y = y_ *ns.x + nsyyyy;
    f32v4 h = 1.0f - glm::abs(x) - glm::abs(y);

    f32v4 b0 = f32v4(x.x, x.y, y.x, y.y);
    f32v4 b1 = f32v4(x.z, x.w, y.z, y.w);

    //f32v4 s0 = f32v4(lessThan(b0,0.0))*2.0 - 1.0;
    //f32v4 s1 = f32v4(lessThan(b1,0.0))*2.0 - 1.0;
    f32v4 s0 = glm::floor(b0)*2.0f + 1.0f;
    f32v4 s1 = glm::floor(b1)*2.0f + 1.0f;
    f32v4 sh = -glm::step(h, f32v4(0.0f));

    f32v4 a0 = f32v4(b0.x, b0.z, b0.y, b0.w) + f32v4(s0.x, s0.z, s0.y, s0.w) * f32v4(sh.x, sh.x, sh.y, sh.y);
    f32v4 a1 = f32v4(s1.x, s1.z, s1.y, s1.w) + f32v4(s1.x, s1.z, s1.y, s1.w) * f32v4(sh.z, sh.z, sh.w, sh.w);

    f32v3 p0 = f32v3(a0.x, a0.y, h.x);
    f32v3 p1 = f32v3(a0.z, a0.w, h.y);
    f32v3 p2 = f32v3(a1.x, a1.y, h.z);
    f32v3 p3 = f32v3(a1.z, a1.w, h.w);

    //Normalize gradients
    f32v4 norm = taylorInvSqrt(f32v4(glm::dot(p0, p0), glm::dot(p1, p1), glm::dot(p2, p2), glm::dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    f32v4 m = glm::max(0.6f - f32v4(glm::dot(x0, x0), glm::dot(x1, x1), glm::dot(x2, x2), glm::dot(x3, x3)), 0.0f);
    m = m * m;
    return 42.0f * glm::dot(m*m, f32v4(glm::dot(p0, x0), glm::dot(p1, x1),
        glm::dot(p2, x2), glm::dot(p3, x3)));
}