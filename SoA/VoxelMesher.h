#pragma once
#include "OpenGLStructs.h"
#include "BlockData.h"

const int NUM_FLORA_MESHES = 4;
const int NUM_CROSSFLORA_MESHES = 2;

//Provides helpful meshing functions for voxels
class VoxelMesher
{
public:
    static void makeFloraFace(BlockVertex *Verts, const ui8* positions, const i8* normals, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const GLubyte color[], const GLubyte overlayColor[], const ui8 sunlight, const ui8 lampColor[3], const BlockTexture& texInfo);
    static void makeCubeFace(BlockVertex *Verts, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const GLubyte color[], const GLubyte overlayColor[], GLfloat ambientOcclusion[], const BlockTexture& texInfo);
    static void setFaceLight(BlockVertex* Verts, int index, ui8 lampColor[3], ui8 sunlight);
    static void makeLiquidFace(std::vector<LiquidVertex>& verts, i32 index, ui8 uOff, ui8 vOff, ui8 light[2], ui8 color[3], ui8 textureUnit);
    static void makePhysicsBlockFace(vector <PhysicsBlockVertex> &verts, const GLfloat *blockPositions, int vertexOffset, int &index, const BlockTexture& blockTexture);

    static const GLfloat leafVertices[72];
    static const GLubyte cubeVertices[72];
    static const int cubeFaceAxis[6][2];
    static const int cubeFaceAxisSign[6][2];
    static const GLfloat liquidVertices[72];
    static const GLfloat waterCubeVertices[72];
    static const GLbyte cubeNormals[72];
    static const GLbyte floraNormals[72];
    static const ui8 floraVertices[NUM_FLORA_MESHES][36];
    static const ui8 crossFloraVertices[NUM_CROSSFLORA_MESHES][24];
    static const GLfloat physicsBlockVertices[72];
};

