#include "stdafx.h"
#include "BlockMesher.h"

// cube //
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3 

const GLfloat BlockMesher::leafVertices[72] = { -0.0f, 1.0f, 0.5f, -0.0f, -0.0f, 0.5f, 1.0f, -0.0f, 0.5f, 1.0f, 1.0f, 0.5f,  // v1-v2-v3-v0 (front)

    0.5f, 1.0f, 1.0f, 0.5f, -0.0f, 1.0f, 0.5f, -0.0f, -0.0f, 0.5f, 1.0f, -0.0f,     // v0-v3-v4-v5 (right)

    -0.0f, 0.5f, -0.0f, -0.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, -0.0f,    // v6-v1-v0-v5 (top)

    0.5f, 1.0f, -0.0f, 0.5f, -0.0f, -0.0f, 0.5f, -0.0f, 1.0f, 0.5f, 1.0f, 1.0f,   // v6-v7-v2-v1 (left)

    -0.0f, 0.5f, -0.0f, 1.0f, 0.5f, -0.0f, 1.0f, 0.5f, 1.0f, -0.0f, 0.5f, 1.0f,    // v7-v4-v3-v2 (bottom)

    1.0f, 1.0f, 0.5f, 1.0f, -0.0f, 0.5f, -0.0f, -0.0f, 0.5f, -0.0f, 1.0f, 0.5f };     // v5-v4-v7-v6 (back)

// Each block has a positional resolution of 7
const GLubyte BlockMesher::cubeVertices[72] = { 0, 7, 7, 0, 0, 7, 7, 0, 7, 7, 7, 7,  // v1-v2-v3-v0 (front)

    7, 7, 7, 7, 0, 7, 7, 0, 0, 7, 7, 0,     // v0-v3-v4-v5 (right)

    0, 7, 0, 0, 7, 7, 7, 7, 7, 7, 7, 0,    // v6-v1-v0-v5 (top)

    0, 7, 0, 0, 0, 0, 0, 0, 7, 0, 7, 7,   // v6-v7-v2-v1 (left)

    7, 0, 0, 7, 0, 7, 0, 0, 7, 0, 0, 0,  // v4-v3-v2-v7 (bottom)

    7, 7, 0, 7, 0, 0, 0, 0, 0, 0, 7, 0 };     // v5-v4-v7-v6 (back)

//0 = x, 1 = y, 2 = z
const int BlockMesher::cubeFaceAxis[6][2] = { { 0, 1 }, { 2, 1 }, { 0, 2 }, { 2, 1 }, { 0, 2 }, { 0, 1 } }; // front, right, top, left, bottom, back, for U and V respectively

const int BlockMesher::cubeFaceAxisSign[6][2] = { { 1, 1 }, { -1, 1 }, { 1, -1 }, { 1, 1 }, { -1, -1 }, { -1, 1 } }; // front, right, top, left, bottom, back, for U and V respectively

const GLfloat BlockMesher::liquidVertices[72] = { 0, 1.0f, 1.0f, 0, 0, 1.0f, 1.0f, 0, 1.0f, 1.0f, 1.0f, 1.0f,  // v1-v2-v3-v0 (front)

    1.0f, 1.0f, 1.0f, 1.0f, 0, 1.0f, 1.0f, 0, 0, 1.0f, 1.0f, 0,     // v0-v3-v4-v5 (right)

    0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0,    // v6-v1-v0-v5 (top)

    0, 1.0f, 0, 0, 0, 0, 0, 0, 1.0f, 0, 1.0f, 1.0f,   // v6-v1.0f-v2-v1 (left)

    1.0f, 0, 0, 1.0f, 0, 1.0f, 0, 0, 1.0f, 0, 0, 0,  // v4-v3-v2-v1.0f (bottom)

    1.0f, 1.0f, 0, 1.0f, 0, 0, 0, 0, 0, 0, 1.0f, 0 };     // v5-v4-v1.0f-v6 (back)


const float wyOff = 0.9999f;
const GLfloat BlockMesher::waterCubeVertices[72] = { 0.0f, wyOff, 1.000f, 0.0f, 0.0f, 1.000f, 1.000f, 0.0f, wyOff, 1.000f, wyOff, 1.000f,  // v1-v2-v3-v0 (front)

    1.000f, wyOff, 1.000f, 1.000f, 0.0f, 1.000f, 1.000f, 0.0f, 0.0f, 1.000f, wyOff, 0.0f,     // v0-v3-v4-v5 (right)

    0.0f, wyOff, 0.0f, 0.0f, wyOff, 1.000f, 1.000f, wyOff, 1.000f, 1.000f, wyOff, 0.0f,    // v6-v1-v0-v5 (top)

    0.0f, wyOff, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.000f, 0.0f, wyOff, 1.000f,   // v6-v7-v2-v1 (left)

    0.0f, 0.0f, 0.0f, 1.000f, 0.0f, 0.0f, 1.000f, 0.0f, 1.000f, 0.0f, 0.0f, 1.000f,    // v7-v4-v3-v2 (bottom)

    1.000f, wyOff, 0.0f, 1.000f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, wyOff, 0.0f };     // v5-v4-v7-v6 (back)


const GLbyte BlockMesher::cubeNormals[72] = { 0, 0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 127,  // v1-v2-v3-v0 (front)

    127, 0, 0, 127, 0, 0, 127, 0, 0, 127, 0, 0,     // v0-v3-v4-v5 (right)

    0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 127, 0,    // v6-v1-v0-v5 (top)

    -127, 0, 0, -127, 0, 0, -127, 0, 0, -127, 0, 0,   // v6-v7-v2-v1 (left)

    0, -127, 0, 0, -127, 0, 0, -127, 0, 0, -127, 0,    // v4-v3-v2-v7  (bottom)

    0, 0, -127, 0, 0, -127, 0, 0, -127, 0, 0, -127 };     // v5-v4-v7-v6 (back)

//For flora, normal is strait up
const GLbyte BlockMesher::floraNormals[72] = { 0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 127, 0,
    0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 127, 0,
    0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 127, 0,
    0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 127, 0,
    0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 127, 0,
    0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 127, 0 };

//We use 4 meshes so that we can add variation to the flora meshes
const ui8 BlockMesher::floraVertices[NUM_FLORA_MESHES][36] = {
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

const ui8 BlockMesher::crossFloraVertices[NUM_CROSSFLORA_MESHES][24] = {
    {
        0, 7, 0, 0, 0, 0, 7, 0, 7, 7, 7, 7,
        0, 7, 7, 0, 0, 7, 7, 0, 0, 7, 7, 0 },
    {
        7, 7, 7, 7, 0, 7, 0, 0, 0, 0, 7, 0,
        7, 7, 0, 7, 0, 0, 0, 0, 7, 0, 7, 7 } };

const GLfloat BlockMesher::physicsBlockVertices[72] = { -0.499f, 0.499f, 0.499f, -0.499f, -0.499f, 0.499f, 0.499f, -0.499f, 0.499f, 0.499f, 0.499f, 0.499f,  // v1-v2-v3-v0 (front)

    0.499f, 0.499f, 0.499f, 0.499f, -0.499f, 0.499f, 0.499f, -0.499f, -0.499f, 0.499f, 0.499f, -0.499f,     // v0-v3-v4-v499 (right)

    -0.499f, 0.499f, -0.499f, -0.499f, 0.499f, 0.499f, 0.499f, 0.499f, 0.499f, 0.499f, 0.499f, -0.499f,    // v6-v1-v0-v499 (top)

    -0.499f, 0.499f, -0.499f, -0.499f, -0.499f, -0.499f, -0.499f, -0.499f, 0.499f, -0.499f, 0.499f, 0.499f,   // v6-v7-v2-v1 (left)

    -0.499f, -0.499f, -0.499f, 0.499f, -0.499f, -0.499f, 0.499f, -0.499f, 0.499f, -0.499f, -0.499f, 0.499f,    // v7-v4-v3-v2 (bottom)

    0.499f, 0.499f, -0.499f, 0.499f, -0.499f, -0.499f, -0.499f, -0.499f, -0.499f, -0.499f, 0.499f, -0.499f };     // v5-v4-v7-v6 (back)


void BlockMesher::makeFloraFace(BlockVertex *Verts, const ui8* positions, const i8* normals, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const GLubyte color[], const GLubyte overlayColor[], const ui8 sunlight, const ui8 lampColor[3], const BlockTexture& texInfo)
{

    //get the face index so we can determine the axis alignment
    int faceIndex = vertexOffset / 12;
    //Multiply the axis by the sign bit to get the correct offset
    GLubyte uOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][0]] * cubeFaceAxisSign[faceIndex][0]);
    GLubyte vOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][1]] * cubeFaceAxisSign[faceIndex][1]);

    // 7 per coord
    pos.x *= 7;
    pos.y *= 7;
    pos.z *= 7;

    //Blend type. The 6 LSBs are used to encode alpha blending, add/subtract, and multiplication factors.
    //They are used in the shader to determine how to blend.
    ubyte blendMode = 0x25; //0x25 = 00 10 01 01
    switch (texInfo.blendMode) {
        case BlendType::BLEND_TYPE_REPLACE:
            blendMode++; //Sets blendType to 00 01 01 10
            break;
        case BlendType::BLEND_TYPE_ADD:
            blendMode += 4; //Sets blendType to 00 01 10 01
            break;
        case BlendType::BLEND_TYPE_SUBTRACT:
            blendMode -= 4; //Sets blendType to 00 01 00 01
            break;
        case BlendType::BLEND_TYPE_MULTIPLY:
            blendMode -= 16; //Sets blendType to 00 01 01 01
            break;
    }
    Verts[vertexIndex].blendMode = blendMode;
    Verts[vertexIndex + 1].blendMode = blendMode;
    Verts[vertexIndex + 2].blendMode = blendMode;
    Verts[vertexIndex + 3].blendMode = blendMode;

    GLubyte texAtlas = (GLubyte)(textureIndex / 256);
    textureIndex %= 256;

    GLubyte overlayTexAtlas = (GLubyte)(overlayTextureIndex / 256);
    GLubyte overlayTex = (GLubyte)(overlayTextureIndex % 256);

    Verts[vertexIndex].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 1].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 1].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 2].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 2].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 3].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 3].textureHeight = (ubyte)texInfo.base.size.y;

    Verts[vertexIndex].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 1].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 1].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 2].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 2].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 3].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 3].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;

    Verts[vertexIndex].position.x = pos.x + positions[vertexOffset];
    Verts[vertexIndex].position.y = pos.y + positions[vertexOffset + 1];
    Verts[vertexIndex].position.z = pos.z + positions[vertexOffset + 2];
    Verts[vertexIndex + 1].position.x = pos.x + positions[vertexOffset + 3];
    Verts[vertexIndex + 1].position.y = pos.y + positions[vertexOffset + 4];
    Verts[vertexIndex + 1].position.z = pos.z + positions[vertexOffset + 5];
    Verts[vertexIndex + 2].position.x = pos.x + positions[vertexOffset + 6];
    Verts[vertexIndex + 2].position.y = pos.y + positions[vertexOffset + 7];
    Verts[vertexIndex + 2].position.z = pos.z + positions[vertexOffset + 8];
    Verts[vertexIndex + 3].position.x = pos.x + positions[vertexOffset + 9];
    Verts[vertexIndex + 3].position.y = pos.y + positions[vertexOffset + 10];
    Verts[vertexIndex + 3].position.z = pos.z + positions[vertexOffset + 11];

    Verts[vertexIndex].color[0] = (GLubyte)(color[0]);
    Verts[vertexIndex].color[1] = (GLubyte)(color[1]);
    Verts[vertexIndex].color[2] = (GLubyte)(color[2]);
    Verts[vertexIndex + 1].color[0] = (GLubyte)(color[0]);
    Verts[vertexIndex + 1].color[1] = (GLubyte)(color[1]);
    Verts[vertexIndex + 1].color[2] = (GLubyte)(color[2]);
    Verts[vertexIndex + 2].color[0] = (GLubyte)(color[0]);
    Verts[vertexIndex + 2].color[1] = (GLubyte)(color[1]);
    Verts[vertexIndex + 2].color[2] = (GLubyte)(color[2]);
    Verts[vertexIndex + 3].color[0] = (GLubyte)(color[0]);
    Verts[vertexIndex + 3].color[1] = (GLubyte)(color[1]);
    Verts[vertexIndex + 3].color[2] = (GLubyte)(color[2]);

    Verts[vertexIndex].overlayColor[0] = (GLubyte)(overlayColor[0]);
    Verts[vertexIndex].overlayColor[1] = (GLubyte)(overlayColor[1]);
    Verts[vertexIndex].overlayColor[2] = (GLubyte)(overlayColor[2]);
    Verts[vertexIndex + 1].overlayColor[0] = (GLubyte)(overlayColor[0]);
    Verts[vertexIndex + 1].overlayColor[1] = (GLubyte)(overlayColor[1]);
    Verts[vertexIndex + 1].overlayColor[2] = (GLubyte)(overlayColor[2]);
    Verts[vertexIndex + 2].overlayColor[0] = (GLubyte)(overlayColor[0]);
    Verts[vertexIndex + 2].overlayColor[1] = (GLubyte)(overlayColor[1]);
    Verts[vertexIndex + 2].overlayColor[2] = (GLubyte)(overlayColor[2]);
    Verts[vertexIndex + 3].overlayColor[0] = (GLubyte)(overlayColor[0]);
    Verts[vertexIndex + 3].overlayColor[1] = (GLubyte)(overlayColor[1]);
    Verts[vertexIndex + 3].overlayColor[2] = (GLubyte)(overlayColor[2]);

    Verts[vertexIndex].normal[0] = normals[vertexOffset];
    Verts[vertexIndex].normal[1] = normals[vertexOffset + 1];
    Verts[vertexIndex].normal[2] = normals[vertexOffset + 2];
    Verts[vertexIndex + 1].normal[0] = normals[vertexOffset + 3];
    Verts[vertexIndex + 1].normal[1] = normals[vertexOffset + 4];
    Verts[vertexIndex + 1].normal[2] = normals[vertexOffset + 5];
    Verts[vertexIndex + 2].normal[0] = normals[vertexOffset + 6];
    Verts[vertexIndex + 2].normal[1] = normals[vertexOffset + 7];
    Verts[vertexIndex + 2].normal[2] = normals[vertexOffset + 8];
    Verts[vertexIndex + 3].normal[0] = normals[vertexOffset + 9];
    Verts[vertexIndex + 3].normal[1] = normals[vertexOffset + 10];
    Verts[vertexIndex + 3].normal[2] = normals[vertexOffset + 11];

    Verts[vertexIndex].lampColor[0] = lampColor[0];
    Verts[vertexIndex].lampColor[1] = lampColor[1];
    Verts[vertexIndex].lampColor[2] = lampColor[2];
    Verts[vertexIndex + 1].lampColor[0] = lampColor[0];
    Verts[vertexIndex + 1].lampColor[1] = lampColor[1];
    Verts[vertexIndex + 1].lampColor[2] = lampColor[2];
    Verts[vertexIndex + 2].lampColor[0] = lampColor[0];
    Verts[vertexIndex + 2].lampColor[1] = lampColor[1];
    Verts[vertexIndex + 2].lampColor[2] = lampColor[2];
    Verts[vertexIndex + 3].lampColor[0] = lampColor[0];
    Verts[vertexIndex + 3].lampColor[1] = lampColor[1];
    Verts[vertexIndex + 3].lampColor[2] = lampColor[2];

    Verts[vertexIndex].sunlight = sunlight;
    Verts[vertexIndex + 1].sunlight = sunlight;
    Verts[vertexIndex + 2].sunlight = sunlight;
    Verts[vertexIndex + 3].sunlight = sunlight;

    Verts[vertexIndex].merge = 0;
    Verts[vertexIndex + 1].merge = 0;
    Verts[vertexIndex + 2].merge = 0;
    Verts[vertexIndex + 3].merge = 0;

    if (waveEffect == 2){
        Verts[vertexIndex].color[3] = 255;
        Verts[vertexIndex + 1].color[3] = 255;
        Verts[vertexIndex + 2].color[3] = 255;
        Verts[vertexIndex + 3].color[3] = 255;
    } else if (waveEffect == 1){
        Verts[vertexIndex].color[3] = 255;
        Verts[vertexIndex + 1].color[3] = 0;
        Verts[vertexIndex + 2].color[3] = 0;
        Verts[vertexIndex + 3].color[3] = 255;
    } else{
        Verts[vertexIndex].color[3] = 0;
        Verts[vertexIndex + 1].color[3] = 0;
        Verts[vertexIndex + 2].color[3] = 0;
        Verts[vertexIndex + 3].color[3] = 0;
    }

    Verts[vertexIndex].tex[0] = 128 + uOffset;
    Verts[vertexIndex].tex[1] = 129 + vOffset;
    Verts[vertexIndex + 1].tex[0] = 128 + uOffset;
    Verts[vertexIndex + 1].tex[1] = 128 + vOffset;
    Verts[vertexIndex + 2].tex[0] = 129 + uOffset;
    Verts[vertexIndex + 2].tex[1] = 128 + vOffset;
    Verts[vertexIndex + 3].tex[0] = 129 + uOffset;
    Verts[vertexIndex + 3].tex[1] = 129 + vOffset;

    // *********** Base Texture
    Verts[vertexIndex].textureIndex = (GLubyte)textureIndex;
    Verts[vertexIndex + 1].textureIndex = (GLubyte)textureIndex;
    Verts[vertexIndex + 2].textureIndex = (GLubyte)textureIndex;
    Verts[vertexIndex + 3].textureIndex = (GLubyte)textureIndex;

    Verts[vertexIndex].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 1].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 2].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 3].textureAtlas = (GLubyte)texAtlas;

    // *********** Overlay texture
    Verts[vertexIndex].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 1].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 2].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 3].overlayTextureIndex = (GLubyte)overlayTex;

    Verts[vertexIndex].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 1].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 2].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 3].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
}

void BlockMesher::makeCubeFace(BlockVertex *Verts, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const GLubyte color[], const GLubyte overlayColor[], GLfloat ambientOcclusion[], const BlockTexture& texInfo)
{

    //get the face index so we can determine the axis alignment
    int faceIndex = vertexOffset / 12;
    //Multiply the axis by the sign bit to get the correct offset
    GLubyte uOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][0]] * cubeFaceAxisSign[faceIndex][0]);
    GLubyte vOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][1]] * cubeFaceAxisSign[faceIndex][1]);

    // 7 per coord
    pos.x *= 7;
    pos.y *= 7;
    pos.z *= 7;

    //Blend type. The 6 LSBs are used to encode alpha blending, add/subtract, and multiplication factors.
    //They are used in the shader to determine how to blend.
    ubyte blendMode = 0x25; //0x25 = 00 10 01 01
    switch (texInfo.blendMode) {
        case BlendType::BLEND_TYPE_REPLACE:
            blendMode++; //Sets blendType to 00 01 01 10
            break;
        case BlendType::BLEND_TYPE_ADD:
            blendMode += 4; //Sets blendType to 00 01 10 01
            break;
        case BlendType::BLEND_TYPE_SUBTRACT:
            blendMode -= 4; //Sets blendType to 00 01 00 01
            break;
        case BlendType::BLEND_TYPE_MULTIPLY:
            blendMode -= 16; //Sets blendType to 00 01 01 01
            break;
    }
    Verts[vertexIndex].blendMode = blendMode;
    Verts[vertexIndex + 1].blendMode = blendMode;
    Verts[vertexIndex + 2].blendMode = blendMode;
    Verts[vertexIndex + 3].blendMode = blendMode;

    GLubyte texAtlas = (GLubyte)(textureIndex / 256);
    textureIndex %= 256;

    GLubyte overlayTexAtlas = (GLubyte)(overlayTextureIndex / 256);
    GLubyte overlayTex = (GLubyte)(overlayTextureIndex % 256);

    Verts[vertexIndex].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 1].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 1].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 2].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 2].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 3].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 3].textureHeight = (ubyte)texInfo.base.size.y;

    Verts[vertexIndex].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 1].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 1].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 2].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 2].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 3].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 3].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;

    Verts[vertexIndex].position.x = pos.x + cubeVertices[vertexOffset];
    Verts[vertexIndex].position.y = pos.y + cubeVertices[vertexOffset + 1];
    Verts[vertexIndex].position.z = pos.z + cubeVertices[vertexOffset + 2];
    Verts[vertexIndex + 1].position.x = pos.x + cubeVertices[vertexOffset + 3];
    Verts[vertexIndex + 1].position.y = pos.y + cubeVertices[vertexOffset + 4];
    Verts[vertexIndex + 1].position.z = pos.z + cubeVertices[vertexOffset + 5];
    Verts[vertexIndex + 2].position.x = pos.x + cubeVertices[vertexOffset + 6];
    Verts[vertexIndex + 2].position.y = pos.y + cubeVertices[vertexOffset + 7];
    Verts[vertexIndex + 2].position.z = pos.z + cubeVertices[vertexOffset + 8];
    Verts[vertexIndex + 3].position.x = pos.x + cubeVertices[vertexOffset + 9];
    Verts[vertexIndex + 3].position.y = pos.y + cubeVertices[vertexOffset + 10];
    Verts[vertexIndex + 3].position.z = pos.z + cubeVertices[vertexOffset + 11];

    Verts[vertexIndex].color[0] = (GLubyte)(color[0] * ambientOcclusion[0]);
    Verts[vertexIndex].color[1] = (GLubyte)(color[1] * ambientOcclusion[0]);
    Verts[vertexIndex].color[2] = (GLubyte)(color[2] * ambientOcclusion[0]);
    Verts[vertexIndex + 1].color[0] = (GLubyte)(color[0] * ambientOcclusion[1]);
    Verts[vertexIndex + 1].color[1] = (GLubyte)(color[1] * ambientOcclusion[1]);
    Verts[vertexIndex + 1].color[2] = (GLubyte)(color[2] * ambientOcclusion[1]);
    Verts[vertexIndex + 2].color[0] = (GLubyte)(color[0] * ambientOcclusion[2]);
    Verts[vertexIndex + 2].color[1] = (GLubyte)(color[1] * ambientOcclusion[2]);
    Verts[vertexIndex + 2].color[2] = (GLubyte)(color[2] * ambientOcclusion[2]);
    Verts[vertexIndex + 3].color[0] = (GLubyte)(color[0] * ambientOcclusion[3]);
    Verts[vertexIndex + 3].color[1] = (GLubyte)(color[1] * ambientOcclusion[3]);
    Verts[vertexIndex + 3].color[2] = (GLubyte)(color[2] * ambientOcclusion[3]);

    Verts[vertexIndex].overlayColor[0] = (GLubyte)(overlayColor[0] * ambientOcclusion[0]);
    Verts[vertexIndex].overlayColor[1] = (GLubyte)(overlayColor[1] * ambientOcclusion[0]);
    Verts[vertexIndex].overlayColor[2] = (GLubyte)(overlayColor[2] * ambientOcclusion[0]);
    Verts[vertexIndex + 1].overlayColor[0] = (GLubyte)(overlayColor[0] * ambientOcclusion[1]);
    Verts[vertexIndex + 1].overlayColor[1] = (GLubyte)(overlayColor[1] * ambientOcclusion[1]);
    Verts[vertexIndex + 1].overlayColor[2] = (GLubyte)(overlayColor[2] * ambientOcclusion[1]);
    Verts[vertexIndex + 2].overlayColor[0] = (GLubyte)(overlayColor[0] * ambientOcclusion[2]);
    Verts[vertexIndex + 2].overlayColor[1] = (GLubyte)(overlayColor[1] * ambientOcclusion[2]);
    Verts[vertexIndex + 2].overlayColor[2] = (GLubyte)(overlayColor[2] * ambientOcclusion[2]);
    Verts[vertexIndex + 3].overlayColor[0] = (GLubyte)(overlayColor[0] * ambientOcclusion[3]);
    Verts[vertexIndex + 3].overlayColor[1] = (GLubyte)(overlayColor[1] * ambientOcclusion[3]);
    Verts[vertexIndex + 3].overlayColor[2] = (GLubyte)(overlayColor[2] * ambientOcclusion[3]);

    Verts[vertexIndex].normal[0] = cubeNormals[vertexOffset];
    Verts[vertexIndex].normal[1] = cubeNormals[vertexOffset + 1];
    Verts[vertexIndex].normal[2] = cubeNormals[vertexOffset + 2];
    Verts[vertexIndex + 1].normal[0] = cubeNormals[vertexOffset + 3];
    Verts[vertexIndex + 1].normal[1] = cubeNormals[vertexOffset + 4];
    Verts[vertexIndex + 1].normal[2] = cubeNormals[vertexOffset + 5];
    Verts[vertexIndex + 2].normal[0] = cubeNormals[vertexOffset + 6];
    Verts[vertexIndex + 2].normal[1] = cubeNormals[vertexOffset + 7];
    Verts[vertexIndex + 2].normal[2] = cubeNormals[vertexOffset + 8];
    Verts[vertexIndex + 3].normal[0] = cubeNormals[vertexOffset + 9];
    Verts[vertexIndex + 3].normal[1] = cubeNormals[vertexOffset + 10];
    Verts[vertexIndex + 3].normal[2] = cubeNormals[vertexOffset + 11];

    Verts[vertexIndex].merge = 1;
    Verts[vertexIndex + 1].merge = 1;
    Verts[vertexIndex + 2].merge = 1;
    Verts[vertexIndex + 3].merge = 1;

    if (waveEffect == 2){
        Verts[vertexIndex].color[3] = 255;
        Verts[vertexIndex + 1].color[3] = 255;
        Verts[vertexIndex + 2].color[3] = 255;
        Verts[vertexIndex + 3].color[3] = 255;
    } else if (waveEffect == 1){
        Verts[vertexIndex].color[3] = 255;
        Verts[vertexIndex + 1].color[3] = 0;
        Verts[vertexIndex + 2].color[3] = 0;
        Verts[vertexIndex + 3].color[3] = 255;
    } else{
        Verts[vertexIndex].color[3] = 0;
        Verts[vertexIndex + 1].color[3] = 0;
        Verts[vertexIndex + 2].color[3] = 0;
        Verts[vertexIndex + 3].color[3] = 0;
    }

    Verts[vertexIndex].tex[0] = 128 + uOffset;
    Verts[vertexIndex].tex[1] = 129 + vOffset;
    Verts[vertexIndex + 1].tex[0] = 128 + uOffset;
    Verts[vertexIndex + 1].tex[1] = 128 + vOffset;
    Verts[vertexIndex + 2].tex[0] = 129 + uOffset;
    Verts[vertexIndex + 2].tex[1] = 128 + vOffset;
    Verts[vertexIndex + 3].tex[0] = 129 + uOffset;
    Verts[vertexIndex + 3].tex[1] = 129 + vOffset;

    // *********** Base Texture
    Verts[vertexIndex].textureIndex = (GLubyte)textureIndex;
    Verts[vertexIndex + 1].textureIndex = (GLubyte)textureIndex;
    Verts[vertexIndex + 2].textureIndex = (GLubyte)textureIndex;
    Verts[vertexIndex + 3].textureIndex = (GLubyte)textureIndex;

    Verts[vertexIndex].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 1].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 2].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 3].textureAtlas = (GLubyte)texAtlas;

    // *********** Overlay texture
    Verts[vertexIndex].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 1].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 2].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 3].overlayTextureIndex = (GLubyte)overlayTex;

    Verts[vertexIndex].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 1].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 2].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 3].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
}

void BlockMesher::setFaceLight(BlockVertex* Verts, int index, ui8 lampColor[3], ui8 sunlight) {
    Verts[index].lampColor[0] = lampColor[0];
    Verts[index].lampColor[1] = lampColor[1];
    Verts[index].lampColor[2] = lampColor[2];
    Verts[index + 1].lampColor[0] = lampColor[0];
    Verts[index + 1].lampColor[1] = lampColor[1];
    Verts[index + 1].lampColor[2] = lampColor[2];
    Verts[index + 2].lampColor[0] = lampColor[0];
    Verts[index + 2].lampColor[1] = lampColor[1];
    Verts[index + 2].lampColor[2] = lampColor[2];
    Verts[index + 3].lampColor[0] = lampColor[0];
    Verts[index + 3].lampColor[1] = lampColor[1];
    Verts[index + 3].lampColor[2] = lampColor[2];

    Verts[index].sunlight = sunlight;
    Verts[index + 1].sunlight = sunlight;
    Verts[index + 2].sunlight = sunlight;
    Verts[index + 3].sunlight = sunlight;
}

const GLubyte waterUVs[8] = { 0, 7, 0, 0, 7, 0, 7, 7 };

void BlockMesher::makeLiquidFace(std::vector<LiquidVertex>& verts, i32 index, ui8 uOff, ui8 vOff, ui8 light[2], ui8 color[3], ui8 textureUnit) {

    verts.resize(verts.size() + 4);
    verts[index].tex[0] = waterUVs[0] + uOff;
    verts[index].tex[1] = waterUVs[1] + vOff;
    verts[index + 1].tex[0] = waterUVs[2] + uOff;
    verts[index + 1].tex[1] = waterUVs[3] + vOff;
    verts[index + 2].tex[0] = waterUVs[4] + uOff;
    verts[index + 2].tex[1] = waterUVs[5] + vOff;
    verts[index + 3].tex[0] = waterUVs[6] + uOff;
    verts[index + 3].tex[1] = waterUVs[7] + vOff;

    verts[index].light = light[0];
    verts[index + 1].light = light[0];
    verts[index + 2].light = light[0];
    verts[index + 3].light = light[0];
    verts[index].sunlight = light[1];
    verts[index + 1].sunlight = light[1];
    verts[index + 2].sunlight = light[1];
    verts[index + 3].sunlight = light[1];

    verts[index].color[0] = color[0];
    verts[index].color[1] = color[1];
    verts[index].color[2] = color[2];
    verts[index + 1].color[0] = color[0];
    verts[index + 1].color[1] = color[1];
    verts[index + 1].color[2] = color[2];
    verts[index + 2].color[0] = color[0];
    verts[index + 2].color[1] = color[1];
    verts[index + 2].color[2] = color[2];
    verts[index + 3].color[0] = color[0];
    verts[index + 3].color[1] = color[1];
    verts[index + 3].color[2] = color[2];

    verts[index].textureUnit = (GLubyte)textureUnit;
    verts[index + 1].textureUnit = (GLubyte)textureUnit;
    verts[index + 2].textureUnit = (GLubyte)textureUnit;
    verts[index + 3].textureUnit = (GLubyte)textureUnit;
}

void BlockMesher::makePhysicsBlockFace(vector <PhysicsBlockVertex> &verts, const GLfloat *blockPositions, int vertexOffset, int &index, const BlockTexture& blockTexture)
{
    ui8 textureAtlas = (ui8)(blockTexture.base.textureIndex / 256);
    ui8 textureIndex = (ui8)(blockTexture.base.textureIndex % 256);

    ui8 overlayTextureAtlas = (ui8)(blockTexture.overlay.textureIndex / 256);
    ui8 overlayTextureIndex = (ui8)(blockTexture.overlay.textureIndex % 256);

    ui8 blendMode = 0x25; //0x25 = 00 10 01 01
    switch (blockTexture.blendMode) {
        case BlendType::BLEND_TYPE_REPLACE:
            blendMode++; //Sets blendType to 00 01 01 10
            break;
        case BlendType::BLEND_TYPE_ADD:
            blendMode += 4; //Sets blendType to 00 01 10 01
            break;
        case BlendType::BLEND_TYPE_SUBTRACT:
            blendMode -= 4; //Sets blendType to 00 01 00 01
            break;
        case BlendType::BLEND_TYPE_MULTIPLY:
            blendMode -= 16; //Sets blendType to 00 01 01 01
            break;
    }
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

    verts[index].position[0] = cubeVertices[vertexOffset];
    verts[index].position[1] = cubeVertices[vertexOffset + 1];
    verts[index].position[2] = cubeVertices[vertexOffset + 2];
    verts[index + 1].position[0] = cubeVertices[vertexOffset + 3];
    verts[index + 1].position[1] = cubeVertices[vertexOffset + 4];
    verts[index + 1].position[2] = cubeVertices[vertexOffset + 5];
    verts[index + 2].position[0] = cubeVertices[vertexOffset + 6];
    verts[index + 2].position[1] = cubeVertices[vertexOffset + 7];
    verts[index + 2].position[2] = cubeVertices[vertexOffset + 8];
    verts[index + 3].position[0] = cubeVertices[vertexOffset + 6];
    verts[index + 3].position[1] = cubeVertices[vertexOffset + 7];
    verts[index + 3].position[2] = cubeVertices[vertexOffset + 8];
    verts[index + 4].position[0] = cubeVertices[vertexOffset + 9];
    verts[index + 4].position[1] = cubeVertices[vertexOffset + 10];
    verts[index + 4].position[2] = cubeVertices[vertexOffset + 11];
    verts[index + 5].position[0] = cubeVertices[vertexOffset];
    verts[index + 5].position[1] = cubeVertices[vertexOffset + 1];
    verts[index + 5].position[2] = cubeVertices[vertexOffset + 2];

    verts[index].tex[0] = 128;
    verts[index].tex[1] = 129;
    verts[index + 1].tex[0] = 128;
    verts[index + 1].tex[1] = 128;
    verts[index + 2].tex[0] = 129;
    verts[index + 2].tex[1] = 128;
    verts[index + 3].tex[0] = 129;
    verts[index + 3].tex[1] = 128;
    verts[index + 4].tex[0] = 129;
    verts[index + 4].tex[1] = 129;
    verts[index + 5].tex[0] = 128;
    verts[index + 5].tex[1] = 129;

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
    verts[index + 5].overlayTextureIndex = overlayTextureIndex;

}
