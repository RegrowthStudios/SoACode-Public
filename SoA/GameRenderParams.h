#pragma once

#ifndef GameRenderParams_h__
#define GameRenderParams_h__

#include "stdafx.h"

struct GameRenderParams {
    f32v3 sunlightDirection;
    f32v3 sunlightColor;  
    float sunlightIntensity;
    f32v3 fogColor;
    float fogEnd;
    float fogStart;
    float lightActive;
};

#endif // GameRenderParams_h__