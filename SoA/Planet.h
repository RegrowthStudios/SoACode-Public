#pragma once
#include <list>
#include <set>
#include <deque>
#include <queue>

#include "Biome.h"
#include "OpenGLStructs.h"
#include "Texture2d.h"
#include "WorldStructs.h"

#define P_TOP 0
#define P_LEFT 1
#define P_RIGHT 2
#define P_FRONT 3
#define P_BACK 4
#define P_BOTTOM 5

class TreeType;
class PlantType;
class Camera;

class Atmosphere {
public:
    Atmosphere();

    void initialize(nString filePath, f32 PlanetRadius);
    void draw(f32 theta, const f32m4& MVP, f32v3 lightPos, const f64v3& ppos);

    std::vector<ColorVertex> vertices;
    std::vector<ui16> indices;
    ui32 vboID, vbo2ID, vboIndexID, vboIndexID2;
    ui32 indexSize;
    f64 radius;
    f64 planetRadius;

    f32 m_fWavelength[3], m_fWavelength4[3];
    f32 m_Kr;        // Rayleigh scattering constant
    f32 m_Km;        // Mie scattering constant
    f32 m_ESun;        // Sun brightness constant
    f32 m_g;        // The Mie phase asymmetry factor
    f32 m_fExposure;
    f32 m_fRayleighScaleDepth;
    f32 fSamples;
    i32 nSamples;
    i32 debugIndex;
};

class Planet {
public:
    Planet();
    ~Planet();

};
