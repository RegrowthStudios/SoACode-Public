#include "stdafx.h"
#include "global.h"

#include <SDL/SDL.h>

void printVec(nString desc, f32v3 vec)
{
    std::cout << desc << " " << vec.x << " " << vec.y << " " << vec.z << std::endl;
}
void printVec(nString desc, f32v4 vec)
{
    std::cout << desc << " " << vec.x << " " << vec.y << " " << vec.z << " " << vec.w << std::endl;
}
void printVec(nString desc, f64v3 vec)
{
    std::cout << desc << " " << vec.x << " " << vec.y << " " << vec.z << std::endl;
}

GLushort boxDrawIndices[6] = {0,1,2,2,3,0};
GLfloat boxUVs[8] = {0, 1, 0, 0, 1, 0, 1, 1};

HGLRC mainHGLRC;
SDL_Window* mainWindow = NULL;

std::mutex mainContextLock;
SDL_GLContext mainOpenGLContext;

float maxPhysicsFps = 62.5f;
bool NoChunkFade;
bool isMouseIn = 0;
bool debugVarc = 0;
bool debugVarh = 0;
GLuint debugTicks;
bool debugTicksDone;
float bdt = 0.0f;
float planetDrawMode = 0.0f;

volatile float physicsFps = 0.0f;
float glFps = 0.0f;

double iDistort = 1.0;
extern double jDistort = 1.0;

float sonarDt = 0;
const int numSettings = 9;
bool MouseButtons[10];
bool isWaterUpdating = 1;
bool isFancyTrees = 1;
float physSpeedFactor = 1.0f;
float glSpeedFactor = 1.0f;

int globalTreeSeed = 0;

bool chunkMeshShortcutActive = 1;

bool sonarActive = 0;
bool clickDragActive = 0;

int csGridWidth;
int drawMode = 0;

double closestTerrainPatchDistance = 100000000.0f;
GLuint EditorState;

bool globalDebug1 = 1, globalDebug2 = 1;
GLuint GuiTextureID;