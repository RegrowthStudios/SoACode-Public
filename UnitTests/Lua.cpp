#include "stdafx.h"
#include "macros.h"

#include <lua/oolua.h>

#undef UNIT_TEST_BATCH
#define UNIT_TEST_BATCH Lua_

TEST(Create) {
    using namespace OOLUA;
    Script s;
    s.run_chunk("print(\"Hello World\")");
    return true;
}