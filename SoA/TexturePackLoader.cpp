#include "stdafx.h"
#include "TexturePackLoader.h"

#include <Vorb/graphics/ImageIO.h>
#include <Vorb/io/Keg.h>

#include "FileSystem.h"
#include "Options.h"

/// yml definition for TexturePackInfo
KEG_TYPE_INIT_BEGIN_DEF_VAR(TexturePackInfo)
KEG_TYPE_INIT_ADD_MEMBER(TexturePackInfo, STRING, name);
KEG_TYPE_INIT_ADD_MEMBER(TexturePackInfo, UI32, resolution);
KEG_TYPE_INIT_ADD_MEMBER(TexturePackInfo, STRING, description);
KEG_TYPE_INIT_END


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
    _packInfo({}),
    _numAtlasPages(0) {
    // Empty
}

TexturePackLoader::~TexturePackLoader() {
    destroy();
}

void TexturePackLoader::loadAllTextures(const nString& texturePackPath) {

    // TODO(Ben): Zip file support
    _texturePackPath = texturePackPath;

    // Load the pack file to get the texture pack description
    _packInfo = loadPackFile(texturePackPath + "pack.yml");

    ui32 width, height;
    for (auto it = _texturesToLoad.begin(); it != _texturesToLoad.end(); ++it) {
        // Load the data
        std::vector<ui8>* pixelStore = new std::vector<ui8>();
        // TODO: Use iom to get a path
        vpath texPath; _ioManager.resolvePath(_texturePackPath + it->first, texPath);
        vio::ImageIO().loadPng(texPath.getString(), *pixelStore, width, height);
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

    // Initialize default values
    texture = BlockTexture();

    auto it = _blockTextureLoadDatas.find(key);
    if (it != _blockTextureLoadDatas.end()) {
        // Initialize default values
        texture = BlockTexture();

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
    atlasTex.width = atlasTex.height = _packInfo.resolution * BLOCK_TEXTURE_ATLAS_WIDTH;

    atlasTex.id = _textureAtlasStitcher.buildTextureArray();

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

void TexturePackLoader::setBlockTextures(BlockPack& blocks) {
    // Initialize all the textures for blocks.
    for (size_t i = 0; i < blocks.size(); i++) {
        blocks[i].InitializeTexture();
    }
    // Since we have set the blocks, we can now free the lookup maps and 
    // blockTextureLayers cache
    std::map <nString, BlockTextureData>().swap(_blockTextureLoadDatas);
    std::set <BlockTextureLayer>().swap(_blockTextureLayers);
}

/// Loads the pack file for this texture pack
/// @param filePath: The path of the pack file
/// @return The texture pack info
TexturePackInfo TexturePackLoader::loadPackFile(const nString& filePath) {
    TexturePackInfo rv = {};
    nString data;
    _ioManager.readFileToString(filePath.c_str(), data);
    if (data.length()) {
        if (Keg::parse(&rv, data.c_str(), "TexturePackInfo") == Keg::Error::NONE) {
            return rv;
        }
    }
    pError("Failed to load texture pack file " + filePath);
    return rv;
}

ColorRGB8* TexturePackLoader::getColorMap(const nString& name) {
    auto it = _blockColorMapLookupTable.find(name);
    if (it != _blockColorMapLookupTable.end()) {
        return _blockColorMaps[it->second];
    } else {
        _blockColorMapLookupTable[name] = _blockColorMaps.size();
        _blockColorMaps.push_back(new ColorRGB8[256 * 256]);
        return _blockColorMaps.back();
    }
}

ui32 TexturePackLoader::getColorMapIndex(const nString& name) {

#define MAP_WIDTH 256

    auto it = _blockColorMapLookupTable.find(name);
    if (it != _blockColorMapLookupTable.end()) {
        return it->second;
    } else {
        _blockColorMapLookupTable[name] = _blockColorMaps.size();
        _blockColorMaps.push_back(new ColorRGB8[256 * 256]);
        ColorRGB8* colorMap = _blockColorMaps.back();

        // Load the color map into a buffer
        std::vector<ui8> buffer;
        ui32 width, height;
        
        // TODO: Same as above
        vpath texPath; _ioManager.resolvePath(name, texPath);
        vio::ImageIO().loadPng(texPath.getString(), buffer, width, height);

        // TODO: Implement
        //// Error checking
        //if (width != MAP_WIDTH) {
        //    pError("Error color map " + name + " must have a width of " + to_string(MAP_WIDTH));
        //    return _blockColorMaps.size() - 1;
        //}
        //if (height != MAP_WIDTH) {
        //    pError("Error color map " + name + " must have a height of " + to_string(MAP_WIDTH));
        //    return _blockColorMaps.size() - 1;
        //}

        //// Set the data
        //for (int y = 0; y < MAP_WIDTH; y++){
        //    for (int x = 0; x < MAP_WIDTH; x++) {
        //        colorMap[(MAP_WIDTH - y - 1) * MAP_WIDTH + x] = ColorRGB8(buffer[(y * MAP_WIDTH + x) * 3],
        //                                                                  buffer[(y * MAP_WIDTH + x) * 3 + 1],
        //                                                                  buffer[(y * MAP_WIDTH + x) * 3 + 2]);
        //    }
        //}
        return _blockColorMaps.size() - 1;
    }
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

    for (auto& color : _blockColorMaps) {
        delete[] color;
    }

    std::map <nString, ui32>().swap(_blockColorMapLookupTable); 
    std::vector <ColorRGB8*>().swap(_blockColorMaps);

    // Make sure to free all pixel data
    for (auto& pixels : _pixelCache) {
        delete pixels.second.data;
    }
    std::map <nString, Pixels>().swap(_pixelCache);
    _numAtlasPages = 0;
}

void TexturePackLoader::writeDebugAtlases() {
    int width = _packInfo.resolution * BLOCK_TEXTURE_ATLAS_WIDTH;
    int height = width;

    int pixelsPerPage = width * height * 4;
    ui8 *pixels = new ui8[width * height * 4 * _numAtlasPages];

    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.id);
    glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    for (int i = 0; i < _numAtlasPages; i++) {

        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(pixels + i * pixelsPerPage, width, height, _packInfo.resolution, 4 * width, 0xFF, 0xFF00, 0xFF0000, 0x0);
        SDL_SaveBMP(surface, ("atlas" + std::to_string(i) + ".bmp").c_str());

    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    delete[] pixels;
}

void TexturePackLoader::loadAllBlockTextures() {
    // Used to get the texture pixel dimensions
    ui32 width, height;

    ui8* pixels;

    // Free _blockTextureLayers in case a pack has been loaded before
    std::set <BlockTextureLayer>().swap(_blockTextureLayers);

    // Loop through all textures to load
    for (const nString& texturePath : _blockTexturesToLoad) {

        // The class we need to populate
        BlockTextureData blockTextureLoadData = {};
        BlockTexture blockTexture;

        // Search for the tex file first
        nString texFileName = _texturePackPath + texturePath;
        // Convert .png to .tex
        texFileName.replace(texFileName.end() - 4, texFileName.end(), ".tex");

        // Load the tex file (if it exists)
        loadTexFile(texFileName, nullptr, &blockTexture);

        // If there wasn't an explicit override base path, we use the texturePath
        if (blockTexture.base.path.empty()) blockTexture.base.path = texturePath;

        // Get pixels for the base texture
        pixels = getPixels(blockTexture.base.path, width, height);
        // Store handle to the layer, do postprocessing, add layer to load
        blockTextureLoadData.base = postProcessLayer(pixels, blockTexture.base, width, height);
        // Init the func
        if (blockTextureLoadData.base) blockTextureLoadData.base->initBlockTextureFunc();

        // Check if we have an overlay
        if (blockTexture.overlay.path.empty() == false) {
            // Get pixels for the overlay texture
            pixels = getPixels(blockTexture.overlay.path, width, height);
            // Store handle to the layer, do postprocessing, add layer to load
            blockTextureLoadData.overlay = postProcessLayer(pixels, blockTexture.overlay, width, height);
            // Init the func
            if (blockTextureLoadData.overlay) blockTextureLoadData.overlay->initBlockTextureFunc();
        }

        // Set blend mode
        blockTextureLoadData.blendMode = blockTexture.blendMode;

        // Add it to the list of load datas
        _blockTextureLoadDatas[texturePath] = blockTextureLoadData;
    }

    // Finish by mapping all the blockTextureLoadDatas to atlases
    mapTexturesToAtlases();
}

bool TexturePackLoader::loadTexFile(nString fileName, ZipFile *zipFile, BlockTexture* rv) {
    
    nString data;
    _ioManager.readFileToString(fileName.c_str(), data);
    if (data.length()) {
        if (Keg::parse(rv, data.c_str(), "BlockTexture") == Keg::Error::NONE) {
            if (rv->base.weights.getLength() > 0) {
                rv->base.totalWeight = 0;
                for (i32 i = 0; i < rv->base.weights.getLength(); i++) {
                    rv->base.totalWeight += rv->base.weights[i];
                }
            }
            if (rv->overlay.weights.getLength() > 0) {
                rv->overlay.totalWeight = 0;
                for (i32 i = 0; i < rv->overlay.weights.getLength(); i++) {
                    rv->overlay.totalWeight += rv->overlay.weights[i];
                }
            }

            // Get ColorMap indices
            if (rv->base.useMapColor.length()) {
                rv->base.colorMapIndex = getColorMapIndex(rv->base.useMapColor);
            }
            if (rv->overlay.useMapColor.length()) {
                rv->overlay.colorMapIndex = getColorMapIndex(rv->overlay.useMapColor);
            }
            return true;
        }
    }
    return false;
}

BlockTextureLayer* TexturePackLoader::postProcessLayer(ui8* pixels, BlockTextureLayer& layer, ui32 width, ui32 height) {
   
    ui32 floraRows;

    // Helper for checking dimensions
#define DIM_CHECK(w, cw, h, ch, method) \
    if (width != _packInfo.resolution * cw) { \
        pError("Texture " + layer.path + " is " #method " but width is not " + std::to_string(cw)); \
        return nullptr; \
    } \
    if (height != _packInfo.resolution * ch) {  \
        pError("Texture " + layer.path + " is " #method " but height is not " + std::to_string(ch)); \
        return nullptr; \
    }

    // Pixels must be != nullptr
    if (!pixels) return nullptr;

    // Check that the texture is sized in units of _packInfo.resolution
    if (width % _packInfo.resolution) {
        pError("Texture " + layer.path + " width must be a multiple of " + std::to_string(_packInfo.resolution));
        return nullptr;
    }
    if (height % _packInfo.resolution) {
        pError("Texture " + layer.path + " height must be a multiple of " + std::to_string(_packInfo.resolution));
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
            if (layer.weights.getLength() == 0) {
                layer.totalWeight = layer.numTiles;
            } else { // Need to check if there is the right number of weights
                if (layer.weights.getLength() * _packInfo.resolution != width) {
                    pError("Texture " + layer.path + " weights length must match number of columns or be empty. weights.length() = " + 
                        std::to_string(layer.weights.getLength()) + " but there are " + std::to_string(width / _packInfo.resolution) + " columns.");
                    return nullptr;
                }
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
            floraRows = BlockTextureLayer::getFloraRows(layer.floraHeight);
            if (height != _packInfo.resolution * floraRows) {
                pError("Texture " + layer.path + " texture height must be equal to (maxFloraHeight^2 + maxFloraHeight) / 2 * resolution = " +
                    std::to_string(height) + " but it is " + std::to_string(_packInfo.resolution * floraRows));
                return nullptr;
            }
            // If no weights, they are all equal
            if (layer.weights.getLength() == 0) {
                layer.totalWeight = width / _packInfo.resolution;
            } else { // Need to check if there is the right number of weights
                if (layer.weights.getLength() * _packInfo.resolution != width) {
                    pError("Texture " + layer.path + " weights length must match number of columns or be empty. weights.length() = " +
                        std::to_string(layer.weights.getLength()) + " but there are " + std::to_string(width / _packInfo.resolution) + " columns.");
                    return nullptr;
                }
            }
            // Tile dimensions and count
            layer.size.x = width / _packInfo.resolution;
            layer.size.y = floraRows;
            layer.numTiles = layer.size.x * layer.size.y;
            break;
        case ConnectedTextureMethods::NONE:
            DIM_CHECK(width, 1, height, 1, NONE);
            break;
        default:
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
    _textureAtlasStitcher.buildPixelData(_layersToLoad, _packInfo.resolution);
}

ui8* TexturePackLoader::getPixels(const nString& filePath, ui32& width, ui32& height) {

    if (filePath.empty()) return nullptr;

    // Check the cache
    auto& it = _pixelCache.find(filePath);

    if (it == _pixelCache.end()) {
        // Load the data
        std::vector<ui8>* pixelStore = new std::vector<ui8>();

        vpath texPath; _ioManager.resolvePath(_texturePackPath + filePath, texPath);
        vio::ImageIO().loadPng(texPath.getString(), *pixelStore, width, height);
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