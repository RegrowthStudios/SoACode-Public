#pragma once
#include <queue>

#include "Vertex.h"

class Chunk;
class ChunkManager;
class PhysicsBlockBatch;

class ExplosionNode {
public:
    ExplosionNode(f64v3& pos, i32 blocktype) : ppos(pos), val(blocktype) {}

    f64v3 ppos;
    i32 val;
};

class ExplosionInfo {
public:
    ExplosionInfo(f64v3& Pos, f32 Force) : pos(Pos), force(Force) {}

    f64v3 pos;
    f32 force;
};

class PressureNode {
public:
    PressureNode(Chunk* Ch, ui16 C, f32 Force, ExplosionInfo* en) : ch(Ch), c(C), force(Force), explosionInfo(en) {}

    Chunk* ch;
    ExplosionInfo* explosionInfo;
    ui16 c;
    f32 force; //todo: make into a ui16 to save 2 bytes
};

class VisitedNode {
public:
    VisitedNode(Chunk* Ch, ui16 C) : ch(Ch), c(C) {}

    Chunk* ch;
    ui16 c;
};

class FallingCheckNode {
public:
    FallingCheckNode(Chunk* chk, ui16 C, i8 expNx = 0, i8 expNy = 0, i8 expNz = 0, ui8 expDist = 0);

    Chunk* ch;
    ui16 c;
    i8 explosionDir[3]; //direction to explosion
    ui8 explosionDist; //distance in meters from explosion
};

class FallingNode {
public:
    inline void setValues(ui16 C, Chunk* Ch, i32 nsw);

    i32 nSinceWood;
    Chunk* ch;
    ui16 c;
    ui16 blockType;
};


const i32 MAX_SEARCH_LENGTH = 100000;
const i32 F_NODES_MAX_SIZE = MAX_SEARCH_LENGTH * 30;

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();

    // TODO(Ben): This is temporary
    void setChunkManager(ChunkManager* chunkManager) { m_chunkManager = chunkManager; }

    void clearAll();
    void update(const f64v3& viewDir);

    // TODO: This is not threadsafe
    void addFallingCheckNode(FallingCheckNode& fallingCheckNode) {
        //_fallingCheckNodes.push_back(fallingCheckNode);
    }
    void addExplosion(ExplosionNode& explosionNode) {
        _deferredExplosions.push(explosionNode);
    }

    void addPhysicsBlock(const f64v3& pos, i32 blockType);
    void addPhysicsBlock(const f64v3& pos, i32 blockType, i32 ydiff, f32v2& dir, f32v3 explosionDir, ui8 temp, ui8 rain);
private:
    // Explosions
    void explosion(const f64v3& pos, i32 numRays, f64 power, f64 loss);
    void pressureExplosion(f64v3& pos);
    void pressureUpdate(PressureNode& pn);
    inline void explosionRay(ChunkManager* chunkManager, const f64v3& pos, f32 force, f32 powerLoss, const f32v3& dir);
    void performExplosions();

    // Floating detection
    void detectFloatingBlocks(const f64v3& viewDir);
    void detectFloating(FallingCheckNode* fallNode, i32& start, const f64v3& viewDirection, i32 searchLength);
    void restoreDetectFloatingBlocks(i32& size);

    std::vector<FallingCheckNode> _fallingCheckNodes;
    FallingNode* _fnodes;

    std::vector<glm::vec3> _precomputedExplosionDirs;
    std::queue<ExplosionNode> _deferredExplosions;
    std::vector<PressureNode> _pressureNodes;
    std::vector<VisitedNode> _visitedNodes;
    std::vector<ExplosionInfo*> _explosionsList;

    // Physics Blocks
    PhysicsBlockBatch* _physicsBlockBatches[65536]; //for all possible block IDs.
    std::vector<PhysicsBlockBatch*> _activePhysicsBlockBatches;

    ChunkManager* m_chunkManager = nullptr;
};