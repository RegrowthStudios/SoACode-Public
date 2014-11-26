#include "stdafx.h"
#include "Collision.h"

#include "BlockData.h"
#include "Chunkmanager.h"
#include "Player.h"
#include "utils.h"
#include "VoxelNavigation.inl"

void blockCollision(Player* player, Chunk* chunk, Chunk* lockedChunk, ui16 blockType, i32 c, f64 bdx, f64 bdy, f64 bdz, f64 dx, f64 dy, f64 dz);


//This method could be easily implemented as a recursive function, but is more efficient if unfolded
//TODO(Ben) This is laughable. 
void aabbChunkCollision(Player* player, f64v3* playerPos, Chunk** chunks, ui8 size)
{
    int x, y, z, x1, y1, z1 ,x2, y2, z2, x3, y3, z3, x4, y4, z4, c; //midpoints
    int blockID;
    double x5, y5, z5; //only this one requires double precision
    double yPosOffset;
    double boxX, boxY, boxZ;
    double dx, dy, dz, bdx, bdy, bdz;
    Chunk *chunk;

    glm::vec3 *playerBox = &(player->boundingBox);
    boxX = playerBox->x;
    boxY = playerBox->y/2.0; //the box point is dead center of the player
    boxZ = playerBox->z;
    yPosOffset = boxY; //since we are using a center point, we use yPosOffset to move the midpoint from the feet to the center

    Chunk* lockedChunk = nullptr;

    for (unsigned char i = 0; i < size; i++) { //loops through chunks

        if (!(chunks[i]) || chunks[i]->isAccessible == false) continue; //avoid errors

        //find the midpoint so that we can subdivide the chunk into 8 sections
        x = chunks[i]->gridPosition.x + CHUNK_WIDTH / 2;
        y = chunks[i]->gridPosition.y + CHUNK_WIDTH / 2;
        z = chunks[i]->gridPosition.z + CHUNK_WIDTH / 2;

        //checks to see if the distance between the players midpoint and the box midpoint is greater than the radius
        //all three axis must be colliding for a collision
        if ((boxX + 16) < ABS(playerPos->x - x)) continue; 
        if ((boxY + 16) < ABS(playerPos->y + yPosOffset - y)) continue;
        if ((boxZ + 16) < ABS(playerPos->z - z)) continue;

        for (unsigned char l0 = 0; l0 < 2; l0++){ //zeroth subdivide
            //loops through each of the smaller boxes, setting x1, y1, z1 to the midpoint
            if (l0 == 0){
                y1 = y + 8;
            }else{
                y1 = y - 8;
            }
            for (unsigned char d = 0; d < 4; d++){
                if (d == 0){
                    x1 = x - 8;
                    z1 = z - 8;
                }else if (d==1){
                    x1 = x + 8;
                    z1 = z - 8;
                }else if (d==2){
                    x1 = x - 8;
                    z1 = z + 8;
                }else if (d==3){
                    x1 = x + 8;
                    z1 = z + 8;
                }
                //same thing, check if the player is inside the intersection
                if ((boxX + 8) < ABS(playerPos->x - x1)) continue;
                if ((boxY + 8) < ABS(playerPos->y + yPosOffset - y1)) continue;
                if ((boxZ + 8) < ABS(playerPos->z - z1)) continue;
                for (unsigned char l = 0; l < 2; l++){ //first subdivide
                    if (l == 0){
                        y2 = y1 + 4;
                    }else{
                        y2 = y1 - 4;
                    }
                    for (unsigned char j = 0; j < 4; j++){
                        if (j == 0){
                            x2 = x1 - 4;
                            z2 = z1 - 4;
                        }else if (j==1){
                            x2 = x1 + 4;
                            z2 = z1 - 4;
                        }else if (j==2){
                            x2 = x1 - 4;
                            z2 = z1 + 4;
                        }else if (j==3){
                            x2 = x1 + 4;
                            z2 = z1 + 4;
                        }

                        if ((boxX + 4) < ABS(playerPos->x - x2)) continue;
                        if ((boxY + 4) < ABS(playerPos->y + yPosOffset - y2)) continue;
                        if ((boxZ + 4) < ABS(playerPos->z - z2)) continue;

                        for (unsigned char l2 = 0; l2 < 2; l2++){//second subdivide
                            if (l2 == 0){
                                y3 = y2 + 2;
                            }else{
                                y3 = y2 - 2;
                            }

                            for (unsigned char k = 0; k < 4; k++){
                                if (k == 0){
                                    x3 = x2 - 2;
                                    z3 = z2 - 2;
                                }else if (k==1){
                                    x3 = x2 + 2;
                                    z3 = z2 - 2;
                                }else if (k==2){
                                    x3 = x2 - 2;
                                    z3 = z2 + 2;
                                }else if (k==3){
                                    x3 = x2 + 2;
                                    z3 = z2 + 2;
                                }
                                if ((boxX + 2) < ABS(playerPos->x - x3)) continue;
                                if ((boxY + 2) < ABS(playerPos->y + yPosOffset - y3)) continue;
                                if ((boxZ + 2) < ABS(playerPos->z - z3)) continue;
                                
                                for (unsigned char l3 = 0; l3 < 2; l3++){ //third subdivide
                                    if (l3 == 0){
                                        y4 = y3 + 1;
                                    }else{
                                        y4 = y3 - 1;
                                    }
                                    for (unsigned char m = 0; m < 4; m++){
                                        if (m == 0){
                                            x4 = x3 - 1;
                                            z4 = z3 - 1;
                                        }else if (m==1){
                                            x4 = x3 + 1;
                                            z4 = z3 - 1;
                                        }else if (m==2){
                                            x4 = x3 - 1;
                                            z4 = z3 + 1;
                                        }else if (m==3){
                                            x4 = x3 + 1;
                                            z4 = z3 + 1;
                                        }
                                        if ((boxX + 1) < ABS(playerPos->x - x4)) continue;
                                        if ((boxY + 1) < ABS(playerPos->y + yPosOffset - y4)) continue; 
                                        if ((boxZ + 1) < ABS(playerPos->z - z4)) continue;

                                        for (unsigned char l4 = 0; l4 < 2; l4++){ //final subdivide. Check if the player is intersecting a block
                                            if (l4 == 0){
                                                y5 = y4 + 0.5;
                                            }else{
                                                y5 = y4 - 0.5;
                                            }

                                            for (unsigned char n = 0; n < 4; n++){
                                                if (n == 0){
                                                    x5 = x4 - 0.5;
                                                    z5 = z4 - 0.5;
                                                }else if (n==1){
                                                    x5 = x4 + 0.5;
                                                    z5 = z4 - 0.5;
                                                }else if (n==2){
                                                    x5 = x4 - 0.5;
                                                    z5 = z4 + 0.5;
                                                }else if (n==3){
                                                    x5 = x4 + 0.5;
                                                    z5 = z4 + 0.5;
                                                }
                
                                                //find the distance from the players midpoint to the block's midpoint.
                                                dx = playerPos->x - x5;
                                                dy = playerPos->y + yPosOffset - y5;
                                                dz = playerPos->z - z5;

                                                if ((boxX + 0.5) < ABS(dx)) continue;
                                                if ((boxY + 0.5) < ABS(dy)) continue; 
                                                if ((boxZ + 0.5) < ABS(dz)) continue;

                                                int blx, bly, blz;
                                                bool moveUp = 1;
                                                
                                                //finds the block coordinate location in the chunk
                                                double tmp = CHUNK_WIDTH/2.0 - 0.5;
                                                blx = (int)(x5 - (x - tmp));
                                                bly = (int)(CHUNK_LAYER * (y5 - (y - tmp)));
                                                blz = (int)(CHUNK_WIDTH * (z5 - (z - tmp)));

                                                //finds the depth of collision. Higher value means deeper collision. Must reference dx dy and dz
                                                //for direction. Substracts the length of both bounding boxes by the distance between to get the
                                                //difference.
                                                bdx = (double)(0.5+boxX) - ABS(dx);
                                                bdy = (double)(0.5+boxY) - ABS(dy);
                                                bdz = (double)(0.5+boxZ) - ABS(dz);

                                                c = blx + bly + blz;
                                                chunk = chunks[i];
                                                if (chunk->isAccessible == false) continue;

                                                blockID = chunk->getBlockIDSafe(lockedChunk, c);

                                                if (blockID){
                                                    blockCollision(player, lockedChunk, chunks[i], blockID, c, bdx, bdy, bdz, dx, dy, dz);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (lockedChunk) lockedChunk->unlock();
}

// TODO(Ben): What the FUCK is this?!?! This code is rated XXX.
void blockCollision(Player *player, Chunk *chunk, Chunk* lockedChunk, GLushort blockType, int c, double bdx, double bdy, double bdz, double dx, double dy, double dz)
{
//    if (chunks[i]->data[blx + bly + blz] == WATER) continue; //no clip water

    glm::dvec3 *playerPos = &(player->gridPosition);
    double boxX = player->boundingBox.x;
    double boxY = player->boundingBox.y/2.0;
    double boxZ = player->boundingBox.z;
    double stepMod = 1.0f/(float)PLAYER_COLLISION_STEPS;
    bool pushedDown = 0;
    CollisionData *cData = &(player->collisionData);

    if (blockType >= LOWWATER){
    //    cout << (player->headPosition.y - player->position.y) << "  " << dy-(blockType - LOWWATER)*0.005 << endl;
        if (dy <= 0) player->isSwimming = 1;
    //    if (dy-(blockType - LOWWATER)*0.005 <= -(player->headPosition.y - player->position.y)) player->underWater = 1;
        if (dy+0.5 < -(player->headPosition.y - player->gridPosition.y - boxY - (blockType - LOWWATER)*0.01)) player->isUnderWater = 1;
    }

    //cout << dx << " " << dy << " " << dz << " " << bdx << " " << bdy << " " << bdz << endl;
    if (Blocks[blockType].moveMod <= 1.0){
        if (Blocks[blockType].moveMod < player->getMoveMod()) player->setMoveMod(Blocks[blockType].moveMod);
    }
    else if (player->getMoveMod() >= 1.0){ //slippery things like ice
        if (Blocks[blockType].moveMod > player->getMoveMod()) player->setMoveMod(Blocks[blockType].moveMod);
    }
    //player->moveMod *= pow(Blocks[blockType].moveMod, stepMod);
    if (Blocks[blockType].collide == 0 || player->isFlying) return; //do no collision with the block

    double mov = 0.07 * glSpeedFactor * stepMod; //mov is the distance we should move from a collision
    double push = 0.9 * stepMod;//how hard the block tries to push us out. Start with an almost full push. Dont set to 1.0 or it pushes too far and we climb jerkily
    bool moveUp = 0;    
    bool collided = 0;

    if (dy >= 1.0) { //if the player is more than 1.0 block above a block
        Chunk *own;
        int nc;
        int topc = vvox::getTopBlockData(chunk, lockedChunk, c, nc, own);
        if (GETBLOCK(topc).collide == 0 && GETBLOCK(vvox::getTopBlockData(own, lockedChunk, nc)).collide == 0) { // if there is at least 2 free spaces above
    //        cout << "TOP: " << chunk->GetTopBlock(c) << " " << (int)GETBLOCK(chunk->GetTopBlock(c)).collide << " ";
            moveUp = 1;
            push = 0.1 * stepMod; //if its a low climb, we can sorta clip into it
        }
    }else if (dy > -2.0 && dy < 1.0){
        player->canCling = 1;
        if (player->isSprinting) { //climbing happens when sprinting or holding jump
            if (GETBLOCK(vvox::getTopBlockData(chunk, lockedChunk, c)).collide == 0) {
                moveUp = 1;
                player->isClinging = 1;
            }
        }
    }

    if (player->isSprinting){
        mov = 0.1 * glSpeedFactor * stepMod;
    }

    if (moveUp){
        if (bdy < mov){ //just a normal top of the block collision
            if (bdy > cData->yMove) cData->yMove = bdy;
            if (player->velocity.y < 0.0f){
                if (-player->velocity.y > cData->yDecel) cData->yDecel = -player->velocity.y;
                player->velocity.y = 0.0f;
            }
            if (bdx > 0.2 && bdz > 0.2){
                player->isGrounded = 1;
            }
        }else{ //climbing collision
            if (mov > cData->yMove) cData->yMove = mov;
            if (player->velocity.y < 0.0f){
                if (-player->velocity.y > cData->yDecel) cData->yDecel = -player->velocity.y;
                player->velocity.y = 0.0f;
            }
            player->isClimbing = 1;
        }
    }

    if (bdy < bdz && bdy < bdx && dy < -0.5 && Blocks[GETBLOCKID(vvox::getBottomBlockData(chunk, lockedChunk, c))].collide == 0) { //head y collision
         //TODO PREVENT A FAST MOVING PERSON FROM GOING THROUGH BOTTOM BY TESTING HOW MUCH WE CAN CROUCH BEFORE WE BOUNCE OFF
    //    cout << "A";
        if (bdy > cData->headSquish) cData->headSquish = bdy; 
        if (player->velocity.y > 0.0f) player->velocity.y = 0.0; //maybe not do this? let people hit their heads and feet scrunch up
    }
    if (bdx < bdz && bdy > 0.2){ //x collision BDY is different when crouching. Look into this
        if (!player->isSprinting  && GETBLOCK(vvox::getBottomBlockData(chunk, lockedChunk, c)).collide == 0) { //auto crouch
            if (player->getCrouch() != 1.0){
                if (dx < 0 && GETBLOCK(vvox::getLeftBlockData(chunk, lockedChunk, c)).collide == 0) {
                    pushedDown = 1;
                } else if (dx > 0 && GETBLOCK(vvox::getRightBlockData(chunk, lockedChunk, c)).collide == 0) {
                    pushedDown = 1;
                }
            }
            if (pushedDown && !player->isClinging){
                collided = 1; //to stop z from colliding
                mov = 0.2 * glSpeedFactor * stepMod;
                if (mov > bdy - 0.2) mov = bdy - 0.2;
                if (mov > cData->headSquish) cData->headSquish = mov; 
    //            cout << "G " << (int)player->clinging;
            }
            //if (player->velocity.y > 0.0f) player->velocity.y = 0.0; 
        }
        if (!pushedDown || dy > -0.2){
            if (dx > 0 && GETBLOCK(vvox::getRightBlockData(chunk, lockedChunk, c)).collide == 0) {
                mov = bdx*push;
                if (mov > ABS(cData->xMove)) cData->xMove = mov;
                collided = 1;
    //            cout << "C";
            } else if (GETBLOCK(vvox::getLeftBlockData(chunk, lockedChunk, c)).collide == 0) {
                mov = bdx*push;
                if (mov > ABS(cData->xMove)) cData->xMove = -mov;
                collided = 1;
//                cout << "C";
            }
        }
        if ((1.0 - push) < cData->xPush) cData->xPush = 1.0 - push;
    }
    if (bdy > 0.2 && !collided){ //z collision
        if (!player->isSprinting && dy < -0.0 && GETBLOCK(vvox::getBottomBlockData(chunk, lockedChunk, c)).collide == 0) { //auto crouch
            if (player->getCrouch() != 1.0){
                if (dz < 0 && GETBLOCK(vvox::getBackBlockData(chunk, lockedChunk, c)).collide == 0) {
                    pushedDown = 1;
                } else if (dz > 0 && GETBLOCK(vvox::getFrontBlockData(chunk, lockedChunk, c)).collide == 0) {
                    pushedDown = 1;
                }
            }
            if (pushedDown && !player->isClinging){
    //            cout << "A ";
                mov = 0.2 * glSpeedFactor * stepMod;
                if (mov > bdy - 0.2) mov = bdy - 0.2;
                if (mov > cData->headSquish) cData->headSquish = mov; 
            }
            //if (player->velocity.y > 0.0f) player->velocity.y = 0.0; 
        }
        if (!pushedDown || dy > -0.2){
            if (dz > 0 && GETBLOCK(vvox::getFrontBlockData(chunk, lockedChunk, c)).collide == 0) {
                mov = bdz*push;
                if (mov > ABS(cData->zMove)) cData->zMove = mov;
    //            cout << "B";
            } else if (GETBLOCK(vvox::getBackBlockData(chunk, lockedChunk, c)).collide == 0) {
                mov = bdz*push;
                if (mov > ABS(cData->zMove)) cData->zMove = -mov;
    //            cout << "B";
            }
        }
        if ((1.0 - push) < cData->zPush) cData->zPush = 1.0 - push;
    }
}