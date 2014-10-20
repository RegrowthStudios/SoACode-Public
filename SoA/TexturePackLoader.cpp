#include "stdafx.h"
#include "TexturePackLoader.h"

#include "FileSystem.h"


TexturePackLoader::TexturePackLoader() :
    _hasLoaded(false) {
}

void TexturePackLoader::loadAllTextures() {

    // Used to get the texture pixel dimensions
    ui32 width, height;

    ui8* pixels;

    // Loop through all textures to load
    for (const nString& texturePath : _blockTexturesToLoad) {

        // The struct we need to populate
        BlockTextureLoadData blockTextureLoadData = {};
        BlockTexture blockTexture = {};

        // Search for the tex file first
        nString texFileName = texturePath;
        // Convert .png to .tex
        texFileName.replace(texFileName.end() - 4, texFileName.end(), ".tex");

        // Load the tex file (if it exists)
        fileManager.loadTexFile(texFileName, nullptr, &blockTexture);

        // If there wasn't an explicit override base path, we use the texturePath
        if (blockTexture.base.path.empty()) blockTexture.base.path = texturePath;
        
        // Get pixels for the base texture
        pixels = getPixels(blockTexture.base.path, width, height);
        // Store handle to the layer
        blockTextureLoadData.base = postProcessLayer(blockTexture.base, width, height);
        // Do necesarry postprocessing and add layer to load
        _layersToLoad.emplace_back(pixels, blockTextureLoadData.base);

        // Check if we have an overlay
        if (blockTexture.overlay.path.empty() == false) {
            
            // Get pixels for the overlay texture
            pixels = getPixels(blockTexture.overlay.path, width, height);
            // Store handle to the layer
            blockTextureLoadData.overlay = postProcessLayer(blockTexture.overlay, width, height);
            // Do necesarry postprocessing and add layer to load
            _layersToLoad.emplace_back(pixels, blockTextureLoadData.overlay);
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

    // temporary
    TextureInfo atlasTex;
    atlasTex.width = atlasTex.height = 32 * BLOCK_TEXTURE_ATLAS_WIDTH;

    atlasTex.ID = _textureAtlasStitcher.buildTextureArray();

    blockPack.initialize(atlasTex);
}

void TexturePackLoader::destroy() {
    /// Free stitcher memory
    _textureAtlasStitcher.destroy();
    /// Free all cache memory
    std::set <nString>().swap(_blockTexturesToLoad);
    std::set <BlockTextureLayer>().swap(_blockTextureLayers);
    std::vector <BlockLayerLoadData>().swap(_layersToLoad);
    std::map <nString, BlockTextureLoadData>().swap(_blockTextureLoadDatas);
    std::map <nString, Pixels>().swap(_pixelCache);
}

void TexturePackLoader::writeDebugAtlases() {
    int width = 32 * BLOCK_TEXTURE_ATLAS_WIDTH;
    int height = width;

    int pixelsPerPage = width * height * 4;
    ui8 *pixels = new ui8[width * height * 4 * _textureAtlasStitcher.getNumPages()];
    ui8 *flip = new ui8[pixelsPerPage];

    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.ID);
    glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    for (int i = 0; i < _textureAtlasStitcher.getNumPages(); i++) {

        int k = height - 1;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width * 4; x++){
                flip[y * 4 * width + x] = pixels[k * 4 * width + x + i * pixelsPerPage];
            }
            k--;
        }

        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(flip, width, height, 32, 4 * width, 0xFF, 0xFF00, 0xFF0000, 0x0);
        SDL_SaveBMP(surface, ("atlas" + to_string(i) + ".bmp").c_str());

    }
    delete[] pixels;
    delete[] flip;
}

BlockTextureLayer* TexturePackLoader::postProcessLayer(BlockTextureLayer& layer, ui32 width, ui32 height) {
    // Need to set up numTiles and totalWeight for RANDOM method
    if (layer.method == ConnectedTextureMethods::RANDOM) {
        layer.numTiles = width / height;
        if (layer.weights.length() == 0) {
            layer.totalWeight = layer.numTiles;
        }
    }

    // Insert the texture layer into the atlas mapping set and return pointer
    return (BlockTextureLayer*)&(*_blockTextureLayers.insert(layer).first);
}

void TexturePackLoader::mapTexturesToAtlases() {
    BlockTextureLayer* layer;
    // Iterate through all the unique texture layers we need to map
    for (auto it = _blockTextureLayers.begin(); it != _blockTextureLayers.end(); it++) {
        // Get a non-const pointer to the data
        layer = (BlockTextureLayer*)&(*it);
        // Map the texture layer to an atlas position
        layer->textureIndex = _textureAtlasStitcher.addTexture(*it);
    }

    // Build the pixel data for the texture array
    _textureAtlasStitcher.buildPixelData(_layersToLoad, 32);
}

ui8* TexturePackLoader::getPixels(const nString& filePath, ui32& width, ui32& height) {
   
    // Check the cache
    auto& it = _pixelCache.find(filePath);
    if (it != _pixelCache.end()) {
        // Load the data
        ui8* data = loadPNG(filePath.c_str(), width, height);
        if (data) {
            // Add the data to the cache
            _pixelCache[filePath] = Pixels(data, width, height);
        }
        return data;
    } else {
        // Return the Cached data
        width = it->second.width;
        height = it->second.height;
        return it->second.data;
    }
}