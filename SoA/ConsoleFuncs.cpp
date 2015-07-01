#include "stdafx.h"
#include "ConsoleFuncs.h"

#include <Vorb/script/Environment.h>

#include "DLLAPI.h"

void registerFuncs(vscript::Environment& env) {
    env.addCRDelegate("loadDLL", makeRDelegate(loadDLL));
}

int loadDLL(const cString name) {
    HINSTANCE dll = LoadLibrary(name);
    int(*f)() = (int(*)())GetProcAddress(dll, "getCode");
    fflush(stderr);
    fflush(stdout);
    return f();
}
