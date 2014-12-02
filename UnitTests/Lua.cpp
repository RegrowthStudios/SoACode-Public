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

int printer(int i) {
    return i;
}

OOLUA_CFUNC(printer, lua_printer);

TEST(Use) {
    using namespace OOLUA;
    Script vm;
    set_global(vm, "printer", lua_printer);
    vm.load_chunk(R"(
local a = 0
for i = 1, 100 do
    a = a + printer(i)
end
print(a)
)");
    lua_State* ls = vm;
    for (int i = 0; i < 100;i++) {
        lua_call(vm, 0, LUA_MULTRET);
    }

    return true;
}
