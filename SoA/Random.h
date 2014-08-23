#pragma once

// Like Goldilocks, Just Right
#define MERSENNE_ARRAY_SIZE 624

// A Seeded Random Number Generator
class Random {
public:
    // Create A Random Generator Using A Seed Value
    Random(i32 s);
    // Create A Random Generator With Seed Value Of Clock Ticks
    Random();
    ~Random();

    // Reform The Random Generator
    void seed(i32 s);

    // Obtain A Random Number Between [0-1] Inclusive
    f32 genMT(); // Mersenne Twister
    f32 genMH(); // Murmur Hash

private:
    // ... Because We Want To Keep Generating Random Stuff After MERSENNE_ARRAY_SIZE Uses
    void perturbMT();

    // The Seed Value Of The Generator
    i32 _seed;

    // An Array Of Randomly Generated Numbers For The Twister To Rampage Upon
    i32* _mtArr;
    i32 _mtIndex;

    // An Updating Key For Murmur Hash
    i32 _mhKey;
};