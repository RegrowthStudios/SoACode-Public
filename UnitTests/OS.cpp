#include "stdafx.h"
#include "macros.h"

#undef UNIT_TEST_BATCH
#define UNIT_TEST_BATCH OS_

#define OS_WINDOWS
#include <MemFile.h>

TEST(ConsoleRedirect) {
    MemFile m;
    FILE old = *stdout;
    *stdout = *m;

    std::cout << "Str1" << std::endl;
    puts("Str2");

    *stdout = old;
    return true;
}
