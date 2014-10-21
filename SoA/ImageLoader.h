// 
//  ImageLoader.h
//  Vorb Engine
//
//  Created by Ben Arnold on 20 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides an implementation of an static
//  image loader class which handles the loading of image files.
//

#pragma once

#ifndef IMAGELOADER_H_
#define IMAGELOADER_H_

#include "SamplerState.h"

namespace vorb {
namespace core {
namespace graphics {

class ImageLoader
{
public:
    /// Loads a PNG file into a pixel buffer
    /// @param imagePath: Path to the image file
    /// @param pixelStore: Buffer to store the pixel data
    /// @param rWidth: Image width gets stored here
    /// @param rHeight: Image height gets stored here
    /// @param printError: When true, will print error if file not found
    /// @return true on success, false on failure
    static bool loadPng(const cString imagepath, std::vector<ui8>& pixelStore, ui32& rWidth, ui32& rHeight, bool printError = true);
};

}
}
}

// Namespace Alias
namespace vg = vorb::core::graphics;

#endif // IMAGELOADER_H_