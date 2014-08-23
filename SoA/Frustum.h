#pragma once
#include "OpenGLStructs.h"
//This file is for frustum culling and such

extern double worldFrustum[6][4];
extern double gridFrustum[6][4];

void ExtractFrustum(glm::dmat4 &projectionMatrix, glm::dmat4 &viewMatrix, double frustum[6][4]);

bool PointInFrustum(float x, float y, float z, double frustum[6][4]);

bool SphereInFrustum(float x, float y, float z, float radius, double frustum[6][4]);