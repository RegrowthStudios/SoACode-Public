///
/// OrbitComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Renders orbit components
///

#pragma once

#ifndef OrbitComponentRenderer_h__
#define OrbitComponentRenderer_h__

#include <Vorb/VorbPreDecl.inl>

class SpaceSystem;
struct OrbitComponent;
struct NamePositionComponent;

DECL_VG(class GLProgram)

class OrbitComponentRenderer {
public:
    /// Draws the ellipse
    void drawPath(OrbitComponent& cmp, vg::GLProgram* colorProgram, const f32m4& WVP, NamePositionComponent* npComponent,
                  const f64v3& camPos, float alpha, NamePositionComponent* parentNpComponent = nullptr);
private:
    void OrbitComponentRenderer::generateOrbitEllipse(OrbitComponent& cmp, vg::GLProgram* colorProgram);
};

#endif // OrbitComponentRenderer_h__