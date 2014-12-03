#include "stdafx.h"
#include "Random.h"

#include <time.h>

// Constructors
Random::Random(i32 s) {
    _mtArr = new i32[MERSENNE_ARRAY_SIZE];
    seed(s);
}
Random::Random() : Random(clock()) {
    // Just Use The Regular Constructor
}
Random::~Random() {
    if (_mtArr) {
        delete[] _mtArr;
        _mtArr = nullptr;
    }
}

// Re-seed The Generator
#define MERSENNE_SEED_PRIME 1812433253
void Random::seed(i32 s) {
    _seed = s;

    _mtIndex = 0;
    *_mtArr = _seed;
    register i32 prev = _seed;
    for (int i = 1; i < MERSENNE_ARRAY_SIZE; i++) {
        prev = MERSENNE_SEED_PRIME * (prev ^ ((prev >> 30))) + i;
        _mtArr[i] = prev;
    }

    _mhKey = _seed;
}

// Obtain Random Numbers
#define MERSENNE_GEN_PRIME1 2636928640
#define MERSENNE_GEN_PRIME2 4022730752
f32 Random::genMT() {
    if(_mtIndex == 0) perturbMT();

    i32 y = _mtArr[_mtIndex];
    y ^= y >> 11;
    y ^= (y << 7) & MERSENNE_GEN_PRIME1;
    y ^= (y << 15) & MERSENNE_GEN_PRIME2;
    y ^= y >> 18;

    _mtIndex = (_mtIndex + 1) % MERSENNE_ARRAY_SIZE;
    return y / (f32)(0x7fffffff);
}
#define MURMUR_GEN_PRIME1 0xcc9e2d51
#define MURMUR_GEN_PRIME2 0x1b873593
#define MURMUR_GEN_PRIME3 0xe6546b64
#define MURMUR_GEN_PRIME4 0x85ebca6b
#define MURMUR_GEN_PRIME5 0xc2b2ae35
f32 Random::genMH() {
        ui32 hash = _seed;
        ui32 k = _mhKey;
        k *= MURMUR_GEN_PRIME1;
        k = (k << 15) | (k >> (32 - 15));
        k *= MURMUR_GEN_PRIME2;

        hash ^= k;
        hash = ((hash << 13) | (hash >> (32 - 13))) * 5 + MURMUR_GEN_PRIME3;
        ui32 k1 = 0;
        hash ^= 4;
        hash ^= (hash >> 16);
        hash *= MURMUR_GEN_PRIME4;
        hash ^= (hash >> 13);
        hash *= MURMUR_GEN_PRIME5;
        hash ^= (hash >> 16);
        _mhKey = hash;
        return hash / (f32)(0xffffffff);
}

// Perturbs Mersenne Twister Array After A Full Traversal
#define MERSENNE_PERT_PRIME1 397
#define MERSENNE_PERT_PRIME2 2567483615
void Random::perturbMT() {
    for(i32 i = 0; i < MERSENNE_ARRAY_SIZE; i++) {
        i32 y = (_mtArr[i] & 0x80000000) + (_mtArr[(i + 1) % MERSENNE_ARRAY_SIZE] & 0x7fffffff);
        _mtArr[i] = _mtArr[(i + MERSENNE_PERT_PRIME1) % MERSENNE_ARRAY_SIZE] ^ (y >> 1);
        if(y & 1) _mtArr[i] ^= MERSENNE_PERT_PRIME2;
    }
}