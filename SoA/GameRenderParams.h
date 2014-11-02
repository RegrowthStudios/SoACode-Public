#pragma once

#ifndef GameRenderParams_h__
#define GameRenderParams_h__

#include "stdafx.h"

class GameRenderParams {
public:
    void calculateParams(const f64v3& worldCameraPos, bool isUnderwater);

    f32v3 sunlightDirection;
    f32v3 sunlightColor;  
    float sunlightIntensity;
    f32v3 fogColor;
    float fogEnd;
    float fogStart;
    float lightActive;
private:
    void calculateFog(float theta, bool isUnderwater);
    void calculateSunlight(float theta);
};

#endif // GameRenderParams_h__