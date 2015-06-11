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

f64v3 mod289(f64v3 x) {
    return x - glm::floor(x * (1.0 / 289.0)) * 289.0;
}

f64v4 mod289(f64v4 x) {
    return x - glm::floor(x * (1.0 / 289.0)) * 289.0;
}

f64v4 permute(f64v4 x) {
    return mod289(((x*34.0) + 1.0)*x);
}

f64v4 taylorInvSqrt(f64v4 r) {
    return 1.79284291400159f - 0.85373472095314f * r;
}

f64 CpuNoise::rawAshimaSimplex3D(f64v3 v) {
    const f64v2 C = f64v2(1.0 / 6.0, 1.0 / 3.0);
    const f64v4 D = f64v4(0.0, 0.5, 1.0, 2.0);

    // First corner
    f64v3 cyyy(C.y, C.y, C.y);
    f64v3 cxxx(C.x, C.x, C.x);
    f64v3 i = glm::floor(v + glm::dot(v, cyyy));
    f64v3 x0 = v - i + glm::dot(i, cxxx);

    // Other corners
    f64v3 g = glm::step(f64v3(x0.y, x0.z, x0.x), x0);
    f64v3 l = 1.0 - g;
    f64v3 lzxy(l.z, l.x, l.y);
    f64v3 i1 = glm::min(g, lzxy);
    f64v3 i2 = glm::max(g, lzxy);

    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    f64v3 x1 = x0 - i1 + cxxx;
    f64v3 x2 = x0 - i2 + cyyy; // 2.0*C.x = 1/3 = C.y
    f64v3 x3 = x0 - f64v3(D.y, D.y, D.y);      // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i);
    f64v4 p = permute(permute(permute(
        i.z + f64v4(0.0, i1.z, i2.z, 1.0))
        + i.y + f64v4(0.0, i1.y, i2.y, 1.0))
        + i.x + f64v4(0.0, i1.x, i2.x, 1.0));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    f64 n_ = 0.142857142857; // 1.0/7.0
    f64v3 ns = n_ * f64v3(D.w, D.y, D.z) - f64v3(D.x, D.z, D.x);

    f64v4 j = p - 49.0 * glm::floor(p * ns.z * ns.z);  //  mod(p,7*7)

    f64v4 x_ = glm::floor(j * ns.z);
    f64v4 y_ = glm::floor(j - 7.0 * x_);    // mod(j,N)

    f64v4 nsyyyy(ns.y, ns.y, ns.y, ns.y);
    f64v4 x = x_ *ns.x + nsyyyy;
    f64v4 y = y_ *ns.x + nsyyyy;
    f64v4 h = 1.0 - glm::abs(x) - glm::abs(y);

    f64v4 b0 = f64v4(x.x, x.y, y.x, y.y);
    f64v4 b1 = f64v4(x.z, x.w, y.z, y.w);

    //f64v4 s0 = f64v4(lessThan(b0,0.0))*2.0 - 1.0;
    //f64v4 s1 = f64v4(lessThan(b1,0.0))*2.0 - 1.0;
    f64v4 s0 = glm::floor(b0)*2.0 + 1.0;
    f64v4 s1 = glm::floor(b1)*2.0 + 1.0;
    f64v4 sh = -glm::step(h, f64v4(0.0));

    f64v4 a0 = f64v4(b0.x, b0.z, b0.y, b0.w) + f64v4(s0.x, s0.z, s0.y, s0.w) * f64v4(sh.x, sh.x, sh.y, sh.y);
    f64v4 a1 = f64v4(s1.x, s1.z, s1.y, s1.w) + f64v4(s1.x, s1.z, s1.y, s1.w) * f64v4(sh.z, sh.z, sh.w, sh.w);

    f64v3 p0 = f64v3(a0.x, a0.y, h.x);
    f64v3 p1 = f64v3(a0.z, a0.w, h.y);
    f64v3 p2 = f64v3(a1.x, a1.y, h.z);
    f64v3 p3 = f64v3(a1.z, a1.w, h.w);

    //Normalize gradients
    f64v4 norm = taylorInvSqrt(f64v4(glm::dot(p0, p0), glm::dot(p1, p1), glm::dot(p2, p2), glm::dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    f64v4 m = glm::max(0.6 - f64v4(glm::dot(x0, x0), glm::dot(x1, x1), glm::dot(x2, x2), glm::dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * glm::dot(m*m, f64v4(glm::dot(p0, x0), glm::dot(p1, x1),
        glm::dot(p2, x2), glm::dot(p3, x3)));
}