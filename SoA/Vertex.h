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

const ui8 MESH_FLAG_ACTIVE = 0x1;
const ui8 MESH_FLAG_MERGE_RIGHT = 0x2;
const ui8 MESH_FLAG_MERGE_FRONT = 0x4;

// Size: 32 Bytes
struct BlockVertex {
    union {
        struct {
            ui8 x;
            ui8 y;
            ui8 z;
        };
        UNIONIZE(ui8v3 position);
    };
    ui8 face;

    UNIONIZE(ui8v2 tex);
    ui8 animationLength;
    ui8 blendMode;

    ui8 textureAtlas;
    ui8 overlayTextureAtlas;
    ui8 textureIndex;
    ui8 overlayTextureIndex;
    
    ui8 normAtlas;
    ui8 overlayNormAtlas;
    ui8 dispAtlas;
    ui8 overlayDispAtlas;

    ui8 normIndex;
    ui8 overlayNormIndex;
    ui8 dispIndex;
    ui8 overlayDispIndex;

    ui8 textureWidth;
    ui8 textureHeight;
    ui8 overlayTextureWidth;
    ui8 overlayTextureHeight;

    UNIONIZE(color3 color);
    ui8 mesherFlags;

    UNIONIZE(color3 overlayColor);
    ui8 padding;

    // This isn't a full comparison. Its just for greedy mesh comparison so its lightweight.
    bool operator==(const BlockVertex& rhs) const {
        return (color == rhs.color && overlayColor == rhs.overlayColor &&
                textureAtlas == rhs.textureAtlas && textureIndex == rhs.textureIndex &&
                overlayTextureAtlas == rhs.overlayTextureAtlas && overlayTextureIndex == rhs.overlayTextureIndex);
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
