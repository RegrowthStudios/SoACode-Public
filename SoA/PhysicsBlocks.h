#pragma once
#include <deque>
#include <queue>

#include "ImageLoading.h"
#include "WorldStructs.h"

struct ChunkSlot;

struct PhysicsBlockVertex {
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

struct PhysicsBlockMesh {
public:
    PhysicsBlockMesh() : vboID(0), positionLightBufferID(0), vecIndex(-1), numBlocks(0) {}

    ui32 vboID;
    ui32 positionLightBufferID;
    i32 numBlocks;
    i32 vecIndex;
    i32 bX, bY, bZ;
};

struct PhysicsBlockMeshMessage {
    PhysicsBlockMeshMessage() : mesh(nullptr), numBlocks(0), bX(0), bY(0), bZ(0) {}

    PhysicsBlockMesh* mesh;
    std::vector<PhysicsBlockPosLight> posLight;
    std::vector<PhysicsBlockVertex> verts;
    i32 numBlocks;
    i32 bX, bY, bZ;
};

class PhysicsBlock {
public:
    PhysicsBlock(const f64v3& pos, i32 BlockType, i32 ydiff, f32v2& dir, f32v3 extraForce);
    bool update(const std::deque< std::deque< std::deque<ChunkSlot*> > >& chunkList, f64 X, f64 Y, f64 Z);

    f64v3 position;
    f32v3 velocity;
    i32 blockType;
    f32 grav, fric;
    ui8 done; //we defer removal by a frame or two
    ui8 light[2];
};

class PhysicsBlockBatch {
public:
    PhysicsBlockBatch(i32 BlockType, ui8 temp, ui8 rain);
    ~PhysicsBlockBatch();

    static void draw(PhysicsBlockMesh* pbm, const f64v3& PlayerPos, f32m4& VP);
    bool update(const std::deque< std::deque< std::deque<ChunkSlot*> > >& chunkList, f64 wX, f64 wY, f64 wZ);
    void addBlock(const f64v3& pos, i32 ydiff, f32v2& dir, f32v3 extraForce);

    std::vector<PhysicsBlock> physicsBlocks;
    i32 blockType;

protected:
    f64 _bX, _bY, _bZ;
    PhysicsBlockMesh* _mesh;
    i32 _numBlocks;
};
