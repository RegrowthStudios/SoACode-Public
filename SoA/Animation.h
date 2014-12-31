#pragma once

#ifndef Animation_h_
#define Animation_h_

#include <Vorb/Texture.h>

class Animation {
public:
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