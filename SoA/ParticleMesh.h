#pragma once
#include "WorldStructs.h"

static const GLubyte particleUVs[12] = {255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255};

struct ParticleMesh {
    ParticleMesh(): uvBufferID(0), billboardVertexBufferID(0), vecIndex(-1), size(0), animated(0) {}

    GLuint uvBufferID, billboardVertexBufferID;
    int vecIndex, size, X, Y, Z, type;
    std::vector <int> usedParticles;
    bool animated;
};

struct ParticleMeshMessage {
    ParticleMeshMessage(): mesh(NULL) {}

    ParticleMesh *mesh;
    std::vector <BillboardVertex> verts;
    std::vector <int> usedParticles;
    int size, X, Y, Z;
};