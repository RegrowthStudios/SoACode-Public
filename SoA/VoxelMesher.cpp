#include "stdafx.h"
#include "VoxelMesher.h"

// cube //
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3 

const GLfloat VoxelMesher::leafVertices[72] = { -0.0f, 1.0f, 0.5f, -0.0f, -0.0f, 0.5f, 1.0f, -0.0f, 0.5f, 1.0f, 1.0f, 0.5f,  // v1-v2-v3-v0 (front) //WRONG!!!

    0.5f, 1.0f, 1.0f, 0.5f, -0.0f, 1.0f, 0.5f, -0.0f, -0.0f, 0.5f, 1.0f, -0.0f,     // v0-v3-v4-v5 (right) //WRONG!!!

    -0.0f, 0.5f, -0.0f, -0.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, -0.0f,    // v6-v1-v0-v5 (top) //WRONG!!!

    0.5f, 1.0f, -0.0f, 0.5f, -0.0f, -0.0f, 0.5f, -0.0f, 1.0f, 0.5f, 1.0f, 1.0f,   // v6-v7-v2-v1 (left) //WRONG!!!

    -0.0f, 0.5f, -0.0f, 1.0f, 0.5f, -0.0f, 1.0f, 0.5f, 1.0f, -0.0f, 0.5f, 1.0f,    // v7-v4-v3-v2 (bottom) //WRONG!!!

    1.0f, 1.0f, 0.5f, 1.0f, -0.0f, 0.5f, -0.0f, -0.0f, 0.5f, -0.0f, 1.0f, 0.5f };     // v5-v4-v7-v6 (back) //WRONG!!!

// Cube Vertex Positional Resolution
#define C_RES 7

const ui8v3 VoxelMesher::VOXEL_POSITIONS[NUM_FACES][4] = {
    { ui8v3(0, C_RES, 0), ui8v3(0, 0, 0), ui8v3(0, 0, C_RES), ui8v3(0, C_RES, C_RES) }, // v6-v7-v2-v1 (left)
    { ui8v3(C_RES, C_RES, C_RES), ui8v3(C_RES, 0, C_RES), ui8v3(C_RES, 0, 0), ui8v3(C_RES, C_RES, 0) }, // v0-v3-v4-v5 (right)
    { ui8v3(0, 0, C_RES), ui8v3(0, 0, 0), ui8v3(C_RES, 0, 0), ui8v3(C_RES, 0, C_RES) }, // v2-v7-v4-v3 (bottom)
    { ui8v3(C_RES, C_RES, C_RES), ui8v3(C_RES, C_RES, 0), ui8v3(0, C_RES, 0), ui8v3(0, C_RES, C_RES) }, // v0-v5-v6-v1 (top)
    { ui8v3(C_RES, C_RES, 0), ui8v3(C_RES, 0, 0), ui8v3(0, 0, 0), ui8v3(0, C_RES, 0) }, // v5-v4-v7-v6 (back)
    { ui8v3(0, C_RES, C_RES), ui8v3(0, 0, C_RES), ui8v3(C_RES, 0, C_RES), ui8v3(C_RES, C_RES, C_RES) } // v1-v2-v3-v0 (front)
};

//0 = x, 1 = y, 2 = z
const int VoxelMesher::cubeFaceAxis[6][2] = { { 0, 1 }, { 2, 1 }, { 0, 2 }, { 2, 1 }, { 0, 2 }, { 0, 1 } }; // front, right, top, left, bottom, back, for U and V respectively

const int VoxelMesher::cubeFaceAxisSign[6][2] = { { 1, 1 }, { -1, 1 }, { 1, -1 }, { 1, 1 }, { -1, -1 }, { -1, 1 } }; // front, right, top, left, bottom, back, for U and V respectively

const GLfloat VoxelMesher::liquidVertices[72] = { 0, 1.0f, 1.0f, 0, 0, 1.0f, 1.0f, 0, 1.0f, 1.0f, 1.0f, 1.0f,  // v1-v2-v3-v0 (front)

    1.0f, 1.0f, 1.0f, 1.0f, 0, 1.0f, 1.0f, 0, 0, 1.0f, 1.0f, 0,     // v0-v3-v4-v5 (right)

    0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0,    // v6-v1-v0-v5 (top)

    0, 1.0f, 0, 0, 0, 0, 0, 0, 1.0f, 0, 1.0f, 1.0f,   // v6-v1.0f-v2-v1 (left)

    1.0f, 0, 0, 1.0f, 0, 1.0f, 0, 0, 1.0f, 0, 0, 0,  // v4-v3-v2-v1.0f (bottom)

    1.0f, 1.0f, 0, 1.0f, 0, 0, 0, 0, 0, 0, 1.0f, 0 };     // v5-v4-v1.0f-v6 (back)


const float wyOff = 0.9999f;
const GLfloat VoxelMesher::waterCubeVertices[72] = { 0.0f, wyOff, 1.000f, 0.0f, 0.0f, 1.000f, 1.000f, 0.0f, wyOff, 1.000f, wyOff, 1.000f,  // v1-v2-v3-v0 (front)

    1.000f, wyOff, 1.000f, 1.000f, 0.0f, 1.000f, 1.000f, 0.0f, 0.0f, 1.000f, wyOff, 0.0f,     // v0-v3-v4-v5 (right)

    0.0f, wyOff, 0.0f, 0.0f, wyOff, 1.000f, 1.000f, wyOff, 1.000f, 1.000f, wyOff, 0.0f,    // v6-v1-v0-v5 (top)

    0.0f, wyOff, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.000f, 0.0f, wyOff, 1.000f,   // v6-v7-v2-v1 (left)

    0.0f, 0.0f, 0.0f, 1.000f, 0.0f, 0.0f, 1.000f, 0.0f, 1.000f, 0.0f, 0.0f, 1.000f,    // v7-v4-v3-v2 (bottom)

    1.000f, wyOff, 0.0f, 1.000f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, wyOff, 0.0f };     // v5-v4-v7-v6 (back)

// 1 for normalized bytes
#define N_1 127

const GLbyte VoxelMesher::cubeNormals[72] = { 0, 0, N_1, 0, 0, N_1, 0, 0, N_1, 0, 0, N_1,  // v1-v2-v3-v0 (front)

    N_1, 0, 0, N_1, 0, 0, N_1, 0, 0, N_1, 0, 0,     // v0-v3-v4-v5 (right)

    0, N_1, 0, 0, N_1, 0, 0, N_1, 0, 0, N_1, 0,    // v6-v1-v0-v5 (top)

    -N_1, 0, 0, -N_1, 0, 0, -N_1, 0, 0, -N_1, 0, 0,   // v6-v7-v2-v1 (left)

    0, -N_1, 0, 0, -N_1, 0, 0, -N_1, 0, 0, -N_1, 0,    // v4-v3-v2-v7  (bottom)

    0, 0, -N_1, 0, 0, -N_1, 0, 0, -N_1, 0, 0, -N_1 };     // v5-v4-v7-v6 (back)

//For flora, normal is strait up
const GLbyte VoxelMesher::floraNormals[72] = { 0, N_1, 0, 0, N_1, 0, 0, N_1, 0, 0, N_1, 0,
    0, N_1, 0, 0, N_1, 0, 0, N_1, 0, 0, N_1, 0,
    0, N_1, 0, 0, N_1, 0, 0, N_1, 0, 0, N_1, 0,
    0, N_1, 0, 0, N_1, 0, 0, N_1, 0, 0, N_1, 0,
    0, N_1, 0, 0, N_1, 0, 0, N_1, 0, 0, N_1, 0,
    0, N_1, 0, 0, N_1, 0, 0, N_1, 0, 0, N_1, 0 };

//We use 4 meshes so that we can add variation to the flora meshes
const ui8 VoxelMesher::floraVertices[NUM_FLORA_MESHES][36] = {
    {
        0, 7, 5, 0, 0, 5, 7, 0, 5, 7, 7, 5,
        6, 7, 7, 6, 0, 7, 1, 0, 0, 1, 7, 0,
        1, 7, 7, 1, 0, 7, 6, 0, 0, 6, 7, 0 },
    {
        2, 7, 0, 2, 0, 0, 2, 0, 7, 2, 7, 7,
        0, 7, 1, 0, 0, 1, 7, 0, 6, 7, 7, 6,
        0, 7, 6, 0, 0, 6, 7, 0, 1, 7, 7, 1 },
    {
        0, 7, 2, 0, 0, 2, 7, 0, 2, 7, 7, 2,
        6, 7, 0, 6, 0, 0, 1, 0, 7, 1, 7, 7,
        1, 7, 0, 1, 0, 0, 6, 0, 7, 6, 7, 7 },
    {
        5, 7, 0, 5, 0, 0, 5, 0, 7, 5, 7, 7,
        7, 7, 1, 7, 0, 1, 0, 0, 6, 0, 7, 6,
        7, 7, 6, 7, 0, 6, 0, 0, 1, 0, 7, 1 } };

const ui8 VoxelMesher::crossFloraVertices[NUM_CROSSFLORA_MESHES][24] = {
    {
        0, 7, 0, 0, 0, 0, 7, 0, 7, 7, 7, 7,
        0, 7, 7, 0, 0, 7, 7, 0, 0, 7, 7, 0 },
    {
        7, 7, 7, 7, 0, 7, 0, 0, 0, 0, 7, 0,
        7, 7, 0, 7, 0, 0, 0, 0, 7, 0, 7, 7 } };


void VoxelMesher::makeFloraFace(BlockVertex *Verts, const ui8* positions, const i8* normals, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const ColorRGB8& color, const ColorRGB8& overlayColor, const ui8 sunlight, const ColorRGB8& lampColor, const BlockTexture* texInfo)
{

//    // 7 per coord
//    pos.x *= POSITION_RESOLUTION;
//    pos.y *= POSITION_RESOLUTION;
//    pos.z *= POSITION_RESOLUTION;
//
//    //Blend type. The 6 LSBs are used to encode alpha blending, add/subtract, and multiplication factors.
//    //They are used in the shader to determine how to blend.
//    ui8 blendMode = getBlendMode(texInfo->blendMode);
//
//    Verts[vertexIndex].blendMode = blendMode;
//    Verts[vertexIndex + 1].blendMode = blendMode;
//    Verts[vertexIndex + 2].blendMode = blendMode;
//    Verts[vertexIndex + 3].blendMode = blendMode;
//
//    GLubyte texAtlas = (GLubyte)(textureIndex / ATLAS_SIZE);
//    textureIndex %= ATLAS_SIZE;
//
//    GLubyte overlayTexAtlas = (GLubyte)(overlayTextureIndex / ATLAS_SIZE);
//    GLubyte overlayTex = (GLubyte)(overlayTextureIndex % ATLAS_SIZE);
//
//    Verts[vertexIndex].textureWidth = (ubyte)texInfo->base.size.x;
//    Verts[vertexIndex].textureHeight = (ubyte)texInfo->base.size.y;
//    Verts[vertexIndex + 1].textureWidth = (ubyte)texInfo->base.size.x;
//    Verts[vertexIndex + 1].textureHeight = (ubyte)texInfo->base.size.y;
//    Verts[vertexIndex + 2].textureWidth = (ubyte)texInfo->base.size.x;
//    Verts[vertexIndex + 2].textureHeight = (ubyte)texInfo->base.size.y;
//    Verts[vertexIndex + 3].textureWidth = (ubyte)texInfo->base.size.x;
//    Verts[vertexIndex + 3].textureHeight = (ubyte)texInfo->base.size.y;
//
//    Verts[vertexIndex].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
//    Verts[vertexIndex].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
//    Verts[vertexIndex + 1].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
//    Verts[vertexIndex + 1].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
//    Verts[vertexIndex + 2].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
//    Verts[vertexIndex + 2].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
//    Verts[vertexIndex + 3].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
//    Verts[vertexIndex + 3].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
//
//    Verts[vertexIndex].position.x = pos.x + positions[vertexOffset];
//    Verts[vertexIndex].position.y = pos.y + positions[vertexOffset + 1];
//    Verts[vertexIndex].position.z = pos.z + positions[vertexOffset + 2];
//    Verts[vertexIndex + 1].position.x = pos.x + positions[vertexOffset + 3];
//    Verts[vertexIndex + 1].position.y = pos.y + positions[vertexOffset + 4];
//    Verts[vertexIndex + 1].position.z = pos.z + positions[vertexOffset + 5];
//    Verts[vertexIndex + 2].position.x = pos.x + positions[vertexOffset + 6];
//    Verts[vertexIndex + 2].position.y = pos.y + positions[vertexOffset + 7];
//    Verts[vertexIndex + 2].position.z = pos.z + positions[vertexOffset + 8];
//    Verts[vertexIndex + 3].position.x = pos.x + positions[vertexOffset + 9];
//    Verts[vertexIndex + 3].position.y = pos.y + positions[vertexOffset + 10];
//    Verts[vertexIndex + 3].position.z = pos.z + positions[vertexOffset + 11];
//
//    Verts[vertexIndex].color = color;
//    Verts[vertexIndex + 1].color = color;
//    Verts[vertexIndex + 2].color = color;
//    Verts[vertexIndex + 3].color = color;
//
//    Verts[vertexIndex].overlayColor = overlayColor;
//    Verts[vertexIndex + 1].overlayColor = overlayColor;
//    Verts[vertexIndex + 2].overlayColor = overlayColor;
//    Verts[vertexIndex + 3].overlayColor = overlayColor;
//
//    Verts[vertexIndex].normal[0] = normals[vertexOffset];
//    Verts[vertexIndex].normal[1] = normals[vertexOffset + 1];
//    Verts[vertexIndex].normal[2] = normals[vertexOffset + 2];
//    Verts[vertexIndex + 1].normal[0] = normals[vertexOffset + 3];
//    Verts[vertexIndex + 1].normal[1] = normals[vertexOffset + 4];
//    Verts[vertexIndex + 1].normal[2] = normals[vertexOffset + 5];
//    Verts[vertexIndex + 2].normal[0] = normals[vertexOffset + 6];
//    Verts[vertexIndex + 2].normal[1] = normals[vertexOffset + 7];
//    Verts[vertexIndex + 2].normal[2] = normals[vertexOffset + 8];
//    Verts[vertexIndex + 3].normal[0] = normals[vertexOffset + 9];
//    Verts[vertexIndex + 3].normal[1] = normals[vertexOffset + 10];
//    Verts[vertexIndex + 3].normal[2] = normals[vertexOffset + 11];
//
//    Verts[vertexIndex].lampColor = lampColor;
//    Verts[vertexIndex + 1].lampColor = lampColor;
//    Verts[vertexIndex + 2].lampColor = lampColor;
//    Verts[vertexIndex + 3].lampColor = lampColor;
//
//
//    Verts[vertexIndex].sunlight = sunlight;
//    Verts[vertexIndex + 1].sunlight = sunlight;
//    Verts[vertexIndex + 2].sunlight = sunlight;
//    Verts[vertexIndex + 3].sunlight = sunlight;
//
//    Verts[vertexIndex].merge = 0;
//    Verts[vertexIndex + 1].merge = 0;
//    Verts[vertexIndex + 2].merge = 0;
//    Verts[vertexIndex + 3].merge = 0;
//
//    if (waveEffect == 2){
//        Verts[vertexIndex].waveEffect = 255;
//        Verts[vertexIndex + 1].waveEffect = 255;
//        Verts[vertexIndex + 2].waveEffect = 255;
//        Verts[vertexIndex + 3].waveEffect = 255;
//    } else if (waveEffect == 1){
//        Verts[vertexIndex].waveEffect = 255;
//        Verts[vertexIndex + 1].waveEffect = 0;
//        Verts[vertexIndex + 2].waveEffect = 0;
//        Verts[vertexIndex + 3].waveEffect = 255;
//    } else{
//        Verts[vertexIndex].waveEffect = 0;
//        Verts[vertexIndex + 1].waveEffect = 0;
//        Verts[vertexIndex + 2].waveEffect = 0;
//        Verts[vertexIndex + 3].waveEffect = 0;
//    }
//
//#define UV_0 128
//#define UV_1 129
//
//    Verts[vertexIndex].tex[0] = UV_0;
//    Verts[vertexIndex].tex[1] = UV_1;
//    Verts[vertexIndex + 1].tex[0] = UV_0;
//    Verts[vertexIndex + 1].tex[1] = UV_0;
//    Verts[vertexIndex + 2].tex[0] = UV_1;
//    Verts[vertexIndex + 2].tex[1] = UV_0;
//    Verts[vertexIndex + 3].tex[0] = UV_1;
//    Verts[vertexIndex + 3].tex[1] = UV_1;
//
//    // *********** Base Texture
//    Verts[vertexIndex].textureIndex = (GLubyte)textureIndex;
//    Verts[vertexIndex + 1].textureIndex = (GLubyte)textureIndex;
//    Verts[vertexIndex + 2].textureIndex = (GLubyte)textureIndex;
//    Verts[vertexIndex + 3].textureIndex = (GLubyte)textureIndex;
//
//    Verts[vertexIndex].textureAtlas = (GLubyte)texAtlas;
//    Verts[vertexIndex + 1].textureAtlas = (GLubyte)texAtlas;
//    Verts[vertexIndex + 2].textureAtlas = (GLubyte)texAtlas;
//    Verts[vertexIndex + 3].textureAtlas = (GLubyte)texAtlas;
//
//    // *********** Overlay texture
//    Verts[vertexIndex].overlayTextureIndex = (GLubyte)overlayTex;
//    Verts[vertexIndex + 1].overlayTextureIndex = (GLubyte)overlayTex;
//    Verts[vertexIndex + 2].overlayTextureIndex = (GLubyte)overlayTex;
//    Verts[vertexIndex + 3].overlayTextureIndex = (GLubyte)overlayTex;
//
//    Verts[vertexIndex].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
//    Verts[vertexIndex + 1].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
//    Verts[vertexIndex + 2].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
//    Verts[vertexIndex + 3].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
}


void VoxelMesher::makeTransparentFace(BlockVertex *Verts, const ui8* positions, const i8* normals, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const ColorRGB8& color, const ColorRGB8& overlayColor, const ui8 sunlight, const ColorRGB8& lampColor, const BlockTexture* texInfo) {
//
//    //get the face index so we can determine the axis alignment
//    int faceIndex = vertexOffset / 12;
//    //Multiply the axis by the sign bit to get the correct offset
//    GLubyte uOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][0]] * cubeFaceAxisSign[faceIndex][0]);
//    GLubyte vOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][1]] * cubeFaceAxisSign[faceIndex][1]);
//
//    // 7 per coord
//    pos.x *= POSITION_RESOLUTION;
//    pos.y *= POSITION_RESOLUTION;
//    pos.z *= POSITION_RESOLUTION;
//
//    //Blend type. The 6 LSBs are used to encode alpha blending, add/subtract, and multiplication factors.
//    //They are used in the shader to determine how to blend.
//    ui8 blendMode = getBlendMode(texInfo->blendMode);
//    
//    Verts[vertexIndex].blendMode = blendMode;
//    Verts[vertexIndex + 1].blendMode = blendMode;
//    Verts[vertexIndex + 2].blendMode = blendMode;
//    Verts[vertexIndex + 3].blendMode = blendMode;
//
//    GLubyte texAtlas = (GLubyte)(textureIndex / ATLAS_SIZE);
//    textureIndex %= ATLAS_SIZE;
//
//    GLubyte overlayTexAtlas = (GLubyte)(overlayTextureIndex / ATLAS_SIZE);
//    GLubyte overlayTex = (GLubyte)(overlayTextureIndex % ATLAS_SIZE);
//
//    Verts[vertexIndex].textureWidth = (ubyte)texInfo->base.size.x;
//    Verts[vertexIndex].textureHeight = (ubyte)texInfo->base.size.y;
//    Verts[vertexIndex + 1].textureWidth = (ubyte)texInfo->base.size.x;
//    Verts[vertexIndex + 1].textureHeight = (ubyte)texInfo->base.size.y;
//    Verts[vertexIndex + 2].textureWidth = (ubyte)texInfo->base.size.x;
//    Verts[vertexIndex + 2].textureHeight = (ubyte)texInfo->base.size.y;
//    Verts[vertexIndex + 3].textureWidth = (ubyte)texInfo->base.size.x;
//    Verts[vertexIndex + 3].textureHeight = (ubyte)texInfo->base.size.y;
//
//    Verts[vertexIndex].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
//    Verts[vertexIndex].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
//    Verts[vertexIndex + 1].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
//    Verts[vertexIndex + 1].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
//    Verts[vertexIndex + 2].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
//    Verts[vertexIndex + 2].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
//    Verts[vertexIndex + 3].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
//    Verts[vertexIndex + 3].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
//
//    Verts[vertexIndex].position.x = pos.x + positions[vertexOffset];
//    Verts[vertexIndex].position.y = pos.y + positions[vertexOffset + 1];
//    Verts[vertexIndex].position.z = pos.z + positions[vertexOffset + 2];
//    Verts[vertexIndex + 1].position.x = pos.x + positions[vertexOffset + 3];
//    Verts[vertexIndex + 1].position.y = pos.y + positions[vertexOffset + 4];
//    Verts[vertexIndex + 1].position.z = pos.z + positions[vertexOffset + 5];
//    Verts[vertexIndex + 2].position.x = pos.x + positions[vertexOffset + 6];
//    Verts[vertexIndex + 2].position.y = pos.y + positions[vertexOffset + 7];
//    Verts[vertexIndex + 2].position.z = pos.z + positions[vertexOffset + 8];
//    Verts[vertexIndex + 3].position.x = pos.x + positions[vertexOffset + 9];
//    Verts[vertexIndex + 3].position.y = pos.y + positions[vertexOffset + 10];
//    Verts[vertexIndex + 3].position.z = pos.z + positions[vertexOffset + 11];
//
//    Verts[vertexIndex].color = color;
//    Verts[vertexIndex + 1].color = color;
//    Verts[vertexIndex + 2].color = color;
//    Verts[vertexIndex + 3].color = color;
//
//    Verts[vertexIndex].overlayColor = overlayColor;
//    Verts[vertexIndex + 1].overlayColor = overlayColor;
//    Verts[vertexIndex + 2].overlayColor = overlayColor;
//    Verts[vertexIndex + 3].overlayColor = overlayColor;
//
//    Verts[vertexIndex].normal[0] = normals[vertexOffset];
//    Verts[vertexIndex].normal[1] = normals[vertexOffset + 1];
//    Verts[vertexIndex].normal[2] = normals[vertexOffset + 2];
//    Verts[vertexIndex + 1].normal[0] = normals[vertexOffset + 3];
//    Verts[vertexIndex + 1].normal[1] = normals[vertexOffset + 4];
//    Verts[vertexIndex + 1].normal[2] = normals[vertexOffset + 5];
//    Verts[vertexIndex + 2].normal[0] = normals[vertexOffset + 6];
//    Verts[vertexIndex + 2].normal[1] = normals[vertexOffset + 7];
//    Verts[vertexIndex + 2].normal[2] = normals[vertexOffset + 8];
//    Verts[vertexIndex + 3].normal[0] = normals[vertexOffset + 9];
//    Verts[vertexIndex + 3].normal[1] = normals[vertexOffset + 10];
//    Verts[vertexIndex + 3].normal[2] = normals[vertexOffset + 11];
//
//    Verts[vertexIndex].lampColor = lampColor;
//    Verts[vertexIndex + 1].lampColor = lampColor;
//    Verts[vertexIndex + 2].lampColor = lampColor;
//    Verts[vertexIndex + 3].lampColor = lampColor;
//
//
//    Verts[vertexIndex].sunlight = sunlight;
//    Verts[vertexIndex + 1].sunlight = sunlight;
//    Verts[vertexIndex + 2].sunlight = sunlight;
//    Verts[vertexIndex + 3].sunlight = sunlight;
//
//    Verts[vertexIndex].merge = 0;
//    Verts[vertexIndex + 1].merge = 0;
//    Verts[vertexIndex + 2].merge = 0;
//    Verts[vertexIndex + 3].merge = 0;
//
//    if (waveEffect == 2) {
//        Verts[vertexIndex].waveEffect = 255;
//        Verts[vertexIndex + 1].waveEffect = 255;
//        Verts[vertexIndex + 2].waveEffect = 255;
//        Verts[vertexIndex + 3].waveEffect = 255;
//    } else if (waveEffect == 1) {
//        Verts[vertexIndex].waveEffect = 255;
//        Verts[vertexIndex + 1].waveEffect = 0;
//        Verts[vertexIndex + 2].waveEffect = 0;
//        Verts[vertexIndex + 3].waveEffect = 255;
//    } else {
//        Verts[vertexIndex].waveEffect = 0;
//        Verts[vertexIndex + 1].waveEffect = 0;
//        Verts[vertexIndex + 2].waveEffect = 0;
//        Verts[vertexIndex + 3].waveEffect = 0;
//    }
//
//#define UV_0 128
//#define UV_1 129
//
//    Verts[vertexIndex].tex[0] = UV_0 + uOffset;
//    Verts[vertexIndex].tex[1] = UV_1 + vOffset;
//    Verts[vertexIndex + 1].tex[0] = UV_0 + uOffset;
//    Verts[vertexIndex + 1].tex[1] = UV_0 + vOffset;
//    Verts[vertexIndex + 2].tex[0] = UV_1 + uOffset;
//    Verts[vertexIndex + 2].tex[1] = UV_0 + vOffset;
//    Verts[vertexIndex + 3].tex[0] = UV_1 + uOffset;
//    Verts[vertexIndex + 3].tex[1] = UV_1 + vOffset;
//
//    // *********** Base Texture
//    Verts[vertexIndex].textureIndex = (GLubyte)textureIndex;
//    Verts[vertexIndex + 1].textureIndex = (GLubyte)textureIndex;
//    Verts[vertexIndex + 2].textureIndex = (GLubyte)textureIndex;
//    Verts[vertexIndex + 3].textureIndex = (GLubyte)textureIndex;
//
//    Verts[vertexIndex].textureAtlas = (GLubyte)texAtlas;
//    Verts[vertexIndex + 1].textureAtlas = (GLubyte)texAtlas;
//    Verts[vertexIndex + 2].textureAtlas = (GLubyte)texAtlas;
//    Verts[vertexIndex + 3].textureAtlas = (GLubyte)texAtlas;
//
//    // *********** Overlay texture
//    Verts[vertexIndex].overlayTextureIndex = (GLubyte)overlayTex;
//    Verts[vertexIndex + 1].overlayTextureIndex = (GLubyte)overlayTex;
//    Verts[vertexIndex + 2].overlayTextureIndex = (GLubyte)overlayTex;
//    Verts[vertexIndex + 3].overlayTextureIndex = (GLubyte)overlayTex;
//
//    Verts[vertexIndex].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
//    Verts[vertexIndex + 1].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
//    Verts[vertexIndex + 2].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
//    Verts[vertexIndex + 3].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
}


void VoxelMesher::makeCubeFace(BlockVertex *Verts, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const ColorRGB8& color, const ColorRGB8& overlayColor, GLfloat ambientOcclusion[], const BlockTexture* texInfo)
{

    ////get the face index so we can determine the axis alignment
    //int faceIndex = vertexOffset / 12;
    ////Multiply the axis by the sign bit to get the correct offset
    //GLubyte uOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][0]] * cubeFaceAxisSign[faceIndex][0]);
    //GLubyte vOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][1]] * cubeFaceAxisSign[faceIndex][1]);

    //const GLubyte* cverts = cubeVertices;

    //// 7 per coord
    //pos.x *= POSITION_RESOLUTION;
    //pos.y *= POSITION_RESOLUTION;
    //pos.z *= POSITION_RESOLUTION;

    ////Blend type. The 6 LSBs are used to encode alpha blending, add/subtract, and multiplication factors.
    ////They are used in the shader to determine how to blend.
    //ui8 blendMode = getBlendMode(texInfo->blendMode);
    //
    //Verts[vertexIndex].blendMode = blendMode;
    //Verts[vertexIndex + 1].blendMode = blendMode;
    //Verts[vertexIndex + 2].blendMode = blendMode;
    //Verts[vertexIndex + 3].blendMode = blendMode;

    //GLubyte texAtlas = (GLubyte)(textureIndex / ATLAS_SIZE);
    //textureIndex %= ATLAS_SIZE;

    //GLubyte overlayTexAtlas = (GLubyte)(overlayTextureIndex / ATLAS_SIZE);
    //GLubyte overlayTex = (GLubyte)(overlayTextureIndex % ATLAS_SIZE);

    //Verts[vertexIndex].textureWidth = (ubyte)texInfo->base.size.x;
    //Verts[vertexIndex].textureHeight = (ubyte)texInfo->base.size.y;
    //Verts[vertexIndex + 1].textureWidth = (ubyte)texInfo->base.size.x;
    //Verts[vertexIndex + 1].textureHeight = (ubyte)texInfo->base.size.y;
    //Verts[vertexIndex + 2].textureWidth = (ubyte)texInfo->base.size.x;
    //Verts[vertexIndex + 2].textureHeight = (ubyte)texInfo->base.size.y;
    //Verts[vertexIndex + 3].textureWidth = (ubyte)texInfo->base.size.x;
    //Verts[vertexIndex + 3].textureHeight = (ubyte)texInfo->base.size.y;

    //Verts[vertexIndex].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
    //Verts[vertexIndex].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
    //Verts[vertexIndex + 1].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
    //Verts[vertexIndex + 1].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
    //Verts[vertexIndex + 2].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
    //Verts[vertexIndex + 2].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;
    //Verts[vertexIndex + 3].overlayTextureWidth = (ubyte)texInfo->overlay.size.x;
    //Verts[vertexIndex + 3].overlayTextureHeight = (ubyte)texInfo->overlay.size.y;

    //Verts[vertexIndex].position.x = pos.x + cverts[vertexOffset];
    //Verts[vertexIndex].position.y = pos.y + cverts[vertexOffset + 1];
    //Verts[vertexIndex].position.z = pos.z + cverts[vertexOffset + 2];
    //Verts[vertexIndex + 1].position.x = pos.x + cverts[vertexOffset + 3];
    //Verts[vertexIndex + 1].position.y = pos.y + cverts[vertexOffset + 4];
    //Verts[vertexIndex + 1].position.z = pos.z + cverts[vertexOffset + 5];
    //Verts[vertexIndex + 2].position.x = pos.x + cverts[vertexOffset + 6];
    //Verts[vertexIndex + 2].position.y = pos.y + cverts[vertexOffset + 7];
    //Verts[vertexIndex + 2].position.z = pos.z + cverts[vertexOffset + 8];
    //Verts[vertexIndex + 3].position.x = pos.x + cverts[vertexOffset + 9];
    //Verts[vertexIndex + 3].position.y = pos.y + cverts[vertexOffset + 10];
    //Verts[vertexIndex + 3].position.z = pos.z + cverts[vertexOffset + 11];

    //Verts[vertexIndex].color.r = (GLubyte)(color.r * ambientOcclusion[0]);
    //Verts[vertexIndex].color.g = (GLubyte)(color.g * ambientOcclusion[0]);
    //Verts[vertexIndex].color.b = (GLubyte)(color.b * ambientOcclusion[0]);
    //Verts[vertexIndex + 1].color.r = (GLubyte)(color.r * ambientOcclusion[1]);
    //Verts[vertexIndex + 1].color.g = (GLubyte)(color.g * ambientOcclusion[1]);
    //Verts[vertexIndex + 1].color.b = (GLubyte)(color.b * ambientOcclusion[1]);
    //Verts[vertexIndex + 2].color.r = (GLubyte)(color.r * ambientOcclusion[2]);
    //Verts[vertexIndex + 2].color.g = (GLubyte)(color.g * ambientOcclusion[2]);
    //Verts[vertexIndex + 2].color.b = (GLubyte)(color.b * ambientOcclusion[2]);
    //Verts[vertexIndex + 3].color.r = (GLubyte)(color.r * ambientOcclusion[3]);
    //Verts[vertexIndex + 3].color.g = (GLubyte)(color.g * ambientOcclusion[3]);
    //Verts[vertexIndex + 3].color.b = (GLubyte)(color.b * ambientOcclusion[3]);

    //Verts[vertexIndex].overlayColor.r = (GLubyte)(overlayColor.r * ambientOcclusion[0]);
    //Verts[vertexIndex].overlayColor.g = (GLubyte)(overlayColor.g * ambientOcclusion[0]);
    //Verts[vertexIndex].overlayColor.b = (GLubyte)(overlayColor.b * ambientOcclusion[0]);
    //Verts[vertexIndex + 1].overlayColor.r = (GLubyte)(overlayColor.r * ambientOcclusion[1]);
    //Verts[vertexIndex + 1].overlayColor.g = (GLubyte)(overlayColor.g * ambientOcclusion[1]);
    //Verts[vertexIndex + 1].overlayColor.b = (GLubyte)(overlayColor.b * ambientOcclusion[1]);
    //Verts[vertexIndex + 2].overlayColor.r = (GLubyte)(overlayColor.r * ambientOcclusion[2]);
    //Verts[vertexIndex + 2].overlayColor.g = (GLubyte)(overlayColor.g * ambientOcclusion[2]);
    //Verts[vertexIndex + 2].overlayColor.b = (GLubyte)(overlayColor.b * ambientOcclusion[2]);
    //Verts[vertexIndex + 3].overlayColor.r = (GLubyte)(overlayColor.r * ambientOcclusion[3]);
    //Verts[vertexIndex + 3].overlayColor.g = (GLubyte)(overlayColor.g * ambientOcclusion[3]);
    //Verts[vertexIndex + 3].overlayColor.b = (GLubyte)(overlayColor.b * ambientOcclusion[3]);

    //Verts[vertexIndex].normal[0] = cubeNormals[vertexOffset];
    //Verts[vertexIndex].normal[1] = cubeNormals[vertexOffset + 1];
    //Verts[vertexIndex].normal[2] = cubeNormals[vertexOffset + 2];
    //Verts[vertexIndex + 1].normal[0] = cubeNormals[vertexOffset + 3];
    //Verts[vertexIndex + 1].normal[1] = cubeNormals[vertexOffset + 4];
    //Verts[vertexIndex + 1].normal[2] = cubeNormals[vertexOffset + 5];
    //Verts[vertexIndex + 2].normal[0] = cubeNormals[vertexOffset + 6];
    //Verts[vertexIndex + 2].normal[1] = cubeNormals[vertexOffset + 7];
    //Verts[vertexIndex + 2].normal[2] = cubeNormals[vertexOffset + 8];
    //Verts[vertexIndex + 3].normal[0] = cubeNormals[vertexOffset + 9];
    //Verts[vertexIndex + 3].normal[1] = cubeNormals[vertexOffset + 10];
    //Verts[vertexIndex + 3].normal[2] = cubeNormals[vertexOffset + 11];

    //Verts[vertexIndex].merge = 1;
    //Verts[vertexIndex + 1].merge = 1;
    //Verts[vertexIndex + 2].merge = 1;
    //Verts[vertexIndex + 3].merge = 1;

    //if (waveEffect == 2){
    //    Verts[vertexIndex].waveEffect = 255;
    //    Verts[vertexIndex + 1].waveEffect = 255;
    //    Verts[vertexIndex + 2].waveEffect = 255;
    //    Verts[vertexIndex + 3].waveEffect = 255;
    //} else if (waveEffect == 1){
    //    Verts[vertexIndex].waveEffect = 255;
    //    Verts[vertexIndex + 1].waveEffect = 0;
    //    Verts[vertexIndex + 2].waveEffect = 0;
    //    Verts[vertexIndex + 3].waveEffect = 255;
    //} else{
    //    Verts[vertexIndex].waveEffect = 0;
    //    Verts[vertexIndex + 1].waveEffect = 0;
    //    Verts[vertexIndex + 2].waveEffect = 0;
    //    Verts[vertexIndex + 3].waveEffect = 0;
    //}

    //#define UV_0 128
    //#define UV_1 129

    //Verts[vertexIndex].tex[0] = UV_0 + uOffset;
    //Verts[vertexIndex].tex[1] = UV_1 + vOffset;
    //Verts[vertexIndex + 1].tex[0] = UV_0 + uOffset;
    //Verts[vertexIndex + 1].tex[1] = UV_0 + vOffset;
    //Verts[vertexIndex + 2].tex[0] = UV_1 + uOffset;
    //Verts[vertexIndex + 2].tex[1] = UV_0 + vOffset;
    //Verts[vertexIndex + 3].tex[0] = UV_1 + uOffset;
    //Verts[vertexIndex + 3].tex[1] = UV_1 + vOffset;

    //// *********** Base Texture
    //Verts[vertexIndex].textureIndex = (GLubyte)textureIndex;
    //Verts[vertexIndex + 1].textureIndex = (GLubyte)textureIndex;
    //Verts[vertexIndex + 2].textureIndex = (GLubyte)textureIndex;
    //Verts[vertexIndex + 3].textureIndex = (GLubyte)textureIndex;

    //Verts[vertexIndex].textureAtlas = (GLubyte)texAtlas;
    //Verts[vertexIndex + 1].textureAtlas = (GLubyte)texAtlas;
    //Verts[vertexIndex + 2].textureAtlas = (GLubyte)texAtlas;
    //Verts[vertexIndex + 3].textureAtlas = (GLubyte)texAtlas;

    //// *********** Overlay texture
    //Verts[vertexIndex].overlayTextureIndex = (GLubyte)overlayTex;
    //Verts[vertexIndex + 1].overlayTextureIndex = (GLubyte)overlayTex;
    //Verts[vertexIndex + 2].overlayTextureIndex = (GLubyte)overlayTex;
    //Verts[vertexIndex + 3].overlayTextureIndex = (GLubyte)overlayTex;

    //Verts[vertexIndex].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    //Verts[vertexIndex + 1].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    //Verts[vertexIndex + 2].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    //Verts[vertexIndex + 3].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
}

const GLubyte waterUVs[8] = { 0, 7, 0, 0, 7, 0, 7, 7 };

void VoxelMesher::makeLiquidFace(std::vector<LiquidVertex>& verts, i32 index, ui8 uOff, ui8 vOff, const ColorRGB8& lampColor, ui8 sunlight, const ColorRGB8& color, ui8 textureUnit) {

    verts.resize(verts.size() + 4);
    verts[index].tex[0] = waterUVs[0] + uOff;
    verts[index].tex[1] = waterUVs[1] + vOff;
    verts[index + 1].tex[0] = waterUVs[2] + uOff;
    verts[index + 1].tex[1] = waterUVs[3] + vOff;
    verts[index + 2].tex[0] = waterUVs[4] + uOff;
    verts[index + 2].tex[1] = waterUVs[5] + vOff;
    verts[index + 3].tex[0] = waterUVs[6] + uOff;
    verts[index + 3].tex[1] = waterUVs[7] + vOff;

    verts[index].lampColor = lampColor;
    verts[index + 1].lampColor = lampColor;
    verts[index + 2].lampColor = lampColor;
    verts[index + 3].lampColor = lampColor;

    verts[index].sunlight = sunlight;
    verts[index + 1].sunlight = sunlight;
    verts[index + 2].sunlight = sunlight;
    verts[index + 3].sunlight = sunlight;

    verts[index].color.r = color.r;
    verts[index].color.g = color.g;
    verts[index].color.b = color.b;
    verts[index + 1].color.r = color.r;
    verts[index + 1].color.g = color.g;
    verts[index + 1].color.b = color.b;
    verts[index + 2].color.r = color.r;
    verts[index + 2].color.g = color.g;
    verts[index + 2].color.b = color.b;
    verts[index + 3].color.r = color.r;
    verts[index + 3].color.g = color.g;
    verts[index + 3].color.b = color.b;

    verts[index].textureUnit = (GLubyte)textureUnit;
    verts[index + 1].textureUnit = (GLubyte)textureUnit;
    verts[index + 2].textureUnit = (GLubyte)textureUnit;
    verts[index + 3].textureUnit = (GLubyte)textureUnit;
}

void VoxelMesher::makePhysicsBlockFace(std::vector <PhysicsBlockVertex> &verts, int vertexOffset, int &index, const BlockTexture& blockTexture)
{
    /*  ui8 textureAtlas = (ui8)(blockTexture.base.index / ATLAS_SIZE);
      ui8 textureIndex = (ui8)(blockTexture.base.index % ATLAS_SIZE);

      ui8 overlayTextureAtlas = (ui8)(blockTexture.overlay.index / ATLAS_SIZE);
      ui8 overlayTextureIndex = (ui8)(blockTexture.overlay.index % ATLAS_SIZE);

      ui8 blendMode = getBlendMode(blockTexture.blendMode);

      const GLubyte* cverts = cubeVertices;

      verts[index].blendMode = blendMode;
      verts[index + 1].blendMode = blendMode;
      verts[index + 2].blendMode = blendMode;
      verts[index + 3].blendMode = blendMode;
      verts[index + 4].blendMode = blendMode;
      verts[index + 5].blendMode = blendMode;

      verts[index].textureWidth = (ui8)blockTexture.base.size.x;
      verts[index].textureHeight = (ui8)blockTexture.base.size.y;
      verts[index + 1].textureWidth = (ui8)blockTexture.base.size.x;
      verts[index + 1].textureHeight = (ui8)blockTexture.base.size.y;
      verts[index + 2].textureWidth = (ui8)blockTexture.base.size.x;
      verts[index + 2].textureHeight = (ui8)blockTexture.base.size.y;
      verts[index + 3].textureWidth = (ui8)blockTexture.base.size.x;
      verts[index + 3].textureHeight = (ui8)blockTexture.base.size.y;
      verts[index + 4].textureWidth = (ui8)blockTexture.base.size.x;
      verts[index + 4].textureHeight = (ui8)blockTexture.base.size.y;
      verts[index + 5].textureWidth = (ui8)blockTexture.base.size.x;
      verts[index + 5].textureHeight = (ui8)blockTexture.base.size.y;

      verts[index].overlayTextureWidth = (ui8)blockTexture.overlay.size.x;
      verts[index].overlayTextureHeight = (ui8)blockTexture.overlay.size.y;
      verts[index + 1].overlayTextureWidth = (ui8)blockTexture.overlay.size.x;
      verts[index + 1].overlayTextureHeight = (ui8)blockTexture.overlay.size.y;
      verts[index + 2].overlayTextureWidth = (ui8)blockTexture.overlay.size.x;
      verts[index + 2].overlayTextureHeight = (ui8)blockTexture.overlay.size.y;
      verts[index + 3].overlayTextureWidth = (ui8)blockTexture.overlay.size.x;
      verts[index + 3].overlayTextureHeight = (ui8)blockTexture.overlay.size.y;
      verts[index + 4].overlayTextureWidth = (ui8)blockTexture.overlay.size.x;
      verts[index + 4].overlayTextureHeight = (ui8)blockTexture.overlay.size.y;
      verts[index + 5].overlayTextureWidth = (ui8)blockTexture.overlay.size.x;
      verts[index + 5].overlayTextureHeight = (ui8)blockTexture.overlay.size.y;

      verts[index].normal[0] = cubeNormals[vertexOffset];
      verts[index].normal[1] = cubeNormals[vertexOffset + 1];
      verts[index].normal[2] = cubeNormals[vertexOffset + 2];
      verts[index + 1].normal[0] = cubeNormals[vertexOffset + 3];
      verts[index + 1].normal[1] = cubeNormals[vertexOffset + 4];
      verts[index + 1].normal[2] = cubeNormals[vertexOffset + 5];
      verts[index + 2].normal[0] = cubeNormals[vertexOffset + 6];
      verts[index + 2].normal[1] = cubeNormals[vertexOffset + 7];
      verts[index + 2].normal[2] = cubeNormals[vertexOffset + 8];
      verts[index + 3].normal[0] = cubeNormals[vertexOffset + 6];
      verts[index + 3].normal[1] = cubeNormals[vertexOffset + 7];
      verts[index + 3].normal[2] = cubeNormals[vertexOffset + 8];
      verts[index + 4].normal[0] = cubeNormals[vertexOffset + 9];
      verts[index + 4].normal[1] = cubeNormals[vertexOffset + 10];
      verts[index + 4].normal[2] = cubeNormals[vertexOffset + 11];
      verts[index + 5].normal[0] = cubeNormals[vertexOffset];
      verts[index + 5].normal[1] = cubeNormals[vertexOffset + 1];
      verts[index + 5].normal[2] = cubeNormals[vertexOffset + 2];

      verts[index].position[0] = cverts[vertexOffset];
      verts[index].position[1] = cverts[vertexOffset + 1];
      verts[index].position[2] = cverts[vertexOffset + 2];
      verts[index + 1].position[0] = cverts[vertexOffset + 3];
      verts[index + 1].position[1] = cverts[vertexOffset + 4];
      verts[index + 1].position[2] = cverts[vertexOffset + 5];
      verts[index + 2].position[0] = cverts[vertexOffset + 6];
      verts[index + 2].position[1] = cverts[vertexOffset + 7];
      verts[index + 2].position[2] = cverts[vertexOffset + 8];
      verts[index + 3].position[0] = cverts[vertexOffset + 6];
      verts[index + 3].position[1] = cverts[vertexOffset + 7];
      verts[index + 3].position[2] = cverts[vertexOffset + 8];
      verts[index + 4].position[0] = cverts[vertexOffset + 9];
      verts[index + 4].position[1] = cverts[vertexOffset + 10];
      verts[index + 4].position[2] = cverts[vertexOffset + 11];
      verts[index + 5].position[0] = cverts[vertexOffset];
      verts[index + 5].position[1] = cverts[vertexOffset + 1];
      verts[index + 5].position[2] = cverts[vertexOffset + 2];

      #define UV_0 128
      #define UV_1 129

      verts[index].tex[0] = UV_0;
      verts[index].tex[1] = UV_1;
      verts[index + 1].tex[0] = UV_0;
      verts[index + 1].tex[1] = UV_0;
      verts[index + 2].tex[0] = UV_1;
      verts[index + 2].tex[1] = UV_0;
      verts[index + 3].tex[0] = UV_1;
      verts[index + 3].tex[1] = UV_0;
      verts[index + 4].tex[0] = UV_1;
      verts[index + 4].tex[1] = UV_1;
      verts[index + 5].tex[0] = UV_0;
      verts[index + 5].tex[1] = UV_1;

      verts[index].textureAtlas = textureAtlas;
      verts[index + 1].textureAtlas = textureAtlas;
      verts[index + 2].textureAtlas = textureAtlas;
      verts[index + 3].textureAtlas = textureAtlas;
      verts[index + 4].textureAtlas = textureAtlas;
      verts[index + 5].textureAtlas = textureAtlas;

      verts[index].textureIndex = textureIndex;
      verts[index + 1].textureIndex = textureIndex;
      verts[index + 2].textureIndex = textureIndex;
      verts[index + 3].textureIndex = textureIndex;
      verts[index + 4].textureIndex = textureIndex;
      verts[index + 5].textureIndex = textureIndex;

      verts[index].overlayTextureAtlas = overlayTextureAtlas;
      verts[index + 1].overlayTextureAtlas = overlayTextureAtlas;
      verts[index + 2].overlayTextureAtlas = overlayTextureAtlas;
      verts[index + 3].overlayTextureAtlas = overlayTextureAtlas;
      verts[index + 4].overlayTextureAtlas = overlayTextureAtlas;
      verts[index + 5].overlayTextureAtlas = overlayTextureAtlas;

      verts[index].overlayTextureIndex = overlayTextureIndex;
      verts[index + 1].overlayTextureIndex = overlayTextureIndex;
      verts[index + 2].overlayTextureIndex = overlayTextureIndex;
      verts[index + 3].overlayTextureIndex = overlayTextureIndex;
      verts[index + 4].overlayTextureIndex = overlayTextureIndex;
      verts[index + 5].overlayTextureIndex = overlayTextureIndex;*/

}

ui8 VoxelMesher::getBlendMode(const BlendType& blendType) {
    ubyte blendMode = 0x14; //0x14 = 00 01 01 00
    switch (blendType) {
        case BlendType::ALPHA:
            blendMode |= 1; //Sets blendMode to 00 01 01 01
            break;
        case BlendType::ADD:
            blendMode += 4; //Sets blendMode to 00 01 10 00
            break;
        case BlendType::SUBTRACT:
            blendMode -= 4; //Sets blendMode to 00 01 00 00
            break;
        case BlendType::MULTIPLY:
            blendMode -= 16; //Sets blendMode to 00 00 01 00
            break;
    }
    return blendMode;
}