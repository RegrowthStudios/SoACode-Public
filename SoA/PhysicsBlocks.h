#pragma once
#include <deque>
#include <queue>

#include "GLProgram.h"
#include "WorldStructs.h"
#include "OpenGLStructs.h"

class Chunk;

class PhysicsBlockMesh {
public:
    PhysicsBlockMesh() : vboID(0), positionLightBufferID(0), vaoID(0), vecIndex(-1), numBlocks(0) {}

    void createVao(const vg::GLProgram* glProgram);

    ui32 vboID;
    ui32 positionLightBufferID;
    ui32 vaoID;
    i32 numBlocks;
    i32 vecIndex;
    i32 bX, bY, bZ;
};

class PhysicsBlockMeshMessage {
public:
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
    bool update();

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

    static void draw(PhysicsBlockMesh* pbm, const vg::GLProgram* program, const f64v3& PlayerPos, const f32m4& VP);
    bool update();
    void addBlock(const f64v3& pos, i32 ydiff, f32v2& dir, f32v3 extraForce);

    std::vector<PhysicsBlock> physicsBlocks;
    i32 blockType;

protected:
    f64 _bX, _bY, _bZ;
    PhysicsBlockMesh* _mesh;
    i32 _numBlocks;
};
