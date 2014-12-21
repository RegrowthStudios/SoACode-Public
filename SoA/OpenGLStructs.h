#pragma once
#include "Constants.h"
#include "Keg.h"

// 4324 53
class ColorVertex {
public:
    f32v3 position;
    ubyte color[4];
};

enum class BlendType {
    ALPHA,
    ADD,
    SUBTRACT,
    MULTIPLY
};
KEG_ENUM_DECL(BlendType);

// Size: 32 Bytes
class BlockVertex {
public:
    class vPosition {  //3 bytes  << 1
    public:
        ubyte x;
        ubyte y;
        ubyte z;
    } position;
    ui8 textureType; //4   

    //   10 = overlay 
    //  100 = animated overlay 
    // 1000 = normal map
    //10000 = specular map
    ui8 tex[2]; //6 
    ui8 animationLength; //7
    ui8 blendMode; //8

    ui8 textureAtlas; //9
    ui8 overlayTextureAtlas; //10
    ui8 textureIndex; //11
    ui8 overlayTextureIndex; //12

    ui8 textureWidth; //13
    ui8 textureHeight; //14
    ui8 overlayTextureWidth; //15;
    ui8 overlayTextureHeight; //16

    ColorRGB8 color; //19
    ui8 waveEffect; //20
    ColorRGB8 overlayColor; //23
    ui8 pad2; //24

    ColorRGB8 lampColor; //27
    ui8 sunlight; //28

    ui8 normal[3]; //31
    ui8 merge; //32
};

class LiquidVertex {
public:
    // TODO: x and z can be bytes?
    f32v3 position; //12
    ui8 tex[2]; //14
    ui8 textureUnit; //15 
    ui8 textureIndex; //16
    ColorRGBA8 color; //20
    ColorRGB8 lampColor; //23
    ui8 sunlight; //24
};

class TerrainVertex {
public:
    f32v3 position; //12
    ColorRGB8 color; //15
    ui8 padding; //16
    ui8v2 texCoords; //18
    ui8 padding2[2]; //20
};

class PhysicsBlockVertex {
public:
    ui8 position[3]; //3
    ui8 blendMode; //4
    ui8 tex[2]; //6
    ui8 pad1[2]; //8

    ui8 textureAtlas; //9
    ui8 overlayTextureAtlas; //10
    ui8 textureIndex; //11
    ui8 overlayTextureIndex; //12

    ui8 textureWidth; //13
    ui8 textureHeight; //14
    ui8 overlayTextureWidth; //15
    ui8 overlayTextureHeight; //16

    i8 normal[3]; //19
    i8 pad2; //20
};

class Face {
public:
    Face(i32 facen, i32 f1, i32 f2, i32 f3, i32 t1, i32 t2, i32 t3, i32 m) : facenum(facen) {
        vertexs[0] = f1;
        vertexs[1] = f2;
        vertexs[2] = f3;
        texcoord[0] = t1;
        texcoord[1] = t2;
        texcoord[2] = t3;
        mat = m;
        isQuad = 0;
    }
    Face(i32 facen, i32 f1, i32 f2, i32 f3, i32 f4, i32 t1, i32 t2, i32 t3, i32 t4, i32 m) : facenum(facen) {
        vertexs[0] = f1;
        vertexs[1] = f2;
        vertexs[2] = f3;
        vertexs[3] = f4;
        texcoord[0] = t1;
        texcoord[1] = t2;
        texcoord[2] = t3;
        texcoord[3] = t4;
        mat = m;
        isQuad = 1;
    }

    i32 facenum;
    bool isQuad;
    i32 vertexs[4];
    i32 texcoord[4];
    i32 mat;
};

class Material {
public:
    Material(const char* na, f32 a, f32 n, f32 ni2, f32* d, f32* am,
        f32* s, i32 il, i32 t);

    std::string name;
    f32 alpha, ns, ni;
    f32 dif[3], amb[3], spec[3];
    i32 illum;
    i32 texture;
};

class TexCoord {
public:
    TexCoord(f32 a, f32 b);

    f32 u, v;
};
