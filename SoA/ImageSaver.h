// 
//  ImageSaver.h
//  Vorb Engine
//
//  Created by Ben Arnold on 20 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides an implementation of an static
//  image saver class which handles the saving of image files.
//

#pragma once

#ifndef IMAGESAVER_H_
#define IMAGESAVER_H_

namespace vorb {
namespace core {
namespace graphics {

class ImageSaver
{
public:
    /// Saves a PNG file in RGBA format
    /// @param fileName: The name of the file to save
    /// @param width: The width of the image data
    /// @param height: the height of the image data
    /// @param imgData: The pixels to save. Should be width * height * 4 in size 
    /// @return true on success, false on failure
    static bool savePng(nString fileName, ui32 width, ui32 height, std::vector<ui8> imgData);

};

}
}
}

// Namespace Alias
namespace vg = vorb::core::graphics;

#endif // IMAGESAVER_H_