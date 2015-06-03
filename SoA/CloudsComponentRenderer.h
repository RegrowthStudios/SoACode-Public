#pragma once

#include <Vorb/ecs/ECS.h>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/gtypes.h>
#include <Vorb/graphics/GLProgram.h>

struct AtmosphereComponent;
struct AxisRotationComponent;
struct CloudsComponent;
struct SpaceLightComponent;

class CloudsComponentRenderer
{
public:
    CloudsComponentRenderer();
    ~CloudsComponentRenderer();

    void draw(const CloudsComponent& cCmp,
              const f32m4& VP,
              const f32v3& relCamPos,
              const f32v3& lightDir,
              const f32 zCoef,
              const SpaceLightComponent* spComponent,
              const AxisRotationComponent& arComponent,
              const AtmosphereComponent& aCmp);
    void disposeShader();
private:
    void buildMesh();

    vg::GLProgram m_program;
    VGBuffer m_icoVbo = 0;
    VGIndexBuffer m_icoIbo = 0;
    VGVertexArray m_vao = 0;
    int m_numIndices = 0;
};

