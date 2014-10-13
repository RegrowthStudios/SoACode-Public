#pragma once
#include "stdafx.h"
#include <mutex>

#include <SDL/SDL.h>

#include "WorldStructs.h"

void printVec(string desc, glm::vec3 vec);
void printVec(string desc, glm::dvec3 vec);

extern glm::mat4 GlobalModelMatrix;

extern Biome blankBiome;

using namespace std;

#if defined(WIN32) || defined(WIN64)
extern HGLRC mainHGLRC;
#endif
extern SDL_Window* mainWindow;

extern bool NoChunkFade;
extern bool isMouseIn;

extern bool debugVarc;
extern bool debugVarh;
extern GLuint debugTicks;
extern bool debugTicksDone;
extern double closestTerrainPatchDistance;
extern int gridState;

extern bool chunkMeshShortcutActive;
extern int globalTreeSeed;
extern float planetDrawMode;

extern GLuint EditorState;
extern volatile float physicsFps;
extern float glFps;
enum EditorGameStates{ E_MAIN, E_AEXIT, E_TREE_EDITOR, E_BLOCK_EDITOR, E_SELECT_TEXTURE, E_BIOME_EDITOR, E_CLIMATE_EDITOR , E_TERRAIN_EDITOR};

const float planetScale = 1.0f;
const float invPlanetScale = 1.0f/planetScale;
const float GRAVITY = 0.0065f;
const int MAXLEAFRADIUS = 12;

extern int csGridWidth;

extern float bdt;

extern int drawMode;
extern float sonarDt;

static GLfloat wholeScreenVertices[] = { 0, 1, 0, 0, 1, 0, 1, 1 };
static GLfloat cubeSpriteVertices[] = {0, 50, 0, 10, 30, 0, 30, 40,
                                        30, 40, 30, 0, 60, 10, 60, 50,
                                        30, 60, 0, 50, 30, 40, 60, 50};


extern bool globalDebug1, globalDebug2;
extern bool clickDragActive;

extern GLuint GuiTextureID;

//debug variables, counts the triangles generated when initial world is created

//temporary
extern int lodLength;
const int TerrainPatchWidth = 262144*8/planetScale;

extern GLushort boxDrawIndices[6];
extern GLfloat boxUVs[8];

//config settings
extern string config_NoiseFileName;

extern float physSpeedFactor;
extern float glSpeedFactor;
extern float maxPhysicsFps;
extern bool isWaterUpdating;
extern bool isFancyTrees;

extern mutex mainContextLock;
extern void * mainOpenGLContext;

extern bool MouseButtons[10];
extern bool sonarActive;

const int maxParticles = 100000;
const int BPARTICLES = 4;

const float LIGHT_MULT = 0.95f, LIGHT_OFFSET = -0.19f;