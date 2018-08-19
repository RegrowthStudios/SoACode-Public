//
// textureUtils.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 2 Mar 2016
// Copyright 2014 Regrowth Studios
// MIT License
//
// Summary:
// 
//

#pragma once

#ifndef textureUtils_h__
#define textureUtils_h__

#include "Constants.h"

#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/Texture.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/types.h>
#include <chrono>
#include <cstdio>
#include <ctime> 
#include <iomanip>
#include <sstream>
#include <string> 

inline void initSinglePixelTexture(VGTexture& texture, color4 pixel) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixel[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
};


#endif // textureUtils_h__
