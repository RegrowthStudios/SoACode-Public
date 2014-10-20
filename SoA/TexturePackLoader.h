// 
//  TexturePackLoader.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 19 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides an implementation of a texture
//  pack loader. It handles loading textures and creating
//  texture atlases.
//

#pragma once

#ifndef TEXTUREPACKLOADER_H_
#define TEXTUREPACKLOADER_H_

#include "TextureAtlasStitcher.h"
#include "BlockData.h"

#include <set>

struct BlockTextureLoadData {
    BlockTextureLayer* base;
    BlockTextureLayer* overlay;
    BlendType blendMode;
};

// TODO(Ben): Make sure to save VRAM diagnostics!
class TexturePackLoader
{
public:
    TexturePackLoader();

    /// Adds a block texture to be loaded
    /// @param filePath: The path of the block texture
    void addBlockTexture(const std::string& filePath) { _blockTexturesToLoad.insert(filePath); }

    /// Loads all textures added to the texture pack and stores them
    /// but does not construct the texture atlases
    void loadAllTextures();

    /// Creates the texture atlases. Must be called after loadAllTextures.
    void createTextureAtlases();

    /// Frees all resources
    void destroy();

private:

    /// Does postprocessing to the layer and adds it to _blockTextureLayers
    /// @param layer: The block texture layer to process
    /// @param width: The width of the texture in pixels
    /// @param height: The height of the texture in pixels
    /// @return Pointer to the BlockTextureLayer that gets stored
    BlockTextureLayer* postProcessLayer(BlockTextureLayer& layer, ui32 width, ui32 height);

    /// Maps all the layers in _blockTextureLayers to an atlas array
    void mapTexturesToAtlases();

    /// Gets the pixels for an image by loading it or grabbing it from cache.
    /// @param filePath: The texture path
    /// @param width: Texture width gets stored here
    /// @param height: Texture height gets stored here
    /// @return Pointer to the pixel data
    ui8* getPixels(const nString& filePath, ui32& width, ui32& height);

    std::set <nString> _blockTexturesToLoad; ///< Set of all unique block texture paths to load

    std::set <BlockTextureLayer> _blockTextureLayers; ///< Set of all unique block texture layers to load

    std::vector <BlockLayerLoadData> _layersToLoad;

    std::map <nString, BlockTextureLoadData> _blockTextureLoadDatas; ///< Map of all texture datas we need to load

    /// Struct used for cacheing pixels
    struct Pixels {
        Pixels(ui8* Data, ui32 Width, ui32 Height) : data(Data), width(Width), height(Height) {
            // Empty
        }
        ui8* data;
        ui32 width;
        ui32 height;
    };

    std::map <nString, Pixels> _pixelCache; ///< Cache of texture pixel data

    TextureAtlasStitcher _textureAtlasStitcher; ///< Class responsible for doing the mapping to the atlas array

    bool _hasLoaded; ///< True after loadAllTextures finishes
};

#endif // TEXTUREPACKLOADER_H_