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

#include "gtypes.h"

namespace vorb {
    namespace core {
        namespace graphics {
            /// Wrapper struct for a texture
            struct Texture {
            public:
                Texture(VGTexture id = 0, ui32 w = 0, ui32 h = 0) :
                    id(id),
                    width(w),
                    height(h) {
                    // Empty
                }

                VGTexture id = 0; ///< OpenGL texture ID
                ui32 width = 0; ///< Texture width in pixels
                ui32 height = 0; ///< Texture height in pixels
            };
        }
    }
}
namespace vg = vorb::core::graphics;

#endif // TEXTURE_H_