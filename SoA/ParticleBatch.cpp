#include "stdafx.h"
#include "ParticleBatch.h"

#include "ChunkManager.h"
#include "GameManager.h"
#include "OpenglManager.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticleEngine.h"
#include "ParticleMesh.h"
#include "shader.h"

BillboardVertex vboBVerts[maxParticles];

ParticleBatch::~ParticleBatch() {

    //if (mesh != NULL){
    //    ParticleMeshMessage *pmm = new ParticleMeshMessage;
    //    pmm->mesh = mesh;
    //    gameToGl.enqueue(Message(GL_M_PARTICLEMESH, pmm)); //tell GL thread to free the batch
    //}
}

ParticleBatch::ParticleBatch(): size(0),
uvWidth(1.0f / 16.0f),
lastAddedParticle(0),
animated(0),
mesh(NULL) {
    memset(lifes, 0, sizeof(lifes));
}

int ParticleBatch::findUnusedParticle() {
    for(int i = lastAddedParticle; i < maxParticles; i++) {
        if(lifes[i] <= 0.0f) {
            lastAddedParticle = i;
            return i;
        }
    }

    for(int i = 0; i<lastAddedParticle; i++) {
        if(lifes[i] <= 0.0f) {
            lastAddedParticle = i;
            return i;
        }
    }
    return -1; // All particles are taken, fail
}

void ParticleBatch::addParticles(int num, f64v3 pos, int tex, double force, float life, GLubyte billSize, GLubyte color[4], f32v3 extraForce) {
#define POS_OFFSET 10000.0
    
    if(size + num >= maxParticles) return;
    f64v3 spos;
    f64v3 dpos;

    if(size == 0) { //new origin
        // We offset by POS_OFFSET so we can guarentee the batch will always be positive
        position = f64v3(GameManager::chunkManager->getChunkPosition(pos - POS_OFFSET) * CHUNK_WIDTH);
    } else {
        dpos = position - pos;
    }

    int j;
    for(int i = 0; i < num; i++) {
        j = findUnusedParticle();
        if(j == -1) { //no spots left
            size += i;
            return;
        }

        spos.x = (rand() % 100) * .01;
        spos.y = (rand() % 100) * .01;
        spos.z = (rand() % 100) * .01;
        particles[j].velocity = (spos - 0.5) * force + glm::dvec3(extraForce);
        particles[j].position = spos - dpos + 0.25;
        particles[j].texUnit = (GLubyte)(tex / 256);
        particles[j].texID = (GLubyte)(tex % 256);
        particles[j].size = billSize;
        memcpy(particles[j].color, color, 4);
        particles[j].light[0] = 0;
        particles[j].light[1] = 0;
        particles[j].isYCollidable = 0;
        particles[j].pc = 65535;

        lifes[j] = life;
    }
    size += num;
}

//This is a documentation comment
void ParticleBatch::addParticles(int num, glm::dvec3 pos, ParticleEmitter *emitter) {
    if(size + num >= maxParticles) return;
    glm::dvec3 spos;
    glm::dvec3 dpos;
    int currMaxSize, currMinSize;

    if(size == 0) { //new origin
        position = pos;
    } else {
        dpos = position - pos;
    }

    int j;

    float interpolater = (emitter->dt / (float)emitter->maxDuration);
    currMaxSize = interpolater * (emitter->maxSizeE - emitter->maxSizeS) + emitter->maxSizeS;
    currMinSize = interpolater * (emitter->minSizeE - emitter->minSizeS) + emitter->minSizeS;

    switch(emitter->type) {
    case EMITTER_STATIC:

        for(int i = 0; i < num; i++) {
            j = findUnusedParticle();
            if(j == -1) { //no spots left
                size += i;
                return;
            }

            spos.x = ((rand() % 201) - 100) * emitter->rposOffset.x * 0.01f;
            spos.y = ((rand() % 201) - 100) * emitter->rposOffset.y * 0.01f;
            spos.z = ((rand() % 201) - 100) * emitter->rposOffset.z * 0.01f;
            particles[j].velocity = glm::vec3(0.0f);
            particles[j].position = spos - dpos + glm::dvec3(emitter->posOffset) + 0.5;
            particles[j].texUnit = (GLubyte)emitter->particleType;
            particles[j].texID = (GLubyte)0;
            particles[j].type = PARTICLE_NONE;


            if(currMaxSize - currMinSize) {
                particles[j].size = (rand() % (currMaxSize - currMinSize)) + currMinSize;
            } else {
                particles[j].size = currMaxSize;
            }
            memcpy(particles[j].color, particleTypes[emitter->particleType].color, 4);
            particles[j].light[0] = 0;
            particles[j].light[1] = 0;
            particles[j].isYCollidable = 0;
            particles[j].pc = 65535;

            lifes[j] = 1.0f;
        }
        break;
    case EMITTER_LINEAR:
        for(int i = 0; i < num; i++) {
            j = findUnusedParticle();
            if(j == -1) { //no spots left
                size += i;
                return;
            }

            spos.x = ((rand() % 201) - 100) * emitter->rposOffset.x * 0.01f;
            spos.y = ((rand() % 201) - 100) * emitter->rposOffset.y * 0.01f;
            spos.z = ((rand() % 201) - 100) * emitter->rposOffset.z * 0.01f;
            particles[j].type = PARTICLE_LINEAR;
            particles[j].velocity = emitter->initialVel;
            particles[j].position = spos - dpos + glm::dvec3(emitter->posOffset) + 0.5;
            particles[j].texUnit = (GLubyte)emitter->particleType;
            particles[j].texID = (GLubyte)0;
            if(currMaxSize - currMinSize) {
                particles[j].size = (rand() % (currMaxSize - currMinSize)) + currMinSize;
            } else {
                particles[j].size = currMaxSize;
            }
            memcpy(particles[j].color, particleTypes[emitter->particleType].color, 4);
            particles[j].light[0] = 0;
            particles[j].light[1] = 0;
            particles[j].isYCollidable = 0;
            particles[j].pc = 65535;

            lifes[j] = 1.0f;
        }
        break;
    }
    size += num;
}

int ParticleBatch::update() {

    glm::quat quaternion;
    glm::dvec3 pos;
    int n = 0;
    float psize;
    if(size == 0) return 0;

    for(int i = 0; i < maxParticles; i++) {
        if(lifes[i] > 0) {
            lifes[i] -= 0.01f * physSpeedFactor;
            if(lifes[i] < 1.0) {
                if(lifes[i] < 0) {
                    size--;
                    continue;
                }
                psize = particles[i].size * 0.0875 * lifes[i];
            } else {
                psize = particles[i].size * 0.0875;
            }

            particles[i].update(position);

            vboBVerts[n].pos = particles[i].position;
            vboBVerts[n].light[0] = particles[i].light[0];
            vboBVerts[n].light[1] = particles[i].light[1];

            vboBVerts[n].texUnit = particles[i].texUnit;
            vboBVerts[n].texID = particles[i].texID;

            memcpy(vboBVerts[n].color, particles[i].color, 4);
            vboBVerts[n].size = psize;

            n++;

        }

    }
    if(n == 0) return 0;
    ParticleMeshMessage *pmm = new ParticleMeshMessage;

    if(mesh == NULL) {
        mesh = new ParticleMesh;
    }
    pmm->size = n;
    pmm->X = position.x;
    pmm->Y = position.y;
    pmm->Z = position.z;
    pmm->mesh = mesh;
    pmm->verts.resize(n);
    memcpy(&(pmm->verts[0]), vboBVerts, n * sizeof(BillboardVertex));

    gameToGl.enqueue(Message(GL_M_PARTICLEMESH, (void *)pmm));

    return 0;
}

int ParticleBatch::updateAnimated() {

    glm::quat quaternion;
    glm::dvec3 pos;
    int n = 0;
    float deg;
    int anim;
    int timePassed;
    int textureCounter = 0;
    Animation *animation;
    if(size == 0) return 0;
    vector <int> usedTexUnits;

    for(int i = 0; i < maxParticles; i++) {
        if(lifes[i] > 0) {

            particles[i].update();
            ParticleType &pt = particleTypes[particles[i].texUnit];

            animation = pt.animation;
            deg = 1.0f / (animation->duration * 0.0625f);

            lifes[i] -= deg * physSpeedFactor;

            if(lifes[i] < 0) {
                size--;
                continue;
            }
            timePassed = (1.0 - lifes[i]) * animation->duration;
            if(timePassed > animation->fadeOutBegin) {
                particles[i].color[3] = (1.0f - (timePassed - animation->fadeOutBegin) / (float)(animation->duration - animation->fadeOutBegin)) * 255;
            }

            if(animation->repetitions > 0) {
                anim = (int)((1.0 - lifes[i]) * animation->frames * animation->repetitions) % animation->frames;
            } else {
                anim = (1.0 - lifes[i]) * animation->frames;
            }

            //For alpha blend sorting
            //particles[i].distance = glm::length((position + glm::dvec3(particles[i].position)) - player->headPosition);

            vboBVerts[n].pos = particles[i].position;
            vboBVerts[n].light[0] = 255;
            vboBVerts[n].light[1] = 0;

            if(pt.tmpTexUnit == -1) {
                usedTexUnits.push_back(particles[i].texUnit); //store the particle type so we can revert tmptexunit
                pt.tmpTexUnit = textureCounter;
                vboBVerts[n].texUnit = textureCounter++; //it takes up the next tex unit space
            } else {
                vboBVerts[n].texUnit = pt.tmpTexUnit;
            }

            vboBVerts[n].texID = anim;

            memcpy(vboBVerts[n].color, particles[i].color, 4);
            vboBVerts[n].size = particles[i].size;
            vboBVerts[n].xMod = pt.animation->xFrames;
            vboBVerts[n].uvMult = glm::vec2(1.0f / pt.animation->xFrames, 1.0f / pt.animation->yFrames);

            n++;
        }

    }

    //std::sort(&particles[0], &particles[maxParticles-1]);

    ParticleMeshMessage *pmm = new ParticleMeshMessage;

    if(mesh == NULL) {
        mesh = new ParticleMesh;
        mesh->animated = 1;
    }

    for(size_t i = 0; i < usedTexUnits.size(); i++) {
        particleTypes[usedTexUnits[i]].tmpTexUnit = -1; //revert the tmp variable
    }

    pmm->usedParticles.swap(usedTexUnits);
    pmm->size = n;
    pmm->X = position.x;
    pmm->Y = position.y;
    pmm->Z = position.z;
    pmm->mesh = mesh;
    pmm->verts.resize(n);
    memcpy(&(pmm->verts[0]), vboBVerts, n * sizeof(BillboardVertex));

    gameToGl.enqueue(Message(GL_M_PARTICLEMESH, (void *)pmm));

    return 0;
}

void ParticleBatch::draw(ParticleMesh *pm, glm::dvec3 &PlayerPos, glm::mat4 &VP) {
 
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, ballMaskTexture.ID);

    glUniform1f(billboardShader.alphaThresholdID, 0.01f);

    if(pm->size > 0 && pm->uvBufferID != 0) {

        GlobalModelMatrix[3][0] = (float)((double)(pm->X - PlayerPos.x));
        GlobalModelMatrix[3][1] = (float)((double)(pm->Y + 0.175 - PlayerPos.y));
        GlobalModelMatrix[3][2] = (float)((double)(pm->Z - PlayerPos.z));

        glm::mat4 MVP = VP * GlobalModelMatrix;

        glUniformMatrix4fv(billboardShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);
        glUniformMatrix4fv(billboardShader.mvpID, 1, GL_FALSE, &MVP[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, pm->uvBufferID);
        glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, pm->billboardVertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BillboardVertex), 0); //position
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BillboardVertex), (void *)12); //uvMult
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BillboardVertex), (void *)20); //texUnit texID light
        glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BillboardVertex), (void *)24); //color
        glVertexAttribPointer(5, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BillboardVertex), (void *)28); //size, xmod

        glVertexAttribDivisor(0, 1);
        glVertexAttribDivisor(1, 0);
        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, pm->size);
    }

}

void ParticleBatch::drawAnimated(ParticleMesh *pm, glm::dvec3 &PlayerPos, glm::mat4 &VP) {
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, ballMaskTexture.ID);

    glDepthMask(GL_FALSE);

    for(size_t i = 0; i < pm->usedParticles.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        if(particleTypes[pm->usedParticles[i]].animation) {
            glBindTexture(GL_TEXTURE_2D, particleTypes[pm->usedParticles[i]].animation->textureInfo.ID);
        }
    }

   
    glUniform1f(billboardShader.alphaThresholdID, 0.01f);

    if(pm->size > 0 && pm->uvBufferID != 0) {

        GlobalModelMatrix[3][0] = (float)((double)(pm->X - PlayerPos.x));
        GlobalModelMatrix[3][1] = (float)((double)(pm->Y + 0.175 - PlayerPos.y));
        GlobalModelMatrix[3][2] = (float)((double)(pm->Z - PlayerPos.z));

        glm::mat4 MVP = VP * GlobalModelMatrix;

        glUniformMatrix4fv(billboardShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);
        glUniformMatrix4fv(billboardShader.mvpID, 1, GL_FALSE, &MVP[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, pm->uvBufferID);
        glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, pm->billboardVertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BillboardVertex), 0); //position
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BillboardVertex), (void *)12); //uvMult
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BillboardVertex), (void *)20); //texUnit texID light
        glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BillboardVertex), (void *)24); //color
        glVertexAttribPointer(5, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BillboardVertex), (void *)28); //size, xmod

        glVertexAttribDivisor(0, 1);
        glVertexAttribDivisor(1, 0);
        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, pm->size);
    }

    glDepthMask(GL_TRUE);
}

