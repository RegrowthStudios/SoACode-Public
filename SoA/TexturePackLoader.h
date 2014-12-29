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

#include <set>

#include <Vorb/IOManager.h>
#include <Vorb/TextureCache.h>

#include "TextureAtlasStitcher.h"
#include "BlockPack.h"

/// Information stored in pack.yml
struct TexturePackInfo {
    nString name;
    ui32 resolution;
    nString description;
};

struct BlockTextureData {
    BlockTextureLayer* base;
    BlockTextureLayer* overlay;
    BlendType blendMode;
};

class ZipFile;

/// This class is designed so that textures are loaded in two passes.
/// First all textures that should be loaded must be registered.
/// In the first pass, textures are loaded into buffers with loadAllTextures().
/// In the second pass, textures are uploaded to the GPU with uploadTextures()
/// TODO(Ben): Split this up with an additional class TexturePack
class TexturePackLoader
{
public:
    /// Constructor
    /// @param textureCache: The texture cache where textures will be stored
    TexturePackLoader(vg::TextureCache* textureCache);
    ~TexturePackLoader();

    /// Register a texture to be loaded
    /// @param filePath: The path of the texture
    /// @param ss: The sampler state for loading the texture
    void registerTexture(const nString& filePath,
                         SamplerState* ss = &SamplerState::LINEAR_CLAMP_MIPMAP) {
        _texturesToLoad[filePath] = ss;
    }

    /// Register a block texture to be loaded
    /// @param filePath: The path of the block texture
    void registerBlockTexture(const nString& filePath) { _blockTexturesToLoad.insert(filePath); }

    /// Loads all textures added to the texture pack and stores them
    /// but does not construct the texture atlases
    /// @param texturePackPath: Path to the texture pack
    void loadAllTextures(const nString& texturePackPath);

    /// Gets a BlockTexture, which contains the information about a block texture
    /// @param texturePath: The path to the texture to look up
    /// @param texture: The block texture that will be filled with data
    void getBlockTexture(nString& texturePath, BlockTexture& texture);

    /// Creates the texture atlases and uploads textures to the GPU.
    /// Must be called after loadAllTextures. Will free pixel memory and
    /// clear arrays other than _texturesToLoad and _blockTexturesToLoad
    void uploadTextures();

    /// Sets the textures for blocks.
    /// Should be called after uploadTextures()
    void setBlockTextures(BlockPack& blocks);

    /// Loads the pack file for this texture pack
    /// @param filePath: The path of the pack file
    /// @return The texture pack info
    TexturePackInfo loadPackFile(const nString& filePath);

    /// Gets a color map, does not load the color map if it doesn't exist
    /// @param name: The name of the color map, should be a filepath if not a default
    /// @return Pointer to the color map data
    ColorRGB8* getColorMap(const nString& name);
    /// Gets a color map, does not load the color map if it doesn't exist
    /// @param name: The name of the color map, should be a filepath if not a default
    /// @return Pointer to the color map data
    ColorRGB8* getColorMap(ui32 index) { return _blockColorMaps.at(index); }
    /// Gets a color map, loads the color map if it doesn't exist
    /// @param name: The name of the color map, should be a filepath if not a default
    /// @return The color map index for fast lookup
    ui32 getColorMapIndex(const nString& name);

    /// Clears caches of textures to load. Use this before you want to
    /// reload a different set of block textures. If you don't call this
    /// after loading a pack, subsequent calls to loadAllTextures will load
    /// the same textures.
    void clearToloadCaches();

    /// Frees all resources
    void destroy();

    /// Dumps all the atlas pages to file
    void writeDebugAtlases();

    /// Getters
    TexturePackInfo getTexturePackInfo()  const { return _packInfo; }
    void getColorMapColor(ui32 colorMapIndex, ColorRGB8& color, int temperature, int rainfall) {
        color = _blockColorMaps[colorMapIndex][rainfall * 256 + temperature];
    }

private:

    /// Loads all the block textures
    void loadAllBlockTextures();

    /// Loads a .tex file
    /// @param fileName: Path of the tex file
    /// @param zipFile: A zip file. nullptr if it is not a zip
    /// @param rv: the return value block texture
    /// @return Returns true if the file opens, false otherwise
    bool loadTexFile(nString fileName, ZipFile *zipFile, BlockTexture* rv);

    /// Does error checking and postprocessing to the layer and adds it to 
    /// _blockTextureLayers and _layersToLoad.
    /// @param layer: The block texture layer to process
    /// @param width: The width of the texture in pixels
    /// @param height: The height of the texture in pixels
    /// @return Pointer to the BlockTextureLayer that gets stored
    BlockTextureLayer* postProcessLayer(ui8* pixels, BlockTextureLayer& layer, ui32 width, ui32 height);

    /// Maps all the layers in _blockTextureLayers to an atlas array
    void mapTexturesToAtlases();

    /// Gets the pixels for an image by loading it or grabbing it from cache.
    /// @param filePath: The texture path
    /// @param width: Texture width gets stored here
    /// @param height: Texture height gets stored here
    /// @return Pointer to the pixel data
    ui8* getPixels(const nString& filePath, ui32& width, ui32& height);

    /// Struct used for cacheing pixels
    struct Pixels {
        Pixels() : data(nullptr), width(0), height(0) {};
        Pixels(std::vector<ui8>* Data, ui32 Width, ui32 Height) : data(Data), width(Width), height(Height) {
            // Empty
        }
        std::vector<ui8>* data;
        ui32 width;
        ui32 height;
    };

    /// Struct used for storing texture upload state
    struct TextureToUpload {
        TextureToUpload() : pixels(nullptr), samplerState(nullptr) {};
        TextureToUpload(Pixels* p, SamplerState* ss) :
            pixels(p), samplerState(ss) {
            // Empty
        }
        Pixels* pixels;
        SamplerState* samplerState;
    };

    std::map <nString, SamplerState*> _texturesToLoad; ///< Map of all unique non-block texture paths to load
    std::map <nString, TextureToUpload> _texturesToUpload; ///< Map of textures to upload

    std::set <nString> _blockTexturesToLoad; ///< Set of all unique block texture paths to load

    std::set <BlockTextureLayer> _blockTextureLayers; ///< Set of all unique block texture layers to load

    std::vector <BlockLayerLoadData> _layersToLoad; ///< Vector of all layers we need to load

    std::map <nString, BlockTextureData> _blockTextureLoadDatas; ///< Map of all texture datas we need to load

    std::map <nString, Pixels> _pixelCache; ///< Cache of texture pixel data

    TextureAtlasStitcher _textureAtlasStitcher; ///< Class responsible for doing the mapping to the atlas array

    nString _texturePackPath; ///< Path for the texture pack

    vg::TextureCache* _textureCache; ///< Cache for storing non-block textures
    
    TexturePackInfo _packInfo; ///< Information of the texture pack, such as resolution and name

    std::map <nString, ui32> _blockColorMapLookupTable; ///< For looking up the index for the block color maps
    std::vector <ColorRGB8*> _blockColorMaps; ///< Storage for the block color maps

    bool _hasLoaded; ///< True after loadAllTextures finishes

    IOManager _ioManager; ///< Provides IO utilities

    int _numAtlasPages;
};

#endif // TEXTUREPACKLOADER_H_