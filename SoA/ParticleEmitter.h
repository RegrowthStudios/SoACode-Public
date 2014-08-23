#pragma once

class ParticleEmitter {
public:
    ParticleEmitter();
    ParticleEmitter(f64v3 pos, i32 typ);

    i32 update();

    f64v3 position;
    f32v3 initialVel;
    i32 type;
    i32 blockType;
    i32 spawnTimeS, spawnTimeE;
    i32 particleType;
    ui32 minDuration;
    ui32 maxDuration;
    i32 minSizeS, minSizeE;
    i32 maxSizeS, maxSizeE;
    f32v3 posOffset;
    f32v3 rposOffset;

    //EmitterData *emitterData;

    ui32 dt;
};

