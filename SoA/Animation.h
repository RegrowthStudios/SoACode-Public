#pragma once

#ifndef Animation_h_
#define Animation_h_

#include <Vorb/graphics/Texture.h>

struct Animation {
    Animation() : fadeOutBegin(INT_MAX) {}
    int duration;
    int fadeOutBegin;
    int repetitions;
    int xFrames;
    int yFrames;
    int frames;
    vg::Texture texture;
};

#endif // Animation_h_