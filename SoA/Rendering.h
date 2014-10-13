#pragma once
#include "stdafx.h"

#include "Constants.h"

// TODO: Remove This
using namespace std;

extern int renderMode;

enum renderModes {RENDER_2D, RENDER_3D};

extern int sunColor[64][3];

// cube //
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3


const float starboxSize = 15000000.0f;
const GLfloat starboxVertices[72] = { -starboxSize, starboxSize, starboxSize, -starboxSize, -starboxSize, starboxSize, starboxSize, -starboxSize, starboxSize, starboxSize, starboxSize, starboxSize,  // v1-v2-v3-v0 (front)

                        starboxSize, starboxSize, starboxSize,   starboxSize, -starboxSize, starboxSize,   starboxSize, -starboxSize, -starboxSize,  starboxSize, starboxSize, -starboxSize,     // v0-v3-v4-v5 (right)

                        -starboxSize, starboxSize, -starboxSize,   -starboxSize, starboxSize, starboxSize,   starboxSize, starboxSize, starboxSize,  starboxSize, starboxSize, -starboxSize,    // v6-v1-v0-v5 (top)

                        -starboxSize, starboxSize, -starboxSize,   -starboxSize, -starboxSize, -starboxSize,  -starboxSize, -starboxSize, starboxSize,   -starboxSize, starboxSize, starboxSize,   // v6-v7-v2-v1 (left)

                        -starboxSize, -starboxSize, -starboxSize,   starboxSize, -starboxSize, -starboxSize,   starboxSize, -starboxSize, starboxSize,  -starboxSize, -starboxSize, starboxSize,    // v7-v4-v3-v2 (bottom)

                        starboxSize, starboxSize, -starboxSize,   starboxSize, -starboxSize, -starboxSize,   -starboxSize, -starboxSize, -starboxSize,  -starboxSize, starboxSize, -starboxSize};     // v5-v4-v7-v6 (back)

// cube //
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3

extern GLushort starboxIndices[6][6];

const GLfloat starboxUVs[48] = {   1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,  // v1-v2-v3-v0 (front)
                                    1.0, 1.0,  1.0, 0.0,  0.0, 0.0, 0.0, 1.0, // v0-v3-v4-v5 (right)
                                    1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,// v6-v1-v0-v5 (top)
                                    1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,// v6-v7-v2-v1 (left)
                                    1.0, 0.0, 0.0, 0.0,   0.0, 1.0, 1.0, 1.0,  // v7-v4-v3-v2 (bottom)
                                    1.0, 1.0,  1.0, 0.0,   0.0, 0.0,  0.0, 1.0 }; // v5-v4-v7-v6 (back)

extern GLfloat colorVertices[1024];
extern GLfloat cubeSpriteVerts[24];
extern GLfloat cubeSpriteUVs[24];
extern GLfloat cubeSpriteColorVertices[48];

extern GLfloat flatSpriteVertices[8];
extern GLushort cubeSpriteDrawIndices[18];

//constants analogous to CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_SIZE for padded chunks.
const int PADDED_WIDTH = CHUNK_WIDTH+2;
const int PADDED_LAYER = PADDED_WIDTH*PADDED_WIDTH;
const int PADDED_SIZE = PADDED_WIDTH*PADDED_LAYER;

//Helper constants for finding neighbor block indices
namespace PADDED_OFFSETS {
    const int LEFT = -1;
    const int RIGHT = 1;
    const int BACK = -PADDED_WIDTH;
    const int FRONT = PADDED_WIDTH;
    const int BOTTOM = -PADDED_LAYER;
    const int TOP = PADDED_LAYER;

    const int BACK_LEFT = BACK + LEFT;
    const int BACK_RIGHT = BACK + RIGHT;
    const int FRONT_RIGHT = FRONT + RIGHT;
    const int FRONT_LEFT = FRONT + LEFT;

    const int BOTTOM_LEFT = BOTTOM + LEFT;
    const int BOTTOM_BACK_LEFT = BOTTOM + BACK_LEFT;
    const int BOTTOM_BACK = BOTTOM + BACK;
    const int BOTTOM_BACK_RIGHT = BOTTOM + BACK_RIGHT;
    const int BOTTOM_RIGHT = BOTTOM + RIGHT;
    const int BOTTOM_FRONT_RIGHT = BOTTOM + FRONT_RIGHT;
    const int BOTTOM_FRONT = BOTTOM + FRONT;
    const int BOTTOM_FRONT_LEFT = BOTTOM + FRONT_LEFT;

    const int TOP_LEFT = TOP + LEFT;
    const int TOP_BACK_LEFT = TOP + BACK_LEFT;
    const int TOP_BACK = TOP + BACK;
    const int TOP_BACK_RIGHT = TOP + BACK_RIGHT;
    const int TOP_RIGHT = TOP + RIGHT;
    const int TOP_FRONT_RIGHT = TOP + FRONT_RIGHT;
    const int TOP_FRONT = TOP + FRONT;
    const int TOP_FRONT_LEFT = TOP + FRONT_LEFT;
}

const int BILLBOARD_VERTS_SIZE = 200000;
const int TREE_VERTS_SIZE = 200000;
extern struct BillboardVertex billVerts[BILLBOARD_VERTS_SIZE];
extern struct TreeVertex treeVerts[TREE_VERTS_SIZE];

const bool sepVBO = 1;

class WorldRenderer
{
public:
    WorldRenderer();
    ~WorldRenderer();
    void Initialize();
    void DrawLine(glm::vec3 a, glm::vec3 b);
    void DrawUnderwater();
    void ChangeWaterTexture();

    GLuint UnderwaterTexture;
private:
    void HeightToNormal();
    //inline void CopyVertexData(Vertex &v1, Vertex &v2);
};

extern WorldRenderer worldRenderer;

void DrawCubeSpriteImage2D(GLfloat *vertices, int sizeOfvertices, GLfloat *uvs, int sizeOfuvs, GLushort *indices,int sizeOfindices, GLuint textureID, glm::vec4 color);

void DrawImage2D(float x, float y, float width, float height, GLuint textureID, float xdim = 800.0f, float ydim = 600.0f, glm::vec4 color = glm::vec4(1.0), int oreintation = 1);

void DrawFullScreenQuad(glm::vec4 color);

void DrawCubeSprite(GLfloat x, GLfloat y, int blockType, float scale);

void DrawRoundSprite(GLfloat x, GLfloat y, int blockType, float scale);

void DrawSun(float theta, glm::mat4 &MVP);

void DrawStars(float theta, glm::mat4 &MVP);

void DrawWireBox(double x, double y, double z, double xw, double yh, double zw, float lineWidth, const glm::dvec3 &playerPos, glm::mat4 &VP, glm::vec4 color);

void DrawLoadingScreen(string text, bool clearColor = 1, glm::vec4 backColor = glm::vec4(0.0, 0.0, 0.0, 1.0), int fontSize = 42);

void Draw3DCube(class Block *block, double x, double y, double z, glm::mat4 &VP, glm::mat4 &rotation);
