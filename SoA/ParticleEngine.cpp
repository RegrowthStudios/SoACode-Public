#include "stdafx.h"
#include "ParticleEngine.h"

#include "Particle.h"
#include "ParticleBatch.h"
#include "ParticleEmitter.h"
#include "SoaOptions.h"

ParticleEngine particleEngine;

ParticleEngine::ParticleEngine() {
    animatedParticles.animated = 1;
    animatedParticles.uvWidth = 1.0f / 5.0f;
}

ParticleEngine::~ParticleEngine() {
    clearEmitters();
}

void ParticleEngine::clearEmitters() {
    //TODO: Fix Race Condition, should only be touched by physics thread
    for(size_t i = 0; i < emitters.size(); i++) {
        delete emitters[i];
    }
    emitters.clear();
}

void ParticleEngine::update(ChunkManager* chunkManager) {
    staticParticles.update();
    animatedParticles.updateAnimated();
    for(size_t i = 0; i < emitters.size();) {
        if(emitters[i]->update(chunkManager)) {

            delete emitters[i];
            emitters[i] = emitters.back();
            emitters.pop_back();
        } else {
            i++;
        }
    }
}

void ParticleEngine::addParticles(ChunkManager* chunkManager, int num, glm::dvec3 pos, int type, double force, int time, int deg, glm::vec4 color, int tex, float life, GLubyte billSize, glm::vec3 expForce) {
    GLubyte Color[4];
    Color[0] = (GLubyte)(color.r);
    Color[1] = (GLubyte)(color.g);
    Color[2] = (GLubyte)(color.b);
    Color[3] = (GLubyte)(color.a);
    if(graphicsOptions.enableParticles) {
        staticParticles.addParticles(chunkManager, num, pos, tex, force, life, billSize, Color, expForce);
    }
}

void ParticleEngine::addAnimatedParticles(ChunkManager* chunkManager, int num, glm::dvec3 pos, int type, double force, int time, int deg, glm::vec4 color, int tex, float life, GLubyte billSize, glm::vec3 expForce) {
    GLubyte Color[4];
    Color[0] = (GLubyte)(color.r);
    Color[1] = (GLubyte)(color.g);
    Color[2] = (GLubyte)(color.b);
    Color[3] = (GLubyte)(color.a);
    //    if (graphicsOptions.enableParticles){
    switch(type) {
    case PARTICLE_EXPLOSION:
        animatedParticles.addParticles(chunkManager, num, pos, tex, force, life, billSize, Color, expForce);
        break;
    case PARTICLE_FIRE:
        animatedParticles.addParticles(chunkManager, num, pos, tex, force, life, billSize, Color, expForce);
        break;
    }
    //    }
}

void ParticleEngine::addAnimatedParticles(ChunkManager* chunkManager, int num, glm::dvec3 pos, ParticleEmitter *emitter) {
    animatedParticles.addParticles(num, pos, emitter);
}

void ParticleEngine::addEmitter(int type, glm::dvec3 pos) {
    emitters.push_back(new ParticleEmitter(pos, type));
}

void ParticleEngine::addEmitter(ParticleEmitter *baseEmitter, glm::dvec3 pos, int blockType) {
    ParticleEmitter* e = new ParticleEmitter();
    *e = *baseEmitter;
    e->position = pos;
    e->blockType = blockType;

    emitters.push_back(e);
}