#pragma once
#include "Particles.h"

class ParticleEmitter;

enum ParticleTypes {
    PARTICLE_NONE,
    PARTICLE_EXPLOSION,
    PARTICLE_FIRE,
    PARTICLE_LINEAR
};
enum EmitterTypes {
    EMITTER_DEFAULT,
    EMITTER_STATIC,
    EMITTER_LINEAR
};

class ParticleEngine {
public:
    ParticleEngine();
    ~ParticleEngine();

    void clearEmitters();

    void update(ChunkManager* chunkManager);
    void addParticles(ChunkManager* chunkManager, i32 num, f64v3 pos, i32 type, f64 force, i32 time, i32 deg, f32v4 color, i32 tex, f32 life, ui8 billSize, f32v3 expForce = f32v3(0.0f));
    void addAnimatedParticles(ChunkManager* chunkManager, i32 num, f64v3 pos, i32 type, f64 force, i32 time, i32 deg, f32v4 color, i32 tex, f32 life, ui8 billSize, f32v3 expForce = f32v3(0.0f));
    void addAnimatedParticles(ChunkManager* chunkManager, i32 num, f64v3 pos, ParticleEmitter* emitter);

    void addEmitter(i32 type, f64v3 pos);
    void addEmitter(ParticleEmitter* baseEmitter, f64v3 pos, i32 blockType);

    std::vector<ParticleEmitter*> emitters;

    ParticleBatch staticParticles;
    ParticleBatch animatedParticles;
};

extern ParticleEngine particleEngine;