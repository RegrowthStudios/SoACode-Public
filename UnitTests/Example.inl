#include <map>

TEST(MyTestGood) {
    puts("Use a map");
    std::map<int, int> m;
    m[1] = 1;
    return true;
}

TEST(MyTestBad) {
    puts("Bad Pointer");
    int* m = nullptr;
    *m = 0;
    return true;
}