#include "stdafx.h"
#include "macros.h"

#include <lua/oolua.h>
extern "C" {
    #include <lua\lua\lua.h>
}

#undef UNIT_TEST_BATCH
#define UNIT_TEST_BATCH Lua_

TEST(Create) {
    using namespace OOLUA;
    Script vm;
    vm.run_chunk("print(\"Hello World\")");
    return true;
}

int printer(int i);
OOLUA_CFUNC(printer, lua_printer);
int printer(int i) {
    static int r = 0;
    r += i;
    return r;
}


TEST(Use) {
    using namespace OOLUA;
    Script vm;
    set_global(vm, "printer", lua_printer);

    vm.run_chunk(R"(
function test()
    for i = 1,100000 do
        printer(1)
    end
end
)");
    vm.call("test");
    printf("VALUE: %d\n", printer(0));

    return true;
}
