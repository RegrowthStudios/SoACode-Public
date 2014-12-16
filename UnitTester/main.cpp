#include <stdafx.h>
#include <Tests.h>

namespace UnitTests {
    class __declspec(dllimport) Tests;
}

int main(int argc, char** argv) {
    using namespace UnitTests;
    for (auto it = Tests::begin(); it != Tests::end(); it++) {
        try {
            puts("===");
            printf("Running %s\n", it->first.c_str());
            Tests::runTest(it->first);
            puts("===");
        } catch (...) {
            puts("Exception(al) Failure");
        }
    }
    std::cin.clear();
    std::cout << std::cin.get();
    return 0;
}