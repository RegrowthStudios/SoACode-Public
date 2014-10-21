#include "stdafx.h"
#include "TexturePackLoader.h"
#include "ImageLoader.h"

#include "Options.h"

#include "FileSystem.h"

TexturePackLoader::TexturePackLoader(vg::TextureCache* textureCache) :
    _textureCache(textureCache),
    _hasLoaded(false) {
    // Empty
}

TexturePackLoader::~TexturePackLoader() {
    destroy();
}

void TexturePackLoader::loadAllTextures() {

    // TODO(Ben): Zip file support
    _texturePackPath = "Textures/TexturePacks/" + graphicsOptions.texturePackString + "/";

    ui32 width, height;
    for (auto it = _texturesToLoad.begin(); it != _texturesToLoad.end(); ++it) {
        // Load the data
        std::vector<ui8>* pixelStore = new std::vector<ui8>();
        vg::ImageLoader::loadPng((_texturePackPath + it->first).c_str(), *pixelStore, width, height);
        if (pixelStore->size()) {
            // Add the data to the cache and get non-const reference to pixels
            Pixels* pixels = (Pixels*)(_pixelCache.insert(std::make_pair(it->first, Pixels(pixelStore, width, height))).second);
            // Store the reference to the pixels and samplerstate so we can upload it in uploadTextures
            _texturesToUpload[it->first] = TextureToUpload(pixels, it->second);
        }
        delete pixelStore;
    }

    // Load all the block textures and map to an atlas array
    loadAllBlockTextures();

    // Mark this texture pack as loaded
    _hasLoaded = true;
}

BlockTextureData* TexturePackLoader::getBlockTexture(nString& key) {

    auto it = _blockTextureLoadDatas.find(key);
    if (it == _blockTextureLoadDatas.end()) {
        return nullptr;
    } else {
        return &(it->second);
    }
}

void TexturePackLoader::uploadTextures() {

    // Upload all the non block textures
    for (auto it = _texturesToUpload.begin(); it != _texturesToUpload.end(); ++it) {
        TextureToUpload& texture = it->second;
        _textureCache->addTexture(it->first, texture.pixels->data->data(), texture.pixels->width, texture.pixels->height, texture.samplerState);
    }

    // TODO(Ben): This could be done better
    // Upload all the block textures
    Texture atlasTex;
    atlasTex.width = atlasTex.height = 32 * BLOCK_TEXTURE_ATLAS_WIDTH;

    atlasTex.ID = _textureAtlasStitcher.buildTextureArray();

    blockPack.initialize(atlasTex);
}

void TexturePackLoader::destroy() {
    /// Free stitcher memory
    _textureAtlasStitcher.destroy();
    /// Free all cache memory
    std::map <nString, SamplerState*>().swap(_texturesToLoad);
    std::map <nString, TextureToUpload>().swap(_texturesToUpload);
    std::set <nString>().swap(_blockTexturesToLoad);
    std::set <BlockTextureLayer>().swap(_blockTextureLayers);
    std::vector <BlockLayerLoadData>().swap(_layersToLoad);
    std::map <nString, BlockTextureData>().swap(_blockTextureLoadDatas);

    // Make sure to free all pixel data
    for (auto& pixels : _pixelCache) {
        delete pixels.second.data;
    }
    std::map <nString, Pixels>().swap(_pixelCache);
}

void TexturePackLoader::writeDebugAtlases() {
    int width = 32 * BLOCK_TEXTURE_ATLAS_WIDTH;
    int height = width;

    int pixelsPerPage = width * height * 4;
    ui8 *pixels = new ui8[width * height * 4 * _textureAtlasStitcher.getNumPages()];

    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.ID);
    glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    for (int i = 0; i < _textureAtlasStitcher.getNumPages(); i++) {

        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(pixels + i * pixelsPerPage, width, height, 32, 4 * width, 0xFF, 0xFF00, 0xFF0000, 0x0);
        SDL_SaveBMP(surface, ("atlas" + to_string(i) + "b.bmp").c_str());

    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    delete[] pixels;
}

/// Loads all the block textures
void TexturePackLoader::loadAllBlockTextures() {
    // Used to get the texture pixel dimensions
    ui32 width, height;

    ui8* pixels;

    // Loop through all textures to load
    for (const nString& texturePath : _blockTexturesToLoad) {

        // The struct we need to populate
        BlockTextureData blockTextureLoadData = {};
        BlockTexture blockTexture = {};

        // Search for the tex file first
        nString texFileName = _texturePackPath + texturePath;
        // Convert .png to .tex
        texFileName.replace(texFileName.end() - 4, texFileName.end(), ".tex");

        // Load the tex file (if it exists)
        fileManager.loadTexFile(texFileName, nullptr, &blockTexture);

        // If there wasn't an explicit override base path, we use the texturePath
        if (blockTexture.base.path.empty()) blockTexture.base.path = texturePath;

        // Get pixels for the base texture
        pixels = getPixels(blockTexture.base.path, width, height);
        if (pixels) {
            // Store handle to the layer
            blockTextureLoadData.base = postProcessLayer(blockTexture.base, width, height);
            // Do necesarry postprocessing and add layer to load
            _layersToLoad.emplace_back(pixels, blockTextureLoadData.base);
        }

        // Check if we have an overlay
        if (blockTexture.overlay.path.empty() == false) {
            // Get pixels for the overlay texture
            pixels = getPixels(blockTexture.overlay.path, width, height);
            if (pixels) {
                // Store handle to the layer
                blockTextureLoadData.overlay = postProcessLayer(blockTexture.overlay, width, height);
                // Do necesarry postprocessing and add layer to load
                _layersToLoad.emplace_back(pixels, blockTextureLoadData.overlay);
            }
        }

        // Add it to the list of load datas
        _blockTextureLoadDatas[texturePath] = blockTextureLoadData;
    }

    // Finish by mapping all the blockTextureLoadDatas to atlases
    mapTexturesToAtlases();
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

    PreciseTimer timerb;
    timerb.start();
    BlockTextureLayer* layer;
    // Iterate through all the unique texture layers we need to map
    for (auto it = _blockTextureLayers.begin(); it != _blockTextureLayers.end(); ++it) {
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

    if (it == _pixelCache.end()) {
        // Load the data
        std::vector<ui8>* pixelStore = new std::vector<ui8>();
        vg::ImageLoader::loadPng((_texturePackPath + filePath).c_str(), *pixelStore, width, height);
        if (pixelStore->size()) {
            // Add the data to the cache
            _pixelCache[filePath] = Pixels(pixelStore, width, height);
            return pixelStore->data();
        }
        delete pixelStore;
        return nullptr;
    } else {
        // Return the Cached data
        width = it->second.width;
        height = it->second.height;
        return it->second.data->data();
    }
}