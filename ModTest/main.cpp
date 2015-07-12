#include <cstdio>

#define _WINSOCKAPI_
#include <Windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    // Perform actions based on the reason for calling.
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
        puts("Mod Attach");
        fprintf(stderr, "DLL");
        break;
    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        puts("Mod Thread Attach");
        break;
    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        puts("Mod Thread Detach");
        break;
    case DLL_PROCESS_DETACH:
        // Perform any necessary cleanup.
        puts("Mod Detach");
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) int getCode() {
    return 42;
}
