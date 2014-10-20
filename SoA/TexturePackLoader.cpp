#include "stdafx.h"
#include "TexturePackLoader.h"

#include "FileSystem.h"


TexturePackLoader::TexturePackLoader() :
    _hasLoaded(false) {
}

void TexturePackLoader::loadAllTextures() {

    // Used to get the texture pixel dimensions
    ui32 width, height;

    // Loop through all textures to load
    for (const nString& texturePath : _blockTexturesToLoad) {

        // The struct we need to populate
        BlockTextureLoadData blockTextureLoadData = {};
        BlockTexture& blockTexture = blockTextureLoadData.texture;

        // Search for the tex file first
        nString texFileName = texturePath;
        // Convert .png to .tex
        texFileName.replace(texFileName.end() - 4, texFileName.end(), ".tex");

        // Load the tex file (if it exists)
        fileManager.loadTexFile(texFileName, nullptr, &blockTexture);

        // If there wasn't an explicit override base path, we use the texturePath
        if (blockTexture.base.path.empty()) blockTexture.base.path = texturePath;
        
        // Get pixels for the base texture
        blockTextureLoadData.basePixels = getPixels(blockTexture.base.path, width, height);
        // Do necesarry postprocessing
        postProcessLayer(blockTexture.base, width, height);

        // Check if we have an overlay
        if (blockTexture.overlay.path.empty() == false) {
            
            // Get pixels for the overlay texture
            blockTextureLoadData.overlayPixels = getPixels(blockTexture.overlay.path, width, height);
            // Do necesarry postprocessing
            postProcessLayer(blockTexture.overlay, width, height);
        }

        // Add it to the list of load datas
        _blockTextureLoadDatas[texturePath] = blockTextureLoadData;
    }

    // Finish by mapping all the blockTextureLoadDatas to atlases
    mapTexturesToAtlases();

    // Mark this texture pack as loaded
    _hasLoaded = true;
}

void TexturePackLoader::createTextureAtlases() {

}

void TexturePackLoader::postProcessLayer(BlockTextureLayer& layer, ui32 width, ui32 height) {
    // Need to set up numTiles and totalWeight for RANDOM method
    if (layer.method == ConnectedTextureMethods::RANDOM) {
        layer.numTiles = width / height;
        if (layer.weights.length() == 0) {
            layer.totalWeight = layer.numTiles;
        }
    }
    // Insert the texture layer into the atlas mapping set
    _blockTextureLayers.insert(layer);
}

void TexturePackLoader::mapTexturesToAtlases() {
    i32 index;
    // Iterate through all the unique texture layers we need to map
    for (const BlockTextureLayer& layer : _blockTextureLayers) {
        index = textureAtlasStitcher.addTexture(layer);
    }
}

//TODO(Ben): Cache the width and height too!!!
ui8* TexturePackLoader::getPixels(const nString& filePath, ui32& width, ui32& height) {
    ui8* data;

    // Check the cache
    auto& it = _pixelCache.find(filePath);
    if (it != _pixelCache.end()) {
        // Load the data
        data = loadPNG(filePath.c_str(), width, height);
        if (data) {
            // Add the data to the cache
            _pixelCache[filePath] = data;
        }
        return data;
    } else {
        // Return the Cached data
        return it->second;
    }
}