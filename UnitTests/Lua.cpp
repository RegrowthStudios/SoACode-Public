#include "stdafx.h"
#include "macros.h"

#include <lua/oolua.h>

#undef UNIT_TEST_BATCH
#define UNIT_TEST_BATCH Lua_

TEST(Create) {
    using namespace OOLUA;
    Script vm;
    vm.run_chunk("print(\"Hello World\")");
    return true;
}

void printer() {
    puts("C++ Hello World");
}

OOLUA_CFUNC(printer, lua_printer);

TEST(Use) {
    using namespace OOLUA;
    Script vm;
    set_global(vm, "printer", lua_printer);
    vm.run_chunk("printer()");
    return true;
}
