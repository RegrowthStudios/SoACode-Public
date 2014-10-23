#include "stdafx.h"
#include "TexturePackLoader.h"
#include "ImageLoader.h"

#include "Options.h"

#include "FileSystem.h"

// Used for error checking
#define CONNECTED_WIDTH 12
#define CONNECTED_HEIGHT 4
#define GRASS_WIDTH 3
#define GRASS_HEIGHT 3
#define HORIZONTAL_WIDTH 4
#define HORIZONTAL_HEIGHT 1

TexturePackLoader::TexturePackLoader(vg::TextureCache* textureCache) :
    _textureCache(textureCache),
    _hasLoaded(false),
    _resolution(32),
    _numAtlasPages(0) {
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
            Pixels* pixels = &(_pixelCache.insert(std::make_pair(it->first, Pixels(pixelStore, width, height))).first->second);
            // Store the reference to the pixels and samplerstate so we can upload it in uploadTextures
            _texturesToUpload[it->first] = TextureToUpload(pixels, it->second);
        } else {
            delete pixelStore;
        }
    }

    // Load all the block textures and map to an atlas array
    loadAllBlockTextures();

    // Mark this texture pack as loaded
    _hasLoaded = true;
}

void TexturePackLoader::getBlockTexture(nString& key, BlockTexture& texture) {

    auto it = _blockTextureLoadDatas.find(key);
    if (it == _blockTextureLoadDatas.end()) {
        texture = BlockTexture();
    } else {
        if (it->second.base) {
            texture.base = *it->second.base;
        }
        if (it->second.overlay) {
            texture.overlay = *it->second.overlay;
        }
        texture.blendMode = it->second.blendMode;
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
    vg::Texture atlasTex;
    atlasTex.width = atlasTex.height = _resolution * BLOCK_TEXTURE_ATLAS_WIDTH;

    atlasTex.ID = _textureAtlasStitcher.buildTextureArray();

    blockPack.initialize(atlasTex);

    // Get the number of atlas pages before freeing atlas
    _numAtlasPages = _textureAtlasStitcher.getNumPages();

    /// Free stitcher memory
    _textureAtlasStitcher.destroy();
    /// Free cache memory used in loading
    std::map <nString, TextureToUpload>().swap(_texturesToUpload);
    std::vector <BlockLayerLoadData>().swap(_layersToLoad);

    // Make sure to free all pixel data
    for (auto& pixels : _pixelCache) {
        delete pixels.second.data;
    }
    std::map <nString, Pixels>().swap(_pixelCache);
}

void TexturePackLoader::setBlockTextures(std::vector<Block>& blocks) {
    // Initialize all the textures for blocks.
    for (size_t i = 0; i < blocks.size(); i++) {
        blocks[i].InitializeTexture();
    }
    // Since we have set the blocks, we can now free the lookup maps and 
    // blockTextureLayers cache
    std::map <nString, BlockTextureData>().swap(_blockTextureLoadDatas);
    std::set <BlockTextureLayer>().swap(_blockTextureLayers);
}

void TexturePackLoader::clearToloadCaches() {
    std::map <nString, SamplerState*>().swap(_texturesToLoad);
    std::set <nString>().swap(_blockTexturesToLoad);
}

void TexturePackLoader::destroy() {
    /// Free stitcher memory
    _textureAtlasStitcher.destroy();
    /// Free all cache memory
    std::map <nString, TextureToUpload>().swap(_texturesToUpload);
    std::vector <BlockLayerLoadData>().swap(_layersToLoad);
    
    std::map <nString, SamplerState*>().swap(_texturesToLoad);
    std::set <nString>().swap(_blockTexturesToLoad);
    
    std::map <nString, BlockTextureData>().swap(_blockTextureLoadDatas);
    std::set <BlockTextureLayer>().swap(_blockTextureLayers);

    // Make sure to free all pixel data
    for (auto& pixels : _pixelCache) {
        delete pixels.second.data;
    }
    std::map <nString, Pixels>().swap(_pixelCache);
    _numAtlasPages = 0;
}


void TexturePackLoader::writeDebugAtlases() {
    int width = _resolution * BLOCK_TEXTURE_ATLAS_WIDTH;
    int height = width;

    int pixelsPerPage = width * height * 4;
    ui8 *pixels = new ui8[width * height * 4 * _numAtlasPages];

    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.ID);
    glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    for (int i = 0; i < _numAtlasPages; i++) {

        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(pixels + i * pixelsPerPage, width, height, _resolution, 4 * width, 0xFF, 0xFF00, 0xFF0000, 0x0);
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

    // Free _blockTextureLayers in case a pack has been loaded before
    std::set <BlockTextureLayer>().swap(_blockTextureLayers);

    // Loop through all textures to load
    for (const nString& texturePath : _blockTexturesToLoad) {

        // The struct we need to populate
        BlockTextureData blockTextureLoadData = {};
        BlockTexture blockTexture;

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
        // Store handle to the layer, do postprocessing, add layer to load
        blockTextureLoadData.base = postProcessLayer(pixels, blockTexture.base, width, height);

        // Check if we have an overlay
        if (blockTexture.overlay.path.empty() == false) {
            // Get pixels for the overlay texture
            pixels = getPixels(blockTexture.overlay.path, width, height);
            // Store handle to the layer, do postprocessing, add layer to load
            blockTextureLoadData.overlay = postProcessLayer(pixels, blockTexture.overlay, width, height);

        }

        // Add it to the list of load datas
        _blockTextureLoadDatas[texturePath] = blockTextureLoadData;
    }

    // Finish by mapping all the blockTextureLoadDatas to atlases
    mapTexturesToAtlases();
}

BlockTextureLayer* TexturePackLoader::postProcessLayer(ui8* pixels, BlockTextureLayer& layer, ui32 width, ui32 height) {
   
    // Helper for checking dimensions
#define DIM_CHECK(w, cw, h, ch, method) \
    if (width != _resolution * cw) { \
        pError("Texture " + layer.path + " is " #method " but width is not " + to_string(cw)); \
        return nullptr; \
    } \
    if (height != _resolution * ch) {  \
        pError("Texture " + layer.path + " is " #method " but height is not " + to_string(ch)); \
        return nullptr; \
    }

    // Pixels must be != nullptr
    if (!pixels) return nullptr;

    // Check that the texture is sized in units of _resolution
    if (width % _resolution) {
        pError("Texture " + layer.path + " width must be a multiple of " + to_string(_resolution));
        return nullptr;
    }
    if (height % _resolution) {
        pError("Texture " + layer.path + " height must be a multiple of " + to_string(_resolution));
        return nullptr;
    }

    // Check for errors and postprocessing based on method
    switch (layer.method) {
        // Need to set up numTiles and totalWeight for RANDOM method
        case ConnectedTextureMethods::CONNECTED:
            DIM_CHECK(width, CONNECTED_WIDTH, height, CONNECTED_HEIGHT, CONNECTED);
            break;
        case ConnectedTextureMethods::RANDOM:
            layer.numTiles = width / height;
            if (layer.weights.length() == 0) {
                layer.totalWeight = layer.numTiles;
            }
            break;
        case ConnectedTextureMethods::GRASS:
            DIM_CHECK(width, GRASS_WIDTH, height, GRASS_HEIGHT, GRASS);
            break;
        case ConnectedTextureMethods::HORIZONTAL:
            DIM_CHECK(width, HORIZONTAL_WIDTH, height, HORIZONTAL_HEIGHT, HORIZONTAL);
            break;
        case ConnectedTextureMethods::VERTICAL:
            DIM_CHECK(width, HORIZONTAL_HEIGHT, height, HORIZONTAL_WIDTH, VERTICAL);
            break;
        case ConnectedTextureMethods::REPEAT:
            DIM_CHECK(width, layer.size.x, height, layer.size.y, REPEAT);
            break;
        case ConnectedTextureMethods::FLORA:
            break;
        case ConnectedTextureMethods::NONE:
            DIM_CHECK(width, 1, height, 1, NONE);
            break;
    }

    // Grab non-const reference to the block texture layer
    BlockTextureLayer* layerToLoad = (BlockTextureLayer*)&(*_blockTextureLayers.insert(layer).first);
    // Mark this layer for load
    _layersToLoad.emplace_back(pixels, layerToLoad);

    // Return pointer
    return layerToLoad;
}

void TexturePackLoader::mapTexturesToAtlases() {

    PreciseTimer timerb;
    timerb.start();
    BlockTextureLayer* layer;
    // Iterate through all the unique texture layers we need to map
    for (auto& it = _blockTextureLayers.begin(); it != _blockTextureLayers.end(); ++it) {
        // Get a non-const pointer to the data
        layer = (BlockTextureLayer*)&(*it);
        // Map the texture layer to an atlas position
        layer->textureIndex = _textureAtlasStitcher.addTexture(*it);
    }

    // Build the pixel data for the texture array
    _textureAtlasStitcher.buildPixelData(_layersToLoad, _resolution);
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