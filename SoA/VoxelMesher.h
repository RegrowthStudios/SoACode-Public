#pragma once
#include "OpenGLStructs.h"
#include "BlockData.h"

#define NUM_FLORA_MESHES 4
#define NUM_FLORA_VERTICES 36

#define NUM_CROSSFLORA_MESHES 2
#define NUM_CROSSFLORA_VERTICES 24

// Offsets into CubeVertices
#define CUBE_FACE_0_VERTEX_OFFSET 0
#define CUBE_FACE_1_VERTEX_OFFSET 12
#define CUBE_FACE_2_VERTEX_OFFSET 24
#define CUBE_FACE_3_VERTEX_OFFSET 36
#define CUBE_FACE_4_VERTEX_OFFSET 48
#define CUBE_FACE_5_VERTEX_OFFSET 60

//Provides helpful meshing functions for voxels
class VoxelMesher
{
public:
    static void makeFloraFace(BlockVertex *Verts, const ui8* positions, const i8* normals, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const ColorRGB8& color, const ColorRGB8& overlayColor, const ui8 sunlight, const ColorRGB8& lampColor, const BlockTexture& texInfo);
    static void makeTransparentFace(BlockVertex *Verts, const ui8* positions, const i8* normals, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const ColorRGB8& color, const ColorRGB8& overlayColor, const ui8 sunlight, const ColorRGB8& lampColor, const BlockTexture& texInfo);
    static void makeCubeFace(BlockVertex *Verts, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const ColorRGB8& color, const ColorRGB8& overlayColor, GLfloat ambientOcclusion[], const BlockTexture& texInfo);
    static void setFaceLight(BlockVertex* Verts, int index, const ColorRGB8& lampColor, ui8 sunlight);
    static void makeLiquidFace(std::vector<LiquidVertex>& verts, i32 index, ui8 uOff, ui8 vOff, const ColorRGB8& lampColor, ui8 sunlight, const ColorRGB8& color, ui8 textureUnit);
    static void makePhysicsBlockFace(vector <PhysicsBlockVertex> &verts, int vertexOffset, int &index, const BlockTexture& blockTexture);
    static ui8 getBlendMode(const BlendType& blendType);
#define POSITION_RESOLUTION 7
#define ATLAS_SIZE 256

#define NUM_FACES 6
#define NUM_VERTICES 72

    static const GLfloat leafVertices[NUM_VERTICES];
    static const GLubyte cubeVertices[NUM_VERTICES];
    static const int cubeFaceAxis[NUM_FACES][2];
    static const int cubeFaceAxisSign[NUM_FACES][2];
    static const GLfloat liquidVertices[NUM_VERTICES];
    static const GLfloat waterCubeVertices[NUM_VERTICES];
    static const GLbyte cubeNormals[NUM_VERTICES];
    static const GLbyte floraNormals[NUM_VERTICES];
    static const ui8 floraVertices[NUM_FLORA_MESHES][NUM_FLORA_VERTICES];
    static const ui8 crossFloraVertices[NUM_CROSSFLORA_MESHES][NUM_CROSSFLORA_VERTICES];
};

