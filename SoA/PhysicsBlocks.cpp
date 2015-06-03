#include "stdafx.h"
#include "PhysicsBlocks.h"

#include <Vorb/utils.h>

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkUpdater.h"
#include "Frustum.h"
#include "GameManager.h"
#include "SoaOptions.h"
#include "Particles.h"
#include "PhysicsEngine.h"
#include "RenderUtils.h"
#include "TerrainGenerator.h"
#include "Texture2d.h"
#include "VoxelMesher.h"

f32m4 PhysicsBlockBatch::worldMatrix(1.0);

void PhysicsBlockMesh::createVao(const vg::GLProgram* glProgram) {
    if (vaoID == 0) {
        glGenVertexArrays(1, &vaoID);
    }
    glBindVertexArray(vaoID);

    ui32 loc; // Attribute location

    // Enable attributes
    glProgram->enableVertexAttribArrays();

    // * Global instance attributes here *
    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    loc = glProgram->getAttribute("vertexPosition_blendMode");
    glVertexAttribPointer(loc, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(PhysicsBlockVertex), (void*)offsetof(PhysicsBlockVertex, position));
    glVertexAttribDivisor(loc, 0);
    //UVs
    loc = glProgram->getAttribute("vertexUV");
    glVertexAttribPointer(loc, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(PhysicsBlockVertex), (void*)offsetof(PhysicsBlockVertex, tex));
    glVertexAttribDivisor(loc, 0);
    //textureAtlas_textureIndex
    loc = glProgram->getAttribute("textureAtlas_textureIndex");
    glVertexAttribPointer(loc, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(PhysicsBlockVertex), (void*)offsetof(PhysicsBlockVertex, textureAtlas));
    glVertexAttribDivisor(loc, 0);
    //textureDimensions
    loc = glProgram->getAttribute("textureDimensions");
    glVertexAttribPointer(loc, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(PhysicsBlockVertex), (void*)offsetof(PhysicsBlockVertex, textureWidth));
    glVertexAttribDivisor(loc, 0);
    //normals
    loc = glProgram->getAttribute("normal");
    glVertexAttribPointer(loc, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PhysicsBlockVertex), (void*)offsetof(PhysicsBlockVertex, normal));
    glVertexAttribDivisor(loc, 0);

    // * Per instance attributes here *
    glBindBuffer(GL_ARRAY_BUFFER, positionLightBufferID);
    //center
    loc = glProgram->getAttribute("centerPosition");
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(PhysicsBlockPosLight), (void*)offsetof(PhysicsBlockPosLight, pos));
    glVertexAttribDivisor(loc, 1);
    //color
    loc = glProgram->getAttribute("vertexColor");
    glVertexAttribPointer(loc, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PhysicsBlockPosLight), (void*)offsetof(PhysicsBlockPosLight, color));
    glVertexAttribDivisor(loc, 1);
    //overlayColor
    loc = glProgram->getAttribute("overlayColor");
    glVertexAttribPointer(loc, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PhysicsBlockPosLight), (void*)offsetof(PhysicsBlockPosLight, overlayColor));
    glVertexAttribDivisor(loc, 1);
    //light
    loc = glProgram->getAttribute("vertexLight");
    glVertexAttribPointer(loc, 2, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PhysicsBlockPosLight), (void*)offsetof(PhysicsBlockPosLight, light));
    glVertexAttribDivisor(loc, 1);

    glBindVertexArray(0);
}

PhysicsBlock::PhysicsBlock(const f32v3& pos, PhysicsBlockBatch* Batch, i32 BlockType, i32 ydiff, f32v2& dir, f32v3 extraForce) :
    position(pos),
    batch(Batch),
    done(false),
    colliding(false)
{
    // TODO(Ben): What the fuck is this shit?
    f32 v = 0.0;
    bool tree = 0;
    done = 0;
    if (dir[0] != 0 || dir[1] != 0) tree = 1;

    if (ydiff < 0) ydiff = -ydiff;

    if (ydiff > 50){
        if (tree){
     //       grav = GRAVITY;
     //       fric = 0.98f - 0.02f;
        }
        v = 1.0f;
    } else if (ydiff > 1){
        v = (ydiff - 1) / 49.0f;
        if (tree){
     //       grav = GRAVITY;
     //       fric = 0.98f - 0.02*(ydiff - 1) / 49.0f;
        }
    }

    if (v){
        velocity.x = ((rand() % 100) * .001f - 0.05f + dir[0] * 1.65f)*v;
        velocity.y = 0;
        velocity.z = ((rand() % 100) * .001f - 0.05f + dir[1] * 1.65f)*v;
    } else{
        velocity = glm::vec3(0.0f);
    }

    velocity += extraForce;

    light[LIGHT] = 0;
    light[SUNLIGHT] = (GLubyte)(255.0f*(LIGHT_OFFSET + LIGHT_MULT));
}

int bdirs[96] = { 0, 1, 2, 3, 0, 1, 3, 2, 0, 2, 3, 1, 0, 2, 1, 3, 0, 3, 2, 1, 0, 3, 1, 2,
1, 0, 2, 3, 1, 0, 3, 2, 1, 2, 0, 3, 1, 2, 3, 0, 1, 3, 2, 0, 1, 3, 0, 2,
2, 0, 1, 3, 2, 0, 3, 1, 2, 1, 3, 0, 2, 1, 0, 3, 2, 3, 0, 1, 2, 3, 1, 0,
3, 0, 1, 2, 3, 0, 2, 1, 3, 1, 2, 0, 3, 1, 0, 2, 3, 2, 0, 1, 3, 2, 1, 0 };

const GLushort boxIndices[36] = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20 };

bool PhysicsBlock::update(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk*& lockedChunk)
{
    //if (done == 2) return true; //defer destruction by two frames for good measure
    //if (done != 0) {
    //    done++;
    //    return false;
    //}

    //i32& blockID = batch->_blockID;

    //if (globalDebug2 == 0) return false;

    //// If we are colliding, we reverse motion until we hit an air block
    //if (colliding) {
    //    position -= velocity * physSpeedFactor;
    //    velocity.y -= 0.01f; // make it move upwards so it doesn't fall forever
    //    if (velocity.y < -0.5f) velocity.y = -0.5f;
    //} else {
    //    // Update Motion
    //    velocity.y -= batch->_gravity * physSpeedFactor;
    //    if (velocity.y < -1.0f) velocity.y = -1.0f;
    //    velocity.z *= batch->_friction;
    //    velocity.x *= batch->_friction;
    //    position += velocity * physSpeedFactor;   
    //}

    //// Get world position
    //f64v3 worldPos = f64v3(position) + batch->_position;

    //// Get the chunk position
    //i32v3 chPos = chunkManager->getChunkPosition(worldPos);
    //
    //// Get the chunk
    //Chunk* ch = chunkManager->getChunk(chPos);
    //if ((!ch) || ch->isAccessible == 0) return true;

    //
    //f32v3 relativePos(worldPos.x - chPos.x * CHUNK_WIDTH,
    //                  worldPos.y - chPos.y * CHUNK_WIDTH,
    //                  worldPos.z - chPos.z * CHUNK_WIDTH);

    //// Grab block index coords
    //int bx = (int)relativePos.x % CHUNK_WIDTH;
    //int by = (int)relativePos.y % CHUNK_WIDTH;
    //int bz = (int)relativePos.z % CHUNK_WIDTH;

    //int c = bx + by*CHUNK_LAYER + bz*CHUNK_WIDTH;

    //// Get the colliding block
    //i32 collideID = ch->getBlockIDSafe(lockedChunk, c);
    //Block& collideBlock = Blocks[collideID];

    //// If we are colliding we need an air block
    //if (colliding) {
    //    if (collideBlock.collide == false) {
    //     //   if (Blocks[blockID].explosivePower) {
    //     //       GameManager::physicsEngine->addExplosion(
    //     //           ExplosionNode(f64v3(position) + batch->_position,
    //     //           blockID));
    //     //   } else {
    //            ChunkUpdater::placeBlock(ch, lockedChunk, c, batch->blockType);
    //     //   }
    //        // Mark this block as done so it can be removed in a few frames
    //        done = 1;
    //    }
    //    return false;
    //}

    //// Check if its collidable
    //if (collideBlock.collide) {
    //    // If physics block crushable, like leaves, just break it.
    //    if (Blocks[blockID].isCrushable) {
    //        glm::vec4 color;
    //        color.r = Blocks[blockID].color.r;
    //        color.g = Blocks[blockID].color.g;
    //        color.b = Blocks[blockID].color.b;
    //        color.a = 255;

    //        particleEngine.addParticles(chunkManager, BPARTICLES, worldPos, 0, 0.1, 300, 1, color, Blocks[blockID].base.px, 2.0f, 4);
    //        return true;
    //    }
    //    
    //    // If colliding block is crushable, break that block
    //    if (collideBlock.isCrushable) {
    //        ChunkUpdater::removeBlock(chunkManager, physicsEngine, ch, lockedChunk, c, true);
    //    } else {
    //        colliding = true;
    //    }
    //} else {
    //    // Grab light data from grid
    //    // TODO(Ben): Get Lamp Light
    //    light[LIGHT] = 0;
    //    light[SUNLIGHT] = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - ch->getSunlight(c))));
    //}

    return false;
}

//temp and rain for dirtgrass
PhysicsBlockBatch::PhysicsBlockBatch(int BlockType, GLubyte temp, GLubyte rain) : blockType(BlockType), _mesh(NULL), _numBlocks(0)
{

    // TODO(Ben): Re-implement
    //physicsBlocks.reserve(512);

    //PhysicsBlockMeshMessage *pbmm = new PhysicsBlockMeshMessage;
    //std::vector <PhysicsBlockVertex> &verts = pbmm->verts;
    //verts.resize(36);

    //_gravity = GRAVITY;
    //_friction = 0.985f;
    //_blockID = GETBLOCKID(BlockType);

    //double v = 0.0;
    //bool tree = 0;

    //int flags = GETFLAGS(BlockType) >> 12;

    //int index = 0;
    //const Block &block = Blocks[_blockID];

    ////front
    //VoxelMesher::makePhysicsBlockFace(verts, 0, index, block.pzTexInfo);
    //index += 6;
    ////right
    //VoxelMesher::makePhysicsBlockFace(verts, 12, index, block.pxTexInfo);
    //index += 6;
    ////top

    //VoxelMesher::makePhysicsBlockFace(verts, 24, index, block.pyTexInfo);
    //index += 6;
    ////left

    //VoxelMesher::makePhysicsBlockFace(verts, 36, index, block.nxTexInfo);
    //index += 6;
    ////bottom

    //VoxelMesher::makePhysicsBlockFace(verts, 48, index, block.nyTexInfo);
    //index += 6;
    ////back

    //VoxelMesher::makePhysicsBlockFace(verts, 60, index, block.nzTexInfo);
    //index += 6;

    //_mesh = new PhysicsBlockMesh;
    //pbmm->mesh = _mesh;
    //
    //GameManager::messageManager->enqueue(ThreadId::UPDATE,
    //                                     Message(MessageID::PHYSICS_BLOCK_MESH,
    //                                     (void *)pbmm));
}

PhysicsBlockBatch::~PhysicsBlockBatch()
{

    // TODO(Ben): Re-implement
    /* if (_mesh != NULL){
         PhysicsBlockMeshMessage *pbmm = new PhysicsBlockMeshMessage;
         pbmm->mesh = _mesh;
         GameManager::messageManager->enqueue(ThreadId::UPDATE,
         Message(MessageID::PHYSICS_BLOCK_MESH,
         (void *)pbmm));
         }*/
}

void PhysicsBlockBatch::draw(PhysicsBlockMesh *pbm, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP)
{
    if (pbm == NULL) return;
    if (pbm->numBlocks == 0) return;

    setMatrixTranslation(worldMatrix, (double)pbm->bX - PlayerPos.x,
                         (double)pbm->bY - PlayerPos.y + 0.5,
                         (double)pbm->bZ - PlayerPos.z);

    glm::mat4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(pbm->vaoID);

    //TODO: glDrawElementsInstanced
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, pbm->numBlocks); //draw instanced arrays

    glBindVertexArray(0);
}

bool PhysicsBlockBatch::update(ChunkManager* chunkManager, PhysicsEngine* physicsEngine)
{

    // TODO(Ben): Re-implement
    //size_t i = 0;

    //PhysicsBlockMeshMessage *pbmm = new PhysicsBlockMeshMessage;
    //std::vector <PhysicsBlockPosLight> &verts = pbmm->posLight;
    //verts.resize(physicsBlocks.size());
    //
    //ColorRGB8 color, overlayColor;

    ////need to fix this so that color is correct
    //Blocks[blockType].GetBlockColor(color, overlayColor, 0, 128, 128, Blocks[blockType].pzTexInfo);

    //Chunk* lockedChunk = nullptr;

    //while (i < physicsBlocks.size()) {
    //    if (physicsBlocks[i].update(chunkManager, physicsEngine, lockedChunk)){
    //        physicsBlocks[i] = physicsBlocks.back();
    //        physicsBlocks.pop_back(); //dont need to increment i 
    //    } else{ //if it was successfully updated, add its data to the buffers
    //        verts[i].pos = physicsBlocks[i].position;

    //        // TODO(Color) can be instanced
    //        verts[i].color = color;
    //        verts[i].overlayColor = overlayColor;

    //        verts[i].light[0] = physicsBlocks[i].light[0];
    //        verts[i].light[1] = physicsBlocks[i].light[1];
    //        i++;
    //    }
    //}
    //if (lockedChunk) lockedChunk->unlock();

    //_numBlocks = i;
    //verts.resize(_numBlocks); //chop off extras

    //if (_numBlocks == 0){
    //    if (_mesh != NULL){
    //        pbmm->mesh = _mesh;
    //        GameManager::messageManager->enqueue(ThreadId::UPDATE,
    //                                             Message(MessageID::PHYSICS_BLOCK_MESH,
    //                                             (void *)pbmm));
    //        _mesh = NULL;
    //    }
    //    return 1;
    //}

    //pbmm->bX = _position.x;
    //pbmm->bY = _position.y;
    //pbmm->bZ = _position.z;
    //pbmm->numBlocks = _numBlocks;
    //if (_mesh == NULL){
    //    pError("AHHHHH WHAT? Physics block mesh null!?");
    //}
    //pbmm->mesh = _mesh;

    //GameManager::messageManager->enqueue(ThreadId::UPDATE,
    //                                     Message(MessageID::PHYSICS_BLOCK_MESH,
    //                                     (void *)pbmm));

    return 0;
}

void PhysicsBlockBatch::addBlock(const glm::dvec3 &pos, int ydiff, glm::vec2 &dir, glm::vec3 extraForce)
{
    if (physicsBlocks.size() == 0){ //if its the first one, set it to this block
        _position = pos;
    }
    physicsBlocks.emplace_back(f32v3(pos - _position), this, blockType, ydiff, dir, extraForce);
}