#pragma once
#include "Particle.h"

#include <Vorb/VorbPreDecl.inl>

class ParticleEmitter;
class ParticleMesh;
class ParticleEngine;

DECL_VG(class GLProgram);

class ParticleBatch {
public:
    ParticleBatch();
    ~ParticleBatch();

    inline int findUnusedParticle();
    void addParticles(ChunkManager* chunkManager, int num, glm::dvec3 pos, int tex, double force, float life, GLubyte billSize, GLubyte color[4], glm::vec3 extraForce);
    void addParticles(int num, glm::dvec3 pos, ParticleEmitter* emitter);

    //returns 1 to indicate destroy
    int update();
    int updateAnimated();

    static void draw(vg::GLProgram* program, ParticleMesh *pm, glm::dvec3 &PlayerPos, glm::mat4 &VP);
    static void drawAnimated(vg::GLProgram* program, ParticleMesh *pm, glm::dvec3 &PlayerPos, glm::mat4 &VP);

    int size;
    int lastAddedParticle;
    float uvWidth;
    glm::dvec3 position; //moving origin
    bool animated;
    float lifes[maxParticles];
    Particle particles[maxParticles];

protected:
    ParticleMesh *mesh;
    static f32m4 worldMatrix;
};