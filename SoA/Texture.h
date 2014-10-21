// 
//  Texture.h
//  Vorb Engine
//
//  Created by Ben Arnold on 20 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides a wrapper for a texture
//
#pragma once

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "stdafx.h"

namespace vorb {
namespace core {
namespace graphics {

// Wrapper struct for a texture
struct Texture {
    Texture() : ID(0), width(0), height(0) {}
    Texture(ui32 TextureID, ui32 Width, ui32 Height) :
        ID(TextureID),
        width(Width),
        height(Height) {
        // Empty
    }
    ui32 ID;
    ui32 width;
    ui32 height;
};

}
}
}

namespace vg = vorb::core::graphics;

#endif // TEXTURE_H_