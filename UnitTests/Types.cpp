#include "stdafx.h"
#include "macros.h"

#undef UNIT_TEST_BATCH
#define UNIT_TEST_BATCH System_

TEST(TypeSizes) {
#define SA(T, S) assert(sizeof(T) == S)
    SA(float, 4);
    SA(double, 8);
    SA(f32, 4);
    SA(f64, 8);
    SA(i32, 4);
    SA(i64, 8);
    SA(i8v3, 3);
    SA(i16v3, 6);
    SA(i32v3, 12);
    SA(i64v3, 24);
    SA(f32v3, 12);
    SA(f64v3, 24);
    return true;
}