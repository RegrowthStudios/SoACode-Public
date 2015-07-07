#include "stdafx.h"
#include "SimplexNoise.h"

#include <Vorb/utils.h>

#define offsetfmult 1.45


inline int fastfloor(const f64 x) { return x > 0 ? (int)x : (int)x - 1; }

inline f64 dot(const int* g, const f64 x, const f64 y) { return g[0] * x + g[1] * y; }
inline f64 dot(const int* g, const f64 x, const f64 y, const f64 z) { return g[0] * x + g[1] * y + g[2] * z; }
inline f64 dot(const int* g, const f64 x, const f64 y, const f64 z, const f64 w) { return g[0] * x + g[1] * y + g[2] * z + g[3] * w; }


// For each octave, a higher frequency/lower amplitude function will be added to the original.
// The higher the persistence [0-1], the more of each succeeding octave will be added.
f64 ridged_octave_noise_2d( const f64 octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y ) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;
    f64 d;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    for( int i=0; i < octaves; i++ ) {
        d = raw_noise_2d( x * frequency, y * frequency );
        if (i > 12){
            if (d < 0) d = -d;
        }else if (i > 0){ //first octave sets the base
            if (d < 0) d = -d;
            d = 1.0 - d;
        }
        total += d * amplitude;

        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 reverse_ridged_octave_noise_2d( const f64 octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y ) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;
    f64 d;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    for( int i=0; i < octaves; i++ ) {
        d = raw_noise_2d( x * frequency, y * frequency );
        if (i > 0){ //first octave sets the base
            if (d < 0){
                d = -d;
            }
            d = d;
        }
        total += d * amplitude;

        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 ridged_octave_noise_3d( const f64 octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y,  const f64 z ) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;
    f64 d;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    for( int i=0; i < octaves; i++ ) {
        d = raw_noise_3d( x * frequency, y * frequency, z * frequency );
        if (i > 12){
            if (d < 0) d = -d;
        }else if (i > 0){ //first octave sets the base
            if (d < 0) d = -d;
            d = 1.0 - d;
        }
        total += d * amplitude;

        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 reverse_ridged_octave_noise_3d( const f64 octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y, const f64 z) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;
    f64 d;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    for( int i=0; i < octaves; i++ ) {
        d = raw_noise_3d( x * frequency, y * frequency, z * frequency );
        if (i > 0){ //first octave sets the base
            if (d < 0){
                d = -d;
            }
            d = d;
        }
        total += d * amplitude;

        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 scaled_ridged_octave_noise_3d( const f64 octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z ) {
    return ridged_octave_noise_3d(octaves, persistence, freq, x, y, z) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

f64 scaled_reverse_ridged_octave_noise_3d( const f64 octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z ) {
    return reverse_ridged_octave_noise_3d(octaves, persistence, freq, x, y, z) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}


f64 octave_noise_2d( const f64 octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y ) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    for( int i=0; i < octaves; i++ ) {
        total += raw_noise_2d( x * frequency, y * frequency ) * amplitude;

        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 ridged_octave_noise_3d_1( const f64 octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y, const f64 z ) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;
    f64 d;
    f64 mult = 4;
    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    //******************** Make it so that successive noise cannot delete cave, and can only create it?
    for( int i=0; i < octaves; i++ ) {
    
        d = raw_noise_3d((y+raw_noise_2d(y * frequency*4, x * frequency*4)*mult) * frequency, (z + raw_noise_2d(z * frequency*4, y * frequency*4)*mult) * frequency, (x+raw_noise_2d(x * frequency*4, z * frequency*4)*mult) * frequency);
    
        if (d < 0) d = -d;

        d = (1.0 - (d)); //first four octaves have extra weight
    
    //    if (d < 0.9) d = 0.9;

        total += d * amplitude;
        maxAmplitude += amplitude;
        frequency *= 3;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 ridged_octave_noise_3d_2( const f64 octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y, const f64 z ) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;
    f64 d;
    f64 mult = 4;
    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    //******************** Make it so that successive noise cannot delete cave, and can only create it?
    for( int i=0; i < octaves; i++ ) {    
        d = raw_noise_3d((y+raw_noise_2d(-z*frequency*offsetfmult*0.9 + y * frequency*offsetfmult*1.2, x * frequency*offsetfmult*0.7)*mult) * frequency, (z + raw_noise_2d(z * frequency*offsetfmult*0.8, y * frequency*offsetfmult*0.69 - x * frequency*offsetfmult*1.17)*mult) * frequency, (x+raw_noise_2d(x * frequency*offsetfmult*0.86, -y*frequency*offsetfmult*0.9 + z * frequency*offsetfmult)*mult) * frequency);
    
        d = (1.0 - (d*d)); 
    
        //if (i < 2 && d > .96){
        //    d = .95 - (d-.95);
            //if (d < .93) d = .93;
    //        d = .96;
    //    }//else if (i > 1 && d < 0.5){
        //    d = 1.0 - d;
        //}
    //if (i > 0){
        total += d * amplitude;
        maxAmplitude += amplitude;
    //}
        frequency *= 2.5;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 octave_noise_3d( const f64 octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y, const f64 z ) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    for( int i=0; i < octaves; i++ ) {
        total += raw_noise_3d( x * frequency, y * frequency, z * frequency ) * amplitude;

        frequency *= 2;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 octave_noise_4d( const f64 octaves, const f64 persistence, const f64 freq, const f64 x, const f64 y, const f64 z, const f64 w ) {
    f64 total = 0;
    f64 frequency = freq;
    f64 amplitude = 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    f64 maxAmplitude = 0;

    for( int i=0; i < octaves; i++ ) {
        total += raw_noise_4d( x * frequency, y * frequency, z * frequency, w * frequency ) * amplitude;

        frequency *= 2;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

f64 scaled_ridged_octave_noise_2d( const f64 octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y ) {
    return ridged_octave_noise_2d(octaves, persistence, freq, x, y) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

f64 scaled_reverse_ridged_octave_noise_2d( const f64 octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y ) {
    return reverse_ridged_octave_noise_2d(octaves, persistence, freq, x, y) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}


f64 scaled_octave_noise_2d( const f64 octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y ) {
    return octave_noise_2d(octaves, persistence, freq, x, y) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

f64 scaled_octave_noise_3d( const f64 octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z ) {
    return octave_noise_3d(octaves, persistence, freq, x, y, z) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

f64 scaled_octave_noise_4d( const f64 octaves, const f64 persistence, const f64 freq, const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z, const f64 w ) {
    return octave_noise_4d(octaves, persistence, freq, x, y, z, w) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}


f64 scaled_raw_noise_2d( const f64 loBound, const f64 hiBound, const f64 x, const f64 y ) {
    return raw_noise_2d(x, y) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

f64 scaled_raw_noise_3d( const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z ) {
    return raw_noise_3d(x, y, z) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

f64 scaled_raw_noise_4d( const f64 loBound, const f64 hiBound, const f64 x, const f64 y, const f64 z, const f64 w ) {
    return raw_noise_4d(x, y, z, w) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

// 2D raw Simplex noise
f64 raw_noise_2d( const f64 x, const f64 y ) {
    // Noise contributions from the three corners
    f64 n0, n1, n2;

    // Skew the input space to determine which simplex cell we're in
    f64 F2 = 0.5 * (sqrtf(3.0) - 1.0);
    // Hairy factor for 2D
    f64 s = (x + y) * F2;
    int i = fastFloor( x + s );
    int j = fastFloor(y + s);

    f64 G2 = (3.0 - sqrtf(3.0)) / 6.0;
    f64 t = (i + j) * G2;
    // Unskew the cell origin back to (x,y) space
    f64 X0 = i-t;
    f64 Y0 = j-t;
    // The x,y distances from the cell origin
    f64 x0 = x-X0;
    f64 y0 = y-Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
    else {i1=0; j1=1;} // upper triangle, YX order: (0,0)->(0,1)->(1,1)

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
    int gi0 = perm[ii+perm[jj]] % 12;
    int gi1 = perm[ii+i1+perm[jj+j1]] % 12;
    int gi2 = perm[ii+1+perm[jj+1]] % 12;

    // Calculate the contribution from the three corners
    f64 t0 = 0.5 - x0*x0-y0*y0;
    if(t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
    }

    f64 t1 = 0.5 - x1*x1-y1*y1;
    if(t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
    }

    f64 t2 = 0.5 - x2*x2-y2*y2;
    if(t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0 * (n0 + n1 + n2);
}

float raw_noise_3df(const float x, const float y, const float z) {
    float n0, n1, n2, n3; // Noise contributions from the four corners

    // Skew the input space to determine which simplex cell we're in
    float F3 = 1.0 / 3.0;
    float s = (x + y + z)*F3; // Very nice and simple skew factor for 3D
    int i = fastFloor(x + s);
    int j = fastFloor(y + s);
    int k = fastFloor(z + s);

    float G3 = 1.0 / 6.0; // Very nice and simple unskew factor, too
    float t = (i + j + k)*G3;
    float X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; // The x,y,z distances from the cell origin
    float y0 = y - Y0;
    float z0 = z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

    if (x0 >= y0) {
        if (y0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // X Y Z order
        else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; } // X Z Y order
        else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; } // Z X Y order
    } else { // x0<y0
        if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; } // Z Y X order
        else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; } // Y Z X order
        else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // Y X Z order
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f*G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0f*G3;
    float z2 = z0 - k2 + 2.0f*G3;
    float x3 = x0 - 1.0 + 3.0f*G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0 + 3.0f*G3;
    float z3 = z0 - 1.0 + 3.0f*G3;

    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = perm[ii + perm[jj + perm[kk]]] % 12;
    int gi1 = perm[ii + i1 + perm[jj + j1 + perm[kk + k1]]] % 12;
    int gi2 = perm[ii + i2 + perm[jj + j2 + perm[kk + k2]]] % 12;
    int gi3 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1]]] % 12;

    // Calculate the contribution from the four corners
    float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0;
    if (t0 < 0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
    }

    float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1;
    if (t1 < 0) n1 = 0.0f;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
    }

    float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2;
    if (t2 < 0) n2 = 0.0f;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
    }

    float t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if (t3 < 0) n3 = 0.0f;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0f*(n0 + n1 + n2 + n3);
}

// 3D raw Simplex noise
f64 raw_noise_3d( const f64 x, const f64 y, const f64 z ) {
    f64 n0, n1, n2, n3; // Noise contributions from the four corners

    // Skew the input space to determine which simplex cell we're in
    const f64 F3 = 1.0/3.0;
    f64 s = (x+y+z)*F3; // Very nice and simple skew factor for 3D
    int i = fastFloor(x + s);
    int j = fastFloor(y + s);
    int k = fastFloor(z + s);

    const f64 G3 = 1.0/6.0; // Very nice and simple unskew factor, too
    f64 t = (i+j+k)*G3;
    f64 X0 = i-t; // Unskew the cell origin back to (x,y,z) space
    f64 Y0 = j-t;
    f64 Z0 = k-t;
    f64 x0 = x-X0; // The x,y,z distances from the cell origin
    f64 y0 = y-Y0;
    f64 z0 = z-Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

    if(x0>=y0) {
        if(y0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
        else if(x0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
        else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Y order
    }
    else { // x0<y0
        if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
        else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
        else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
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
    int gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
    int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
    int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
    int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;

    // Calculate the contribution from the four corners
    f64 t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if(t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
    }

    f64 t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if(t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
    }

    f64 t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if(t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
    }

    f64 t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if(t3<0) n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0 * (n0 + n1 + n2 + n3);
}

// 4D raw Simplex noise
f64 raw_noise_4d( const f64 x, const f64 y, const f64 z, const f64 w ) {
    // The skewing and unskewing factors are hairy again for the 4D case
    f64 F4 = (sqrtf(5.0)-1.0)/4.0;
    f64 G4 = (5.0-sqrtf(5.0))/20.0;
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
    i1 = simplex[c][0]>=3 ? 1 : 0;
    j1 = simplex[c][1]>=3 ? 1 : 0;
    k1 = simplex[c][2]>=3 ? 1 : 0;
    l1 = simplex[c][3]>=3 ? 1 : 0;
    // The number 2 in the "simplex" array is at the second largest coordinate.
    i2 = simplex[c][0]>=2 ? 1 : 0;
    j2 = simplex[c][1]>=2 ? 1 : 0;
    k2 = simplex[c][2]>=2 ? 1 : 0;
    l2 = simplex[c][3]>=2 ? 1 : 0;
    // The number 1 in the "simplex" array is at the second smallest coordinate.
    i3 = simplex[c][0]>=1 ? 1 : 0;
    j3 = simplex[c][1]>=1 ? 1 : 0;
    k3 = simplex[c][2]>=1 ? 1 : 0;
    l3 = simplex[c][3]>=1 ? 1 : 0;
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
    int gi0 = perm[ii+perm[jj+perm[kk+perm[ll]]]] % 32;
    int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1+perm[ll+l1]]]] % 32;
    int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2+perm[ll+l2]]]] % 32;
    int gi3 = perm[ii+i3+perm[jj+j3+perm[kk+k3+perm[ll+l3]]]] % 32;
    int gi4 = perm[ii+1+perm[jj+1+perm[kk+1+perm[ll+1]]]] % 32;

    // Calculate the contribution from the five corners
    f64 t0 = 0.6 - x0*x0 - y0*y0 - z0*z0 - w0*w0;
    if(t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad4[gi0], x0, y0, z0, w0);
    }

    f64 t1 = 0.6 - x1*x1 - y1*y1 - z1*z1 - w1*w1;
    if(t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad4[gi1], x1, y1, z1, w1);
    }

    f64 t2 = 0.6 - x2*x2 - y2*y2 - z2*z2 - w2*w2;
    if(t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad4[gi2], x2, y2, z2, w2);
    }

    f64 t3 = 0.6 - x3*x3 - y3*y3 - z3*z3 - w3*w3;
    if(t3<0) n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad4[gi3], x3, y3, z3, w3);
    }

    f64 t4 = 0.6 - x4*x4 - y4*y4 - z4*z4 - w4*w4;
    if(t4<0) n4 = 0.0;
    else {
        t4 *= t4;
        n4 = t4 * t4 * dot(grad4[gi4], x4, y4, z4, w4);
    }

    // Sum up and scale the result to cover the range [-1,1]
    return 27.0 * (n0 + n1 + n2 + n3 + n4);
}
