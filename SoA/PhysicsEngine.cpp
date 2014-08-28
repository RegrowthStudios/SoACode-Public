#include "stdafx.h"
#include "PhysicsEngine.h"

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "ChunkUpdater.h"
#include "GameManager.h"
#include "Particles.h"
#include "PhysicsBlocks.h"
#include "Sound.h"
#include "VoxelLightEngine.h"
#include "VoxelRay.h"

#include "utils.h"

PhysicsEngine::PhysicsEngine() {

    //precompute explosion dirs so we dont need to call rand 6000 times per explosion.
    _precomputedExplosionDirs.resize(20000);
    _fallingCheckNodes.reserve(1000);
    glm::vec3 dir;
    srand(55);
    for (size_t i = 0; i < _precomputedExplosionDirs.size(); i++){
        dir.x = (rand()) / (double)RAND_MAX*2.0 - 1.0;
        dir.y = (rand()) / (double)RAND_MAX*2.0 - 1.0;
        dir.z = (rand()) / (double)RAND_MAX*2.0 - 1.0;
        if (dir.x == 0 && dir.y == 0 && dir.z == 0) dir.x = 1.0f;
        _precomputedExplosionDirs[i] = glm::normalize(dir);
    }

    _fnodes = new FallingNode[F_NODES_MAX_SIZE + 1];

    for (int i = 0; i < 65536; i++){
        _physicsBlockBatches[i] = NULL;
    }
}

PhysicsEngine::~PhysicsEngine() {
    clearAll();
}

void PhysicsEngine::clearAll() {
    queue <ExplosionNode>().swap(_deferredExplosions);

    for (Uint32 i = 0; i < _activePhysicsBlockBatches.size(); i++){
        _physicsBlockBatches[_activePhysicsBlockBatches[i]->blockType] = NULL;
        delete _activePhysicsBlockBatches[i];
    }
    _activePhysicsBlockBatches.clear();
}

void PhysicsEngine::update(const glm::dvec3 &viewDir) {

    performExplosions();
    detectFloatingBlocks(viewDir);

    int X = GameManager::chunkManager->cornerPosition.x;
    int Y = GameManager::chunkManager->cornerPosition.y;
    int Z = GameManager::chunkManager->cornerPosition.z;

    const deque <deque < deque<ChunkSlot *> > > &chunkList = GameManager::getChunkList();

    for (Uint32 i = 0; i < _activePhysicsBlockBatches.size();){
        if (_activePhysicsBlockBatches[i]->update(chunkList, (double)X, (double)Y, (double)Z)){
            _physicsBlockBatches[_activePhysicsBlockBatches[i]->blockType] = NULL;
            delete _activePhysicsBlockBatches[i];
            _activePhysicsBlockBatches[i] = _activePhysicsBlockBatches.back();
            _activePhysicsBlockBatches.pop_back();
        } else{
            i++;
        }
    }
}

void PhysicsEngine::explosion(const glm::dvec3 &pos, int numRays, double power, double loss)
{
    //pressureExplosion(pos);
    //return; 

    static int k = 0;
    if (numRays > _precomputedExplosionDirs.size()) numRays = _precomputedExplosionDirs.size();

    //particleEngine.AddAnimatedParticles(1, pos, PARTICLE_EXPLOSION, 0, 0, 0, glm::vec4(255.0, 255.0f, 255.0f, 255.0), 7, 0, 1.0f, 160, glm::vec3(0.0f, 0.05f, 0.0f)); //explosion effect
    GameManager::soundEngine->PlayExistingSound("Explosion", 0, 0.65f, 0, glm::dvec3(pos.x, pos.y, pos.z));

    if (k + numRays >= _precomputedExplosionDirs.size()){ //prevent overflow and numerous if checks
        k = 0;
    }

    for (int i = 0; i < numRays; i++){
        explosionRay(pos, power, loss, _precomputedExplosionDirs[k++]);
    }
}

void PhysicsEngine::pressureExplosion(glm::dvec3 &pos)
{
  /*  int startx = pos.x;
    int starty = pos.y;
    int startz = pos.z;

    const deque <deque< deque<ChunkSlot *> > > &chunkList = GameManager::chunkManager->getChunkList();

    int c;
    short x, y, z;
    int gridRelY, gridRelX, gridRelZ;
    int bx, by, bz, px = -1, py = -1, pz = -1;
    Chunk *ch;

    gridRelX = (pos.x - GameManager::chunkManager->X);
    gridRelY = (-(pos.y - GameManager::chunkManager->Y));
    gridRelZ = (pos.z - GameManager::chunkManager->Z);

    x = (gridRelX / chunkWidth);
    y = (gridRelY / chunkWidth + 1);
    z = (gridRelZ / chunkWidth);

    if (x != px || y != py || z != pz){

        px = x;
        py = y;
        pz = z;

        if (x < 0 || y < 0 || z < 0 || x >= csGridWidth || y >= csGridWidth || z >= csGridWidth) return;

        ch = chunkList[y][z][x]->chunk;

        if ((!ch) || ch->isAccessible == 0) return;
    }

    bx = gridRelX % chunkWidth;
    by = 31 - (gridRelY % chunkWidth);
    bz = gridRelZ % chunkWidth;

    c = bx + by*chunkLayer + bz*chunkWidth;

    GLuint sticks = SDL_GetTicks();

    pressureNodes.reserve(300000);


    explosionsList.push_back(new ExplosionInfo(pos, 10000.0f));
    pressureNodes.push_back(PressureNode(ch, c, 0, explosionsList.back()));
    int nn = 0;
    for (size_t j = 0; j < pressureNodes.size(); j++){
        nn++;
        pressureUpdate(pressureNodes[j]);

    }
    pressureNodes.clear();
    for (size_t j = 0; j < visitedNodes.size(); j++){
        visitedNodes[j].ch->data[visitedNodes[j].c] = NONE;
    }
    visitedNodes.clear();
    cout << "SIZE: " << nn << " Time: " << SDL_GetTicks() - sticks << endl; */
}

void PhysicsEngine::pressureUpdate(PressureNode &pn)
{
    //	cout << pn.force << " ";
    int c = pn.c;
    int x = c%CHUNK_WIDTH;
    int y = c / CHUNK_LAYER;
    int z = (c%CHUNK_LAYER) / CHUNK_WIDTH;
    Chunk *ch = pn.ch;
    ExplosionInfo *explosionInfo = pn.explosionInfo;
    int c2;
    int blockID;
    float force = pn.force;

    _visitedNodes.push_back(VisitedNode(ch, c));

    glm::dvec3 worldSpaceCoords(ch->position * CHUNK_WIDTH + glm::ivec3(x, y, z));

    double distance = glm::length(worldSpaceCoords - explosionInfo->pos) - 30;

    float currForce;
    if (distance > 0){
        currForce = explosionInfo->force / (distance*distance);
    } else{
        currForce = explosionInfo->force;
    }

    force -= 1.0f;
    currForce += force;
    if (currForce <= 0.0f) return;

    //LEFT
    if (x > 0){
        c2 = c - 1;
        blockID = ch->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch, c2, 1);

            ch->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    } else if (ch->left && ch->left->isAccessible){
        c2 = c + CHUNK_WIDTH - 1;
        blockID = ch->left->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch->left, c2, 1);

            ch->left->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->left, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->left, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    }

    //RIGHT
    if (x < 31){
        c2 = c + 1;
        blockID = ch->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch, c2, 1);

            ch->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    } else if (ch->right && ch->right->isAccessible){
        c2 = c - CHUNK_WIDTH + 1;
        blockID = ch->right->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch->right, c2, 1);

            ch->right->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->right, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->right, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    }

    //BOTTOM
    if (y > 0){
        c2 = c - CHUNK_LAYER;
        blockID = ch->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch, c2, 1);

            ch->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    } else if (ch->bottom && ch->bottom->isAccessible){
        c2 = c + CHUNK_SIZE - CHUNK_LAYER;
        blockID = ch->bottom->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch->bottom, c2, 1);

            ch->bottom->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->bottom, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->bottom, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    }

    //TOP
    if (y < 31){
        c2 = c + CHUNK_LAYER;
        blockID = ch->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch, c2, 1);

            ch->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    } else if (ch->top && ch->top->isAccessible){
        c2 = c - CHUNK_SIZE + CHUNK_LAYER;
        blockID = ch->top->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch->top, c2, 1);

            ch->top->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->top, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->top, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    }

    //BACK
    if (z > 0){
        c2 = c - CHUNK_WIDTH;
        blockID = ch->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch, c2, 1);

            ch->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    } else if (ch->back && ch->back->isAccessible){
        c2 = c + CHUNK_LAYER - CHUNK_WIDTH;
        blockID = ch->back->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch->back, c2, 1);

            ch->back->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->back, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->back, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    }

    //FRONT
    if (z < 31){
        c2 = c + CHUNK_WIDTH;
        blockID = ch->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch, c2, 1);

            ch->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    } else if (ch->front && ch->front->isAccessible){
        c2 = c - CHUNK_LAYER + CHUNK_WIDTH;
        blockID = ch->front->getBlockID(c2);
        if (blockID != VISITED_NODE){
            Block &block = GETBLOCK(blockID);
            if (blockID != NONE) ChunkUpdater::removeBlock(ch->front, c2, 1);

            ch->front->setBlockID(c2, VISITED_NODE);
            if (block.explosivePower != 0){
                ExplosionInfo *newExplosion = new ExplosionInfo(worldSpaceCoords, currForce + 10000.0f);
                _explosionsList.push_back(newExplosion);
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->front, c2, force - block.explosionResistance, newExplosion));
            } else{
                if (currForce > block.explosionResistance) _pressureNodes.push_back(PressureNode(ch->front, c2, force - block.explosionResistance, explosionInfo));
            }
        }
    }

    //water pressure attempt (not working)
    /*if (ch->GetLeftBlock(c, x, &c2, &owner) != VISITED_NODE) pressureNodes.push(PressureNode(owner, c2, force, 0));

    if (ch->GetRightBlock(c, x, &c2, &owner) != VISITED_NODE) pressureNodes.push(PressureNode(owner, c2, force, 1));

    if (ch->GetTopBlock(c, y, &c2, &owner) != VISITED_NODE) pressureNodes.push(PressureNode(owner, c2, force, 2));

    if (ch->GetBottomBlock(c, y, &c2, &owner) != VISITED_NODE) pressureNodes.push(PressureNode(owner, c2, force, 3));

    if (ch->GetFrontBlock(c, z, &c2, &owner) != VISITED_NODE) pressureNodes.push(PressureNode(owner, c2, force, 4));

    if (ch->GetBackBlock(c, z, &c2, &owner) != VISITED_NODE) pressureNodes.push(PressureNode(owner, c2, force, 5));*/


}

void PhysicsEngine::explosionRay(const glm::dvec3 &pos, float force, float powerLoss, const glm::vec3 &dir) {

    const deque< deque< deque<ChunkSlot*> > >& chunkList = GameManager::chunkManager->getChunkList();
    f32v3 relativePos = f32v3(pos - f64v3(GameManager::chunkManager->cornerPosition));
    VoxelRay vr(relativePos, dir);
    i32v3 loc = vr.getNextVoxelPosition();
    i32v3 chunkPos = loc / CHUNK_WIDTH;
    f32v3 rayToBlock;

    const float maxDistance = 10;

    // Loop Traversal
    while (vr.getDistanceTraversed() < maxDistance) {
        // Make Sure Ray Stays In Bounds
        if (!GameManager::chunkManager->isChunkPositionInBounds(chunkPos)) return;

        Chunk* currentChunk = chunkList[chunkPos.y][chunkPos.z][chunkPos.x]->chunk;
        if (currentChunk) {
            // Calculate Voxel Index
            i32 voxelIndex = loc.x % CHUNK_WIDTH + (loc.y % CHUNK_WIDTH) * CHUNK_LAYER + (loc.z % CHUNK_WIDTH) * CHUNK_WIDTH;

            // Get Block ID
            i32 blockID = currentChunk->getBlockID(voxelIndex);

            // Check For The Block ID
            if (blockID && (blockID < LOWWATER)) {
                force -= Blocks[blockID].explosionResistance;

                rayToBlock = dir * vr.getDistanceTraversed();
                ChunkUpdater::removeBlock(currentChunk, voxelIndex, 1, 0.4*force, rayToBlock);
                if (Blocks[blockID].explosivePower){
                    _deferredExplosions.push(ExplosionNode(pos + f64v3(rayToBlock), blockID));
                }
            }

        }
        // Traverse To The Next
        loc = vr.getNextVoxelPosition();
        chunkPos = loc / CHUNK_WIDTH;
    }
}

void PhysicsEngine::performExplosions()
{
    if (_deferredExplosions.empty()) return;

    int val;

    GLuint sticks = SDL_GetTicks();

    while (!(_deferredExplosions.empty())){
        val = _deferredExplosions.front().val;
        if (Blocks[val].emitterOnBreak){
            particleEngine.addEmitter(Blocks[val].emitterOnBreak, _deferredExplosions.front().ppos, val);
        }
        explosion(_deferredExplosions.front().ppos, Blocks[val].explosionRays, Blocks[val].explosivePower, Blocks[val].powerLoss);
        _deferredExplosions.pop();
        if (SDL_GetTicks() - sticks > 20) break;
    }

}

void PhysicsEngine::detectFloatingBlocks(const glm::dvec3 &viewDir)
{
    int detectFloatingSize = 0;
    glm::vec3 explosionDir;
    for (Uint32 ni = 0; ni < _fallingCheckNodes.size(); ni++){ //allocate only enough time per frame.
        if (detectFloatingSize >= F_NODES_MAX_SIZE - MAX_SEARCH_LENGTH){
            for (int i = 0; i < MAX_SEARCH_LENGTH; i++){
                _fnodes[detectFloatingSize - MAX_SEARCH_LENGTH + i].ch->setBlockData(_fnodes[detectFloatingSize - MAX_SEARCH_LENGTH + i].c, _fnodes[detectFloatingSize - MAX_SEARCH_LENGTH + i].blockType);
            }
            detectFloatingSize -= MAX_SEARCH_LENGTH;
        }

        detectFloating(&_fallingCheckNodes[ni], detectFloatingSize, viewDir, MAX_SEARCH_LENGTH);
    }

    _fallingCheckNodes.clear();
    restoreDetectFloatingBlocks(detectFloatingSize);
}

//TODO: Refactor the crap out of this. Tell Ben to change this before you try reading it.
void PhysicsEngine::detectFloating(FallingCheckNode *fallNode, int &start, const glm::dvec3 &viewDirection, int searchLength)
{
    Chunk *ch = fallNode->ch;
    int c = fallNode->c;
    double explosionDist = ((double)fallNode->explosionDist);
    glm::vec3 explosionDir(0.0f);
    glm::dvec3 explosionLoc((double)fallNode->explosionDir[0], (double)fallNode->explosionDir[1], (double)fallNode->explosionDir[2]);
    if (explosionDist != 0){
        searchLength /= 5;
        explosionLoc = explosionLoc + glm::dvec3(ch->position * CHUNK_WIDTH) + glm::dvec3(c%CHUNK_WIDTH, c / CHUNK_LAYER, (c%CHUNK_LAYER) / CHUNK_WIDTH);
    }

    Chunk *sch = ch;
    Chunk *chunk2;
    GLushort blockType;
    int n, ntree, ln;
    int sc = c;
    int ti;
    int c2;
    int startY, btype;
    int floatingAction, nSinceWood, btypeMasked;
    startY = ch->position.y + c / CHUNK_LAYER;
    bool isLeaf, fall;
    GLubyte support;
    bool toDo[6] = { 1, 1, 1, 1, 1, 1 }; //left right front back bottom
    glm::vec4 color;
    glm::vec3 crs;
    glm::vec2 right;
    crs = glm::cross(glm::vec3(viewDirection.x, 0, viewDirection.z), glm::vec3(0.0, 1.0, 0.0));
    //	if (exposion){
    right = glm::normalize(glm::vec2(crs.x, crs.z));
    floatingAction = GETBLOCK((blockType = ch->getTopBlockData(c))).floatingAction;

    if (GETBLOCK(sch->getLeftBlockData(sc)).floatingAction == 0) toDo[1] = 0;
    if (GETBLOCK(sch->getRightBlockData(sc)).floatingAction == 0) toDo[2] = 0;
    if (GETBLOCK(sch->getFrontBlockData(sc)).floatingAction == 0) toDo[3] = 0;
    if (GETBLOCK(sch->getBackBlockData(sc)).floatingAction == 0) toDo[4] = 0;
    if (GETBLOCK(sch->getBottomBlockData(sc)).floatingAction == 0) toDo[5] = 0;

    for (ti = 0; ti < 6; ti++){
        fall = 1;
        n = start;
        ntree = 0;
        if (toDo[ti] == 0) continue;

        switch (ti){
        case 0: //top
            if (floatingAction == 1){
                if (sc / CHUNK_LAYER < CHUNK_WIDTH - 1){
                    _fnodes[n++].setValues(sc + CHUNK_LAYER, sch, 0);
                } else if (sch->top && sch->top->isAccessible){
                    _fnodes[n++].setValues(sc - CHUNK_SIZE + CHUNK_LAYER, sch->top, 0);
                }
            } else if (floatingAction == 2){
                if (sc / CHUNK_LAYER < CHUNK_WIDTH - 1){
                    ChunkUpdater::removeBlock(sch, sc + CHUNK_LAYER, 1);
                } else if (sch->top && sch->top->isAccessible){
                    ChunkUpdater::removeBlock(sch->top, sc - CHUNK_SIZE + CHUNK_LAYER, 1);
                }
                continue;
            } else{
                continue;
            }
            break;
        case 1: //left
            if (sc%CHUNK_WIDTH > 0){
                _fnodes[n++].setValues(sc - 1, sch, 0);
            } else if (sch->left && sch->left->isAccessible){
                _fnodes[n++].setValues(sc + CHUNK_WIDTH - 1, sch->left, 0);
            }
            break;
        case 2: //right
            if (sc%CHUNK_WIDTH < CHUNK_WIDTH - 1){
                _fnodes[n++].setValues(sc + 1, sch, 0);
            } else if (sch->right && sch->right->isAccessible){
                _fnodes[n++].setValues(sc - CHUNK_WIDTH + 1, sch->right, 0);
            }
            break;
        case 3: //front
            if ((sc%CHUNK_LAYER) / CHUNK_WIDTH < CHUNK_WIDTH - 1){
                _fnodes[n++].setValues(sc + CHUNK_WIDTH, sch, 0);
            } else if (sch->front && sch->front->isAccessible){
                _fnodes[n++].setValues(sc - CHUNK_LAYER + CHUNK_WIDTH, sch->front, 0);
            }
            break;
        case 4: //back
            if ((sc%CHUNK_LAYER) / CHUNK_WIDTH > 0){
                _fnodes[n++].setValues(sc - CHUNK_WIDTH, sch, 0);
            } else if (sch->back && sch->back->isAccessible){
                _fnodes[n++].setValues(sc + CHUNK_LAYER - CHUNK_WIDTH, sch->back, 0);
            }
            break;
        case 5: //bottom
            if (sc / CHUNK_LAYER > 0){
                _fnodes[n++].setValues(sc - CHUNK_LAYER, sch, 0);
            } else if (sch->bottom && sch->bottom->isAccessible){
                _fnodes[n++].setValues(sc + CHUNK_SIZE - CHUNK_LAYER, sch->bottom, 0);
            }
            break;
        }

        ln = start + 1;
        if (start == n) fall = 0;
        for (int i = start; i < n; i++){
            c = _fnodes[i].c;
            ch = _fnodes[i].ch;
            btype = _fnodes[i].blockType;
            btypeMasked = GETBLOCKTYPE(btype);
            support = Blocks[btype].isSupportive;
            nSinceWood = _fnodes[i].nSinceWood;
            //if (nSinceWood > MAXLEAFRADIUS) continue;                   REMOVED 
            if (!support && nSinceWood) nSinceWood++;
            if (btypeMasked == WOOD){
                nSinceWood = 1;
                ntree++;
                isLeaf = 0;
            } else if (btypeMasked == LEAVES1 || btypeMasked == LEAVES2){
                ntree++;
                isLeaf = 1;
            } else{
                isLeaf = 0;
            }

            floatingAction = GETBLOCK((blockType = ch->getTopBlockData(c))).floatingAction;
            if (nSinceWood && !support && GETBLOCK(blockType).isSupportive){ ch->setBlockData(c, btype); continue; }//belongs to another tree!
            if ((nSinceWood == 0 || !isLeaf || ((GETBLOCKTYPE(blockType)) != LEAVES1) || blockType == btype) && floatingAction == 1){
                if (c / CHUNK_LAYER < CHUNK_WIDTH - 1){
                    _fnodes[n++].setValues(c + CHUNK_LAYER, ch, nSinceWood);
                } else if (ch->top && ch->top->isAccessible){
                    _fnodes[n++].setValues(c - CHUNK_SIZE + CHUNK_LAYER, ch->top, nSinceWood);
                }
            }


            floatingAction = Blocks[GETBLOCKTYPE((blockType = ch->getLeftBlockData(c)))].floatingAction;
            if (nSinceWood && !support && Blocks[GETBLOCKTYPE(blockType)].isSupportive){ while (n > ln){ _fnodes[--n].ch->setBlockData(_fnodes[n].c, _fnodes[n].blockType); } ch->setBlockData(c, btype); continue; }
            if ((nSinceWood == 0 || !isLeaf || ((GETBLOCKTYPE(blockType)) != LEAVES1) || blockType == btype) && floatingAction == 1){
                if (c%CHUNK_WIDTH > 0){
                    _fnodes[n++].setValues(c - 1, ch, nSinceWood);
                } else if (ch->left && ch->left->isAccessible){
                    _fnodes[n++].setValues(c + CHUNK_WIDTH - 1, ch->left, nSinceWood);
                }
            }

            floatingAction = Blocks[GETBLOCKTYPE((blockType = ch->getRightBlockData(c)))].floatingAction;
            if (nSinceWood && !support && Blocks[GETBLOCKTYPE(blockType)].isSupportive){ while (n > ln){ _fnodes[--n].ch->setBlockData(_fnodes[n].c, _fnodes[n].blockType); } ch->setBlockData(c, btype); continue; }
            if ((nSinceWood == 0 || !isLeaf || ((GETBLOCKTYPE(blockType)) != LEAVES1) || blockType == btype) && floatingAction == 1){
                if (c%CHUNK_WIDTH < CHUNK_WIDTH - 1){
                    _fnodes[n++].setValues(c + 1, ch, nSinceWood);
                } else if (ch->right && ch->right->isAccessible){
                    _fnodes[n++].setValues(c - CHUNK_WIDTH + 1, ch->right, nSinceWood);
                }
            }

            floatingAction = Blocks[GETBLOCKTYPE((blockType = ch->getFrontBlockData(c)))].floatingAction;
            if (nSinceWood && !support && Blocks[GETBLOCKTYPE(blockType)].isSupportive){ while (n > ln){ _fnodes[--n].ch->setBlockData(_fnodes[n].c, _fnodes[n].blockType); } ch->setBlockData(c, btype); continue; }
            if ((nSinceWood == 0 || !isLeaf || ((GETBLOCKTYPE(blockType)) != LEAVES1) || blockType == btype) && floatingAction == 1){
                if ((c%CHUNK_LAYER) / CHUNK_WIDTH < CHUNK_WIDTH - 1){
                    _fnodes[n++].setValues(c + CHUNK_WIDTH, ch, nSinceWood);
                } else if (ch->front && ch->front->isAccessible){
                    _fnodes[n++].setValues(c - CHUNK_LAYER + CHUNK_WIDTH, ch->front, nSinceWood);
                }
            }

            floatingAction = Blocks[GETBLOCKTYPE((blockType = ch->getBackBlockData(c)))].floatingAction;
            if (nSinceWood && !support && Blocks[GETBLOCKTYPE(blockType)].isSupportive){ while (n > ln){ _fnodes[--n].ch->setBlockData(_fnodes[n].c, _fnodes[n].blockType); } ch->setBlockData(c, btype); continue; }
            if ((nSinceWood == 0 || !isLeaf || ((GETBLOCKTYPE(blockType)) != LEAVES1) || blockType == btype) && floatingAction == 1){
                if ((c%CHUNK_LAYER) / CHUNK_WIDTH > 0){
                    _fnodes[n++].setValues(c - CHUNK_WIDTH, ch, nSinceWood);
                } else if (ch->back && ch->back->isAccessible){
                    _fnodes[n++].setValues(c + CHUNK_LAYER - CHUNK_WIDTH, ch->back, nSinceWood);
                }
            }

            floatingAction = Blocks[GETBLOCKTYPE((blockType = ch->getBottomBlockData(c)))].floatingAction;
            if (nSinceWood && !support && Blocks[GETBLOCKTYPE(blockType)].isSupportive){ while (n > ln){ _fnodes[--n].ch->setBlockData(_fnodes[n].c, _fnodes[n].blockType); } ch->setBlockData(c, btype); continue; }
            if ((nSinceWood == 0 || !isLeaf || ((GETBLOCKTYPE(blockType)) != LEAVES1) || blockType == btype) && floatingAction == 1){
                if (c / CHUNK_LAYER > 0){
                    _fnodes[n++].setValues(c - CHUNK_LAYER, ch, nSinceWood);
                } else if (ch->bottom && ch->bottom->isAccessible){
                    _fnodes[n++].setValues(c + CHUNK_SIZE - CHUNK_LAYER, ch->bottom, nSinceWood);
                }
            }
            ln = n;
            if (n >= start + searchLength){
                fall = 0;
                //GET BLOCK RETURNS -1 ON FAILURE COULD CRASH
                if (GETBLOCKTYPE(sch->getLeftBlockData(sc)) == NONE) toDo[1] = 0;
                if (GETBLOCKTYPE(sch->getRightBlockData(sc)) == NONE) toDo[2] = 0;
                if (GETBLOCKTYPE(sch->getFrontBlockData(sc)) == NONE) toDo[3] = 0;
                if (GETBLOCKTYPE(sch->getBackBlockData(sc)) == NONE) toDo[4] = 0;
                if (GETBLOCKTYPE(sch->getBottomBlockData(sc)) == NONE) toDo[5] = 0;
                start = n;
                //for (int j = 0; j < n; j++){
                //	fnodes[j].ch->data[fnodes[j].c] = fnodes[j].blockType;
                //}
                break;
            }
        }

        if (fall){
            if (GETBLOCKTYPE(sch->getLeftBlockData(sc)) == NONE) toDo[1] = 0;
            if (GETBLOCKTYPE(sch->getRightBlockData(sc)) == NONE) toDo[2] = 0;
            if (GETBLOCKTYPE(sch->getFrontBlockData(sc)) == NONE) toDo[3] = 0;
            if (GETBLOCKTYPE(sch->getBackBlockData(sc)) == NONE) toDo[4] = 0;
            if (GETBLOCKTYPE(sch->getBottomBlockData(sc)) == NONE) toDo[5] = 0;


            if ((float)ntree / (float)(n - start) < 0.7f) right = glm::vec2(0.0f);
            //make those blocks fall
            for (int i = n - 1; i >= start; i--){
                blockType = GETBLOCKTYPE(_fnodes[i].blockType);
                Block &block = Blocks[blockType];
                ch = _fnodes[i].ch;
                c = _fnodes[i].c;
                if (GETBLOCK(ch->getTopBlockData(c, c / CHUNK_LAYER, &c2, &chunk2)).floatingAction == 2){
                    ChunkUpdater::removeBlock(chunk2, c2, 1);
                }

                if (explosionDist != 0){
                    explosionDir = glm::vec3((glm::dvec3(ch->position) + glm::dvec3(c%CHUNK_WIDTH, c / CHUNK_LAYER, (c%CHUNK_LAYER) / CHUNK_WIDTH)) - explosionLoc);
                }

                int y = c / CHUNK_LAYER;
                int xz = c - y*CHUNK_LAYER;
                int ydiff = startY - (ch->position.y + c / CHUNK_LAYER);
                if (ydiff > 0) ydiff = 0;

                //TODO: BAD! Make this a script
                if (blockType != FIRE) addPhysicsBlock(glm::dvec3(ch->position) + glm::dvec3(c%CHUNK_WIDTH + 0.5, c / CHUNK_LAYER, (c%CHUNK_LAYER) / CHUNK_WIDTH + 0.5), _fnodes[i].blockType, ydiff, right, explosionDir, ch->getTemperature(xz), ch->getRainfall(xz));

                ch->data[c] = blockType; //so that removeBlock functions correctly
                ChunkUpdater::removeBlock(ch, c, false);
            }
        }
    }
}

void PhysicsEngine::restoreDetectFloatingBlocks(int &size)
{
    for (int i = 0; i < size; i++){
        _fnodes[i].ch->setBlockData(_fnodes[i].c, _fnodes[i].blockType);
    }
    size = 0;
}

void PhysicsEngine::addPhysicsBlock(const glm::dvec3 &pos, int blockType) {
    if (_physicsBlockBatches[blockType] == NULL){
        _physicsBlockBatches[blockType] = new PhysicsBlockBatch(blockType, 128, 128);
        _activePhysicsBlockBatches.push_back(_physicsBlockBatches[blockType]);
    }
    _physicsBlockBatches[blockType]->addBlock(pos, 0, glm::vec2(0.0f), glm::vec3(0.0f));
}

void PhysicsEngine::addPhysicsBlock(const glm::dvec3 &pos, int blockType, int ydiff, glm::vec2 &dir, glm::vec3 explosionDir, GLubyte temp, GLubyte rain)
{
    if (_physicsBlockBatches[blockType] == NULL){
        _physicsBlockBatches[blockType] = new PhysicsBlockBatch(blockType, temp, rain);
        _activePhysicsBlockBatches.push_back(_physicsBlockBatches[blockType]);
    }
    float expLength = glm::length(explosionDir);
    if (expLength){
        float force = pow(0.89, expLength)*0.6;
        if (force < 0.0f) force = 0.0f;
        glm::vec3 expForce = glm::normalize(explosionDir)*force;
        glm::vec2 tmp(0.0);
        _physicsBlockBatches[blockType]->addBlock(pos, ydiff, tmp, expForce);
    } else{
        _physicsBlockBatches[blockType]->addBlock(pos, ydiff, dir, glm::vec3(0.0f));
    }
}

FallingCheckNode::FallingCheckNode(Chunk *chk, GLushort C, GLbyte expNx, GLbyte expNy, GLbyte expNz, GLubyte expDist){
    ch = chk;
    c = C;
    explosionDir[0] = expNx;
    explosionDir[1] = expNy;
    explosionDir[2] = expNz;
    explosionDist = expDist;
}

void FallingNode::setValues(GLushort C, Chunk *Ch, int nsw){
    c = C;
    ch = Ch;
    blockType = ch->getBlockData(c);
    ch->setBlockData(c, NONE); //set the actual block to none for now
    nSinceWood = nsw;
}
