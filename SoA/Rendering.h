#pragma once
#include "Constants.h"
#include <Vorb/VorbPreDecl.inl>

DECL_VG(class, GLProgram);
DECL_VG(class, GLProgramManager);

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



extern GLfloat colorVertices[1024];
extern GLfloat cubeSpriteVerts[24];
extern GLfloat cubeSpriteUVs[24];
extern GLfloat cubeSpriteColorVertices[48];

extern GLfloat flatSpriteVertices[8];

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
extern class BillboardVertex billVerts[BILLBOARD_VERTS_SIZE];
extern class TreeVertex treeVerts[TREE_VERTS_SIZE];

const bool sepVBO = 1;

class WorldRenderer
{
public:
    WorldRenderer();
    ~WorldRenderer();
    void DrawLine(glm::vec3 a, glm::vec3 b);
    void DrawUnderwater();
    void ChangeWaterTexture();

    GLuint UnderwaterTexture;
private:
    void HeightToNormal();
    //inline void CopyVertexData(Vertex &v1, Vertex &v2);
};

extern WorldRenderer worldRenderer;

void DrawWireBox(vg::GLProgram* program, double x, double y, double z, double xw, double yh, double zw, float lineWidth, const f64v3 &playerPos, const f32m4 &VP, const f32v4& color);

void Draw3DCube(vg::GLProgramManager* glProgramManager, class Block *block, double x, double y, double z, glm::mat4 &VP, glm::mat4 &rotation);
