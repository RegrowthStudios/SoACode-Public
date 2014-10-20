// 
//  TextureAtlasStitcher.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 19 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides an implementation of a texture
//  atlas stitcher. This class handles the mapping of
//  textures to an array of texture atlases.
//

#pragma once

#include "BlockData.h"

// Texture Atlas Size (Needs to be power of two)
const i32 BLOCK_TEXTURE_ATLAS_WIDTH = 16;
const i32 BLOCK_TEXTURE_ATLAS_SIZE = BLOCK_TEXTURE_ATLAS_WIDTH * BLOCK_TEXTURE_ATLAS_WIDTH;

/// Stores Information About An Atlas Page For Construction Purposes
struct BlockAtlasPage;

struct BlockLayerLoadData {
    BlockLayerLoadData(ui8* Pixels, BlockTextureLayer* Layer) : pixels(Pixels), layer(Layer) {
        // Empty
    }
    BlockTextureLayer* layer;
    ui8* pixels;
};

/// This class handles stitching of block textures to an array of atlases
class TextureAtlasStitcher {
public:
    TextureAtlasStitcher();
    ~TextureAtlasStitcher();
    
    /// Maps a BlockTextureLayer to a position in the atlase array
    /// @param layer: The block texture layer to map
    /// @return The index of the texture start into the atlas array.
    i32 addTexture(const BlockTextureLayer& layer);

    /// Allocates the pixels for the array and fills with the proper data.
    /// Should only be called after all textures are added, before buildTextureArray.
    /// @param layers: Vector of all layer data needed for building pixel data
    /// @param resolution: The horizontal resolution of a single block texture
    void buildPixelData(const std::vector <BlockLayerLoadData>& layers, int resolution);

    /// Builds the OpenGL texture atlas array
    /// @return The ID for the texture array
    ui32 buildTextureArray();

    /// Frees all memory and returns to initial state
    void destroy();

private:
    /// Maps a single block texture to the atlases
    /// @return The index of the texture start into the atlas array.
    i32 mapSingle();

    /// Maps a large box texture to the atlases
    /// @param width: The width of the box
    /// @param height: The height of the box
    /// @return The index of the texture start into the atlas array.
    i32 mapBox(int width, int height);

    /// Maps a contiguous array of single textures to the atlases
    /// @param numTiles: The number of tiles to map
    /// @return The index of the texture start into the atlas array.
    i32 mapContiguous(int numTiles);

    void writeToAtlas(int texIndex, ui8* pixels, int pixelWidth, int pixelHeight);

    void writeToAtlasContiguous(int texIndex, ui8* pixels, int width, int height, int numTiles);

    std::map <BlockTextureLayer, ui32> _textureLayerCache; ///< Cache of texture layer mappings

    std::vector<BlockAtlasPage*> _pages; ///< list of pages

    i32 _bytesPerPixelRow; ///< Number of bytes in a row of pixels
    i32 _bytesPerTileRow; ///< Number of bytes in pixel data per row of textures
    i32 _bytesPerPage; ///< Number of bytes in pixel data per page
    i32 _oldestFreeSlot; ///< The left-most free slot in the atlas array
    i32 _resolution; ///< The horizontal resolution of a single block texture
    ui8* _pixelData; ///< Pointer to the pixel data for the texture array
};