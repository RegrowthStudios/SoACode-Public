#pragma once
#include "PhysicsBlocks.h"

class ParticleType {
public:
    ParticleType() : tmpTexUnit(-1) {
        color[0] = 255;
        color[1] = 255;
        color[2] = 255;
        color[3] = 255;
    }

    class Animation* animation;
    vg::Texture texture;
    i32 tmpTexUnit;
    ui8 color[4];
};

extern std::vector<ParticleType> particleTypes;

class Particle {
public:
    Particle() {
        light[0] = 0;
        light[1] = 0;
        isYCollidable = 0;
        pc = 0xffff; // 32768 is the max
    }

    // Sort in reverse order : far particles drawn first.
    bool operator < (Particle& that) {
        return this->distance > that.distance;
    }

    f32v3 position;
    f32v3 velocity;
    ui8 light[2];
    ui16 pc;
    ui8 texUnit;
    ui8 texID;
    bool isYCollidable;
    ui8 color[4];
    ui8 size;
    f32 distance;
    i32 type;

    bool billboardUpdate(const std::deque< std::deque< std::deque<class ChunkSlot*> > >& chunkList, f64v3&, class Actor* player);
    bool update();
    bool update(const f64v3& batchPosition);
};