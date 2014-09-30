#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#define SOA_GAME
#include <mutex>

#include <SDL/SDL.h>

#include "WorldStructs.h"

void printVec(string desc, glm::vec3 vec);
void printVec(string desc, glm::dvec3 vec);

extern glm::mat4 GlobalModelMatrix;

extern Biome blankBiome;

using namespace std;

extern HGLRC mainHGLRC;
extern SDL_Window* mainWindow;

extern bool NoChunkFade;
extern bool isMouseIn;

//relative rotations for the face transitions
//0 = top, 1 = left, 2 = right, 3 = front, 4 = back, 5 = bottom
const int FaceTransitions[6][6]={ {0, 1, -1, 0, 2, 0}, //top
                                {-1, 0, 0, 0, 0, 1}, //left
                                {1, 0, 0, 0, 0, -1}, //right
                                {0, 0, 0, 0, 0, 0}, //front
                                {2, 0, 0, 0, 0, 2}, //back
                                {0, -1, 1, 0, 2, 0} }; //bottom

const int FaceNeighbors[6][4]={ {2, 4, 1, 3}, //top
                                {3, 0, 4, 5}, //left
                                {4, 0, 3, 5}, //right
                                {2, 0, 1, 5}, //front
                                {1, 0, 2, 5}, //back
                                {2, 3, 1, 4} }; //bottom

// 6 faces, 4 rotations, i j r
//could just do rotation % 2 but repeating works fine
const int FaceCoords[6][4][3] = { { {2,0,1}, {0,2,1}, {2,0,1}, {0,2,1} },  //top
                                    { {1,2,0}, {2,1,0}, {1,2,0}, {2,1,0} }, //left
                                    { {1,2,0}, {2,1,0}, {1,2,0}, {2,1,0} }, //right
                                    { {1,0,2}, {0,1,2}, {1,0,2}, {0,1,2} }, //front
                                    { {1,0,2}, {0,1,2}, {1,0,2}, {0,1,2} }, //back
                                    { {2,0,1}, {0,2,1}, {2,0,1}, {0,2,1} } }; //bottom

// 6 faces, 4 rotations, ioff joff
//determined by taking the base case for i and j direction, and then rotating it 3 times, recording the new i j directions
const int FaceOffsets[6][4][2] = { { {1, 1}, {1, -1}, {-1, -1}, {-1, 1} }, //top
                                    { {-1, 1}, {1, 1}, {1, -1}, {-1, -1} }, //left
                                    { {-1, -1}, {-1, 1}, {1, 1}, {1, -1} }, //right
                                    { {-1, 1}, {1, 1}, {1, -1}, {-1, -1} }, //front
                                    { {-1, -1}, {-1, 1}, {1, 1}, {1, -1} }, //back
                                    { {-1, 1}, {1, 1}, {1, -1}, {-1, -1} } };

const int FaceRadSign[6] = {1, -1, 1, 1, -1, -1};

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