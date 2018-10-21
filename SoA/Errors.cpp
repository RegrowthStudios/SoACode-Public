#include "stdafx.h"
#include "Errors.h"

#include <SDL2/SDL.h>

#ifndef _WINDOWS
#include <limits.h>
#include <stdlib.h>
#endif//_WINDOWS

void showMessage(const nString& message VORB_MAYBE_UNUSED)
{
#if defined(_WIN32) || defined(_WIN64)
    //SDL_WM_IconifyWindow();
    SDL_Delay(100);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    MessageBox(NULL, message.c_str(), "SoA", MB_OK);
#else
    std::cout << "ERROR! MESSAGE BOX NOT IMPLEMENTED FOR THIS FILE SYSTEM\n";
    int a;
    std::cin >> a;
#endif
}

//yes 1, no 0
int showYesNoBox(const nString& message VORB_MAYBE_UNUSED)
{
#if defined(_WIN32) || defined(_WIN64)
    SDL_Delay(100);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    int id = MessageBox(NULL, message.c_str(), "SoA", MB_YESNO);
    if (id == IDYES) return 1;
    if (id == IDNO) return 0;
    return 0;
#else
    std::cout << "ERROR! YESNO BOX NOT IMPLEMENTED FOR THIS FILE SYSTEM\n";
    int a;
    std::cin >> a;
    return 0;
#endif
}

///yes 1, no 0, cancel -1
int showYesNoCancelBox(const nString& message VORB_MAYBE_UNUSED)
{
#if defined(_WIN32) || defined(_WIN64)
    SDL_Delay(100);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    int id = MessageBox(NULL, message.c_str(), "SoA", MB_YESNOCANCEL);
    if (id == IDYES) return 1;
    if (id == IDNO) return 0;
    if (id == IDCANCEL) return -1;
    return 0;
#else
    std::cout << "ERROR! YESNO BOX NOT IMPLEMENTED FOR THIS FILE SYSTEM\n";
    int a;
    std::cin >> a;
    return 0;
#endif
}

nString getFullPath(const char *initialDir)
{
    nString rval;
#ifdef _WINDOWS
    char pathBuffer[1024];
    _fullpath(pathBuffer, initialDir, 1024);
#else//_WINDOWS
    char pathBuffer[PATH_MAX];
    realpath(initialDir, pathBuffer);
#endif//_WINDOWS
    rval = pathBuffer;
    return rval;
}

void pError(const char *message)
{
    FILE *logFile = NULL;
    SDL_SetRelativeMouseMode(SDL_FALSE);
    logFile = fopen("errorlog.txt", "a+");
    if (logFile != NULL){
        fprintf(logFile, "*ERROR: %s \n", message);
        fclose(logFile);
    }
    printf("*ERROR: %s \n", message);
    fflush(stdout);
    showMessage("ERROR: " + nString(message));
}

void pError(const nString& message)
{
    FILE *logFile = NULL;
    SDL_SetRelativeMouseMode(SDL_FALSE);
    logFile = fopen("errorlog.txt", "a+");
    if (logFile != NULL){
        fprintf(logFile, "*ERROR: %s \n", message.c_str());
        fclose(logFile);
    }
    printf("*ERROR: %s \n", message.c_str());
    fflush(stdout);
    showMessage("ERROR: " + message);
}

//Checks the output of glGetError and prints an appropriate error message if needed.
bool checkGlError(const nString& errorLocation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        switch (error) {
        case GL_INVALID_ENUM:
            pError("At " + errorLocation + ". Error code 1280: GL_INVALID_ENUM");
            break;
        case GL_INVALID_VALUE:
            pError("At " + errorLocation + ". Error code 1281: GL_INVALID_VALUE");
            break;
        case GL_INVALID_OPERATION:
            pError("At " + errorLocation + ". Error code 1282: GL_INVALID_OPERATION");
            break;
        case GL_STACK_OVERFLOW:
            pError("At " + errorLocation + ". Error code 1283: GL_STACK_OVERFLOW");
            break;
        case GL_STACK_UNDERFLOW:
            pError("At " + errorLocation + ". Error code 1284: GL_STACK_UNDERFLOW");
            break;
        case GL_OUT_OF_MEMORY:
            pError("At " + errorLocation + ". Error code 1285: GL_OUT_OF_MEMORY");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            pError("At " + errorLocation + ". Error code 1285: GL_INVALID_FRAMEBUFFER_OPERATION");
            break;
        default:
            pError("At " + errorLocation + ". Error code " + std::to_string(error) + ": UNKNOWN");
            break;
        }
        return true;
    }
    return false;
}
