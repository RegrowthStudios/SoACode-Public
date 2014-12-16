#include "stdafx.h"
#include "macros.h"

#undef UNIT_TEST_BATCH
#define UNIT_TEST_BATCH Example_

TEST(MyTestGood) {
    std::map<int, int> m;
    m[1] = 1;
    return true;
}

TEST(MyTestBad) {
    int* m = nullptr;
    //*m = 0;
    return true;
}