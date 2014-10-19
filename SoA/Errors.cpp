#include "stdafx.h"
#include "Errors.h"

#include <cstdlib>

#include <SDL/SDL.h>

#include "OpenglManager.h"

void showMessage(const string& message)
{
#if defined(_WIN32) || defined(_WIN64)
    //SDL_WM_IconifyWindow();
    SDL_Delay(100);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    MessageBox(NULL, message.c_str(), "SoA", MB_OK);
#elif
    cout << "ERROR! MESSAGE BOX NOT IMPLEMENTED FOR THIS FILE SYSTEM\n";
    int a;
    cin >> a;
#endif
}

//yes 1, no 0
int showYesNoBox(const string& message)
{
#if defined(_WIN32) || defined(_WIN64)
    SDL_Delay(100);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    int id = MessageBox(NULL, message.c_str(), "SoA", MB_YESNO);
    if (id == IDYES) return 1;
    if (id == IDNO) return 0;
    return 0;
#elif
    cout << "ERROR! YESNO BOX NOT IMPLEMENTED FOR THIS FILE SYSTEM\n";
    int a;
    cin >> a;
#endif
}

///yes 1, no 0, cancel -1
int showYesNoCancelBox(const string& message)
{
#if defined(_WIN32) || defined(_WIN64)
    SDL_Delay(100);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    int id = MessageBox(NULL, message.c_str(), "SoA", MB_YESNOCANCEL);
    if (id == IDYES) return 1;
    if (id == IDNO) return 0;
    if (id == IDCANCEL) return -1;
    return 0;
#elif
    cout << "ERROR! YESNO BOX NOT IMPLEMENTED FOR THIS FILE SYSTEM\n";
    int a;
    cin >> a;
#endif
}

string getFullPath(const char *initialDir)
{
    string rval;
    char pathBuffer[1024];
    _fullpath(pathBuffer, initialDir, 1024);
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
    showMessage("ERROR: " + string(message));
}

void pError(const string& message)
{
    FILE *logFile = NULL;
    SDL_SetRelativeMouseMode(SDL_FALSE);
    logFile = fopen("errorlog.txt", "a+");
    if (logFile != NULL){
        fprintf(logFile, "*ERROR: %s \n", message.c_str());
        fclose(logFile);
    }
    printf("*ERROR: %s \n", message);
    fflush(stdout);
    showMessage("ERROR: " + message);
}

//Checks the output of glGetError and prints an appropriate error message if needed.
bool checkGlError(const string& errorLocation) {
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
        default:
            pError("At " + errorLocation + ". Error code UNKNOWN");
            break;
        }
        return true;
    }
    return false;
}