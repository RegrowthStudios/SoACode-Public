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

f64 CpuNoise::rawAshimaSimplex3D(const f64v3& v) {
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

// Permutation polynomial: (34x^2 + x) mod 289
f64v3 permute(const f64v3& x) {
    return glm::mod((34.0 * x + 1.0) * x, 289.0);
}

f64v2 CpuNoise::cellular(const f64v3& P) {
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
    d11 = (d11.x < d11.y) ? f64v3(d11.x, d11.y, d11.z) : f64v3(d11.y, d11.x, d11.z);
    d11 = (d11.x < d11.z) ? f64v3(d11.x, d11.y, d11.z) : f64v3(d11.z, d11.y, d11.x);
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