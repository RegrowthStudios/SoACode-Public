#include "stdafx.h"
#include "ParticleEmitter.h"

#include "ChunkManager.h"
#include "GameManager.h"
#include "ParticleEngine.h"

ParticleEmitter::ParticleEmitter(): dt(0), position(0.0), type(EMITTER_DEFAULT), maxDuration(INT_MAX), initialVel(0.0f), minSizeE(0),
maxSizeE(0), minSizeS(0), maxSizeS(0), spawnTimeS(100), spawnTimeE(100) {}

ParticleEmitter::ParticleEmitter(glm::dvec3 pos, int typ) : position(pos), type(typ), dt(0), maxDuration(INT_MAX), initialVel(0.0f), minSizeE(0),
maxSizeE(0), minSizeS(0), maxSizeS(0), spawnTimeS(100), spawnTimeE(100) {}

i32 ParticleEmitter::update(ChunkManager* chunkManager) {
    const glm::dvec3 chunkListPos(0);
    GLuint currSpawnTime;
    switch(type) {
    case EMITTER_LINEAR:
    case EMITTER_STATIC:
        if(dt >= maxDuration) return 1;

        //interpolate between spawn times
        currSpawnTime = (dt / (float)maxDuration) * (spawnTimeE - spawnTimeS) + spawnTimeS;

        if((dt % currSpawnTime) == 0) {
            particleEngine.addAnimatedParticles(1, position, this);
            if(dt >= minDuration) {

                int gx = position.x - chunkListPos.x;
                int gy = position.y - chunkListPos.y;
                int gz = position.z - chunkListPos.z;

                if(gx < 0 || gy < 0 || gz < 0 || gx >= csGridWidth*CHUNK_WIDTH || gy >= csGridWidth*CHUNK_WIDTH || gz >= csGridWidth*CHUNK_WIDTH) {
                    return 1;
                }

                Chunk *ch;

                i32v3 pos;
                pos.x = (gx / CHUNK_WIDTH);
                pos.y = (gy / CHUNK_WIDTH);
                pos.z = (gz / CHUNK_WIDTH);

                ch = chunkManager->getChunk(pos);
                if((!ch) || ch->isAccessible == 0) return 1;

                pos.x = gx % CHUNK_WIDTH;
                pos.y = gy % CHUNK_WIDTH;
                pos.z = gz % CHUNK_WIDTH;

                int c = pos.x + pos.y*CHUNK_LAYER + pos.z*CHUNK_WIDTH;
                if(ch->getBlockData(c) != blockType) {
                    return 1;
                }
            }
        }
        break;
    }
    dt++;
    return 0;
}