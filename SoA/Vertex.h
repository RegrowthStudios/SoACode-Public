///
/// Vertex.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 21 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Vertex definitions for SoA
///

#pragma once

#ifndef Vertex_h__
#define Vertex_h__

class ColorVertex {
public:
    f32v3 position;
    ubyte color[4];
};

// Size: 32 Bytes
struct BlockVertex {
    union {
        struct {  //3 bytes  << 1
            ui8 x;
            ui8 y;
            ui8 z;
        };
        UNIONIZE(ui8v3 position);
    };
    ui8 textureType; //4   

    //   10 = overlay 
    //  100 = animated overlay 
    // 1000 = normal map
    //10000 = specular map
    UNIONIZE(ui8v2 tex); //6 
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

    // Have to use struct since color3 has constructor
    UNIONIZE(color3 color); // 19
    ui8 waveEffect; //20

    UNIONIZE(color3 overlayColor); //23
    ui8 pad2; //24

    UNIONIZE(color3 lampColor); //27
    ui8 sunlight; //28

    UNIONIZE(ui8v3 normal); //31
    bool isActive; //32 // Helpful during the meshing process.

    // This isn't a full comparison. Its just for greedy mesh comparison so its lightweight.
    bool operator==(const BlockVertex& rhs) const {
        return (color == rhs.color && overlayColor == rhs.overlayColor &&
                textureAtlas == rhs.textureAtlas && textureIndex == rhs.textureIndex);
    }
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

#endif // Vertex_h__
