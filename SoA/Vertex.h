///
/// Vertex.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 21 Jun 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Vertex definitions for SoA
///

#pragma once

#ifndef Vertex_h__
#define Vertex_h__

#include "Vorb/types.h"

class ColorVertex {
public:
    f32v3 position;
    ubyte color[4];
};

const ui8 MESH_FLAG_ACTIVE = 0x1;
const ui8 MESH_FLAG_MERGE_RIGHT = 0x2;
const ui8 MESH_FLAG_MERGE_FRONT = 0x4;

// Describes atlas positioning for a BlockVertex
struct AtlasTexturePosition {
    struct {
        ui8 atlas;
        ui8 index;
    } base;
    struct {
        ui8 atlas;
        ui8 index;
    } overlay;

    bool operator==(const AtlasTexturePosition& rhs) const {
        // Assumes 32 bit struct.
        return *((const ui32*)this) == *((const ui32*)&rhs);
    }
};
static_assert(sizeof(AtlasTexturePosition) == sizeof(ui32), "AtlasTexturePosition compare will fail.");

// Size: 32 Bytes
struct BlockVertex {

    BlockVertex() {}
    //need deconstructor because of the non-trivial union
    ~BlockVertex() { position.ui8v3::~ui8v3(); }

    union {
        struct {
            ui8 x;
            ui8 y;
            ui8 z;
        };
        ui8v3 position;
    };
    ui8 face;

    ui8v2 tex;
    ui8 animationLength;
    ui8 blendMode;

    AtlasTexturePosition texturePosition;
    AtlasTexturePosition normTexturePosition;
    AtlasTexturePosition dispTexturePosition;
    
    ui8v2 textureDims;
    ui8v2 overlayTextureDims;

    color3 color;
    ui8 mesherFlags;

    color3 overlayColor;
    ui8 padding;

    // This isn't a full comparison. Its just for greedy mesh comparison so its lightweight.
    bool operator==(const BlockVertex& rhs) const {
        return (color == rhs.color && overlayColor == rhs.overlayColor &&
                texturePosition == rhs.texturePosition);
    }
};
static_assert(sizeof(BlockVertex) == 32, "Size of BlockVertex is not 32");

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
