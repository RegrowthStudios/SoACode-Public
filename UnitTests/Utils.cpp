#include "stdafx.h"
#include "macros.h"

#undef UNIT_TEST_BATCH
#define UNIT_TEST_BATCH Utils_

#include <random>

#include <Vorb/Random.h>
#include <Vorb/Timing.h>

TEST(MersenneTwister) {
    f32 t1, t2;
    f32 v1 = 0.0f, v2 = 0.0f;

    ::Random zr(20);
    std::mt19937 mt(20);

    PreciseTimer timer;

    timer.start();
    for (size_t i = 0; i < 10000000; i++) {
        v1 += mt() / (f32)0xffffffffu;
    }
    t1 = timer.stop();

    timer.start();
    for (size_t i = 0; i < 10000000; i++) {
        v2 += zr.genMT();
    }
    t2 = timer.stop();

    std::cout << t1 << " : " << v1 / 10000000.0f << std::endl;
    std::cout << t2 << " : " << v2 / 10000000.0f << std::endl;

    return true;
}