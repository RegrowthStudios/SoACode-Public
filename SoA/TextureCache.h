// 
//  TextureCache.h
//  Vorb Engine
//
//  Created by Ben Arnold on 20 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides an implementation of a TextureCache
//  which handles loading and cacheing of textures.
//

#pragma once

#ifndef TEXTURECACHE_H_
#define TEXTURECACHE_H_

#include <unordered_map>
#include <map>
#include <vector>

#include "ImageLoader.h"

namespace vorb {
namespace core {
namespace graphics {

class TextureCache
{
public:
    TextureCache();
    ~TextureCache();

    /// Finds a texture if it exists in the cache
    /// @param filePath: The path of the texture
    /// @return the texture ID or 0 if it doesn't exist
    ui32 findTexture(const nString& filePath);

    /// Returns the file path associated with the texture
    /// @param textureID: The ID of the texture
    /// @return The filepath or empty string if it doesn't exist
    nString getTexturePath(ui32 textureID);

    /// Loads and uploads a png texture and adds it to the cache or returns
    /// an existing texture ID if it already exists in the cache
    /// @param filePath: The file path of the texture
    /// @param samplingParameters: The texture sampler parameters
    /// @param mipmapLevels: The max number of mipmap levels
    /// @return The texture ID or 0 if loading fails
    ui32 addTexture(const nString& filePath,
                    SamplerState* samplingParameters = &SamplerState::LINEAR_WRAP_MIPMAP,
                    i32 mipmapLevels = INT_MAX);

    /// Uploads a png texture and adds it to the cache
    /// an existing texture ID if it already exists in the cache
    /// @param filePath: The path of the texture
    /// @param pixels: The pixel data
    /// @param width: The texture width in pixels
    /// @param height: The texture height in pixels
    /// @param samplingParameters: The texture sampler parameters
    /// @param mipmapLevels: The max number of mipmap levels
    /// @return The texture ID or 0 if loading fails
    ui32 addTexture(const nString& filePath,
                    const ui8* pixels,
                    ui32 width,
                    ui32 height,
                    SamplerState* samplingParameters = &SamplerState::LINEAR_WRAP_MIPMAP,
                    i32 mipmapLevels = INT_MAX);

    /// Adds a texture to the cache
    /// @param filePath: The path of the texture
    /// @param textureID: The opengGL texture ID
    void addTexture(const nString& filePath, ui32 textureID);

    /// Frees a texture from the cache
    /// @param filePath: The path of the texture to free
    void freeTexture(const nString& filePath);

    /// Frees a texture from the cache
    /// @param textureID: The ID of the texture to free
    void freeTexture(ui32 textureID);

    /// Frees all textures
    void destroy();

private:

    /// Inserts a texture into the cache
    void insertTexture(const nString& filePath, ui32 textureID);

    /// We store two maps here so that users can free textures using either the ID or filePath
    std::unordered_map <nString, ui32> _textureStringMap; ///< Textures store here keyed on filename
    std::map <ui32, std::unordered_map <nString, ui32>::iterator> _textureIdMap; ///< Textures are stored here keyed on ID
};

}
}
}

namespace vg = vorb::core::graphics;

#endif // TEXTURECACHE_H_

