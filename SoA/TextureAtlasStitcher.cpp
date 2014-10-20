#include "stdafx.h"
#include "TextureAtlasStitcher.h"

#include "Errors.h"

#define CONNECTED_TILES 47
#define GRASS_TILES 9
#define HORIZONTAL_TILES 4
#define VERTICAL_TILES 4

#define BYTES_PER_PIXEL 4

struct BlockAtlasPage {
    bool slots[BLOCK_TEXTURE_ATLAS_SIZE];
};


TextureAtlasStitcher::TextureAtlasStitcher() :
    _bytesPerPage(0),
    _oldestFreeSlot(0),
    _resolution(0),
    _pixelData(nullptr) {

}

TextureAtlasStitcher::~TextureAtlasStitcher() {
    i32 pageCount = _pages.size();
    if (pageCount > 0) {
        for (i32 i = 0; i < pageCount; i++) {
            delete _pages[i];
        }
    }
}

i32 TextureAtlasStitcher::addTexture(const BlockTextureLayer& layer) {

    i32 index;
    switch (layer.method) {
        // Map it to the atlas based on its method
        case ConnectedTextureMethods::CONNECTED:
            index = mapContiguous(CONNECTED_TILES);
            break;
        case ConnectedTextureMethods::RANDOM:
            index = mapContiguous(layer.numTiles);
            break;
        case ConnectedTextureMethods::REPEAT:
            index = mapBox(layer.size.x, layer.size.y);
            break;
        case ConnectedTextureMethods::GRASS:
            index = mapContiguous(GRASS_TILES);
            break;
        case ConnectedTextureMethods::HORIZONTAL:
            index = mapContiguous(HORIZONTAL_TILES);
            break;
        case ConnectedTextureMethods::VERTICAL:
            index = mapContiguous(VERTICAL_TILES);
            break;
        default:
            index = mapSingle();
            break;
    }

    // Store the layer and the index
    _textureLayerCache[layer] = index;
    return index;
}

/// Allocates the pixels for the array and fills with the proper data
void TextureAtlasStitcher::buildPixelData(const std::vector <BlockLayerLoadData>& layers, int resolution) {

    // Set the resolution of the blocks
    _resolution = resolution;

    // Store some helpful values for when we do writing
    _bytesPerPixelRow = resolution * BLOCK_TEXTURE_ATLAS_WIDTH * BYTES_PER_PIXEL; 
    _bytesPerTileRow = resolution * _bytesPerPixelRow;
    _bytesPerPage = _bytesPerTileRow * BLOCK_TEXTURE_ATLAS_WIDTH;

    // Allocate pixel data for entire texture array
    _pixelData = new ui8[_bytesPerPage * _pages.size()];

    // Loop through all layers to load
    for (auto& loadData : layers) {
        // Get the layer handle
        BlockTextureLayer* layer = loadData.layer;
        ui8* pixels = loadData.pixels;
        // Write pixels to the _pixelData array based on method
        switch (layer->method) {
            case ConnectedTextureMethods::CONNECTED:
                writeToAtlasContiguous(layer->textureIndex, pixels, 12, 4, 47);
                break;
            case ConnectedTextureMethods::RANDOM:
                writeToAtlasContiguous(layer->textureIndex, pixels, layer->numTiles, 1, layer->numTiles);
                break;
            case ConnectedTextureMethods::REPEAT:
                writeToAtlas(layer->textureIndex, pixels, _resolution * layer->size.x, _resolution * layer->size.y);
                break;
            case ConnectedTextureMethods::GRASS:
                writeToAtlasContiguous(layer->textureIndex, pixels, 3, 3, 9);
                break;
            case ConnectedTextureMethods::HORIZONTAL:
                writeToAtlasContiguous(layer->textureIndex, pixels, 4, 1, 4);
                break;
            case ConnectedTextureMethods::VERTICAL:
                writeToAtlasContiguous(layer->textureIndex, pixels, 1, 4, 4);
                break;
            default:
                writeToAtlas(layer->textureIndex, pixels, _resolution, _resolution);
                break;
        }
    }
}

ui32 TextureAtlasStitcher::buildTextureArray() {

    ui32 textureID;
    int imageWidth = _resolution * BLOCK_TEXTURE_ATLAS_WIDTH;

    // Set up the storage
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

    // Calculate max mipmap level
    int maxMipLevel = 0;
    int width = imageWidth;
    while (width > BLOCK_TEXTURE_ATLAS_WIDTH) {
        width >>= 1;
        maxMipLevel++;
    }

    // Set up all the mimpap storage
    width = imageWidth;
    for (i32 i = 0; i < maxMipLevel; i++) {
        glTexImage3D(GL_TEXTURE_2D_ARRAY, i, GL_RGBA8, width, width, _pages.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        width >>= 1;
        if (width < 1) width = 1;
    }

    // Upload the data to VRAM
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, imageWidth, imageWidth, _pages.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixelData);

    // Free up the RAM
    delete[] _pixelData;
    _pixelData = nullptr;

    // Set up tex parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, maxMipLevel);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD, maxMipLevel);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    
    // Check if we had any errors
    checkGlError("TextureAtlasStitcher::buildAtlasArray()");
}

void TextureAtlasStitcher::destroy() {

    std::map <BlockTextureLayer, ui32>().swap(_textureLayerCache);

    for (int i = 0; i < _pages.size(); i++) {
        delete _pages[i];
    }
    std::vector<BlockAtlasPage*>().swap(_pages);

    _bytesPerPage = 0;
    _oldestFreeSlot = 0;
    _resolution = 0;
    
    if (_pixelData) {
        delete[] _pixelData;
        _pixelData = nullptr;
    }

}

i32 TextureAtlasStitcher::mapSingle() {
 
    int i;
    int pageIndex;

    // Find the next free slot
    do {
        i = _oldestFreeSlot % BLOCK_TEXTURE_ATLAS_SIZE;
        pageIndex = _oldestFreeSlot / BLOCK_TEXTURE_ATLAS_SIZE;

        // If we need to allocate a new page
        if (pageIndex >= _pages.size()) {
            _pages.push_back(new BlockAtlasPage({}));
        }

        // Since we are mapping single textures, we know this is the oldest free
        _oldestFreeSlot++;
    } while (_pages[pageIndex]->slots[i] == false);

    //mark this slot as not free
    _pages[pageIndex]->slots[i] = false;

    return pageIndex * BLOCK_TEXTURE_ATLAS_SIZE + i;
}

i32 TextureAtlasStitcher::mapBox(int width, int height) {

    if (width > BLOCK_TEXTURE_ATLAS_WIDTH || height > BLOCK_TEXTURE_ATLAS_WIDTH) {
        pError("Repeat texture width or height > " + to_string(BLOCK_TEXTURE_ATLAS_WIDTH));
        return 0;
    }

    int i;
    int pageIndex;
    int x, y;

    // Start the search at the oldest known free spot.
    int searchIndex = _oldestFreeSlot;
    bool fits;
    // Find the next free slot that is large enough
    while (true) {
        i = searchIndex % BLOCK_TEXTURE_ATLAS_SIZE;
        pageIndex = searchIndex / BLOCK_TEXTURE_ATLAS_SIZE;
        x = i % BLOCK_TEXTURE_ATLAS_WIDTH;
        y = i / BLOCK_TEXTURE_ATLAS_WIDTH;
        fits = true;

        // If we need to alocate a new atlas
        if (pageIndex >= _pages.size()) {
            _pages.push_back(new BlockAtlasPage({}));
        }

        //if it doesn't fit in Y direction, go to next page
        if (y + height > BLOCK_TEXTURE_ATLAS_WIDTH) {
            searchIndex += BLOCK_TEXTURE_ATLAS_SIZE - i;
            continue;
        }
        //if it doesnt fit in X direction, go to next row
        if (x + width > BLOCK_TEXTURE_ATLAS_WIDTH) {
            searchIndex += BLOCK_TEXTURE_ATLAS_WIDTH - x;
            continue;
        }

        searchIndex++;

        //Search to see if all needed slots are free
        for (int j = y; j < y + height; j++) {
            for (int k = x; k < x + width; k++) {
                if (_pages[pageIndex]->slots[j * BLOCK_TEXTURE_ATLAS_WIDTH + k] == false) {
                    fits = false;
                    j = 100; //force to fall out of loop
                    break;
                }
            }
        }

        if (fits) {
            //if we reach here, it will fit at this position
            break;
        }
    }

    //Set all free slots to false
    for (int j = y; j < y + height; j++) {
        for (int k = x; k < x + width; k++) {
            _pages[pageIndex]->slots[j * BLOCK_TEXTURE_ATLAS_WIDTH + k] = false;
        }
    }

    return i + pageIndex * BLOCK_TEXTURE_ATLAS_SIZE;
}

i32 TextureAtlasStitcher::mapContiguous(int numTiles) {

    int i;
    int pageIndex;
    int numContiguous = 0;
    //Start the search at the oldest known free spot.
    int searchIndex = _oldestFreeSlot;
    bool passedFreeSlot = false;

    //Find the next free slot that is large enough
    while (true) {
        i = searchIndex % BLOCK_TEXTURE_ATLAS_SIZE;
        pageIndex = searchIndex / BLOCK_TEXTURE_ATLAS_SIZE;

        //If we need to alocate a new atlas
        if (pageIndex >= _pages.size()) {
            _pages.push_back(new BlockAtlasPage({}));
        }

        searchIndex++;
        if (_pages[pageIndex]->slots[i] == false) {
            if (numContiguous) {
                passedFreeSlot = true;
            }
            numContiguous = 0;
        } else {
            numContiguous++;
        }

        //Stop searching if we have found a contiguous block that is large enough
        if (numContiguous == numTiles) {
            i = searchIndex % BLOCK_TEXTURE_ATLAS_SIZE;
            pageIndex = searchIndex / BLOCK_TEXTURE_ATLAS_SIZE;
            break;
        }
    }

    //Move the oldest known free slot forward if we havent passed a free spot
    if (passedFreeSlot == false) {
        _oldestFreeSlot = i + pageIndex * BLOCK_TEXTURE_ATLAS_SIZE;
    }

    return i + pageIndex * BLOCK_TEXTURE_ATLAS_SIZE - numTiles;
}



void TextureAtlasStitcher::writeToAtlas(int texIndex, ui8* pixels, int pixelWidth, int pixelHeight) {
    
    // Get the location in the array
    int i = texIndex % BLOCK_TEXTURE_ATLAS_SIZE;
    int dx = i % BLOCK_TEXTURE_ATLAS_WIDTH;
    int dy = i % BLOCK_TEXTURE_ATLAS_WIDTH;
    int pageIndex = texIndex / BLOCK_TEXTURE_ATLAS_SIZE;

    // Start of destination
    ui8* dest = _pixelData + pageIndex * _bytesPerPage + dx * BYTES_PER_PIXEL + dy * _bytesPerTileRow;

    int offset;
    // Copy the block of pixels
    for (int y = 0; y < pixelHeight; y++) {
        offset = y * _bytesPerPixelRow;
        memcpy(dest + offset, pixels + offset, pixelWidth);
    }
}

void TextureAtlasStitcher::writeToAtlasContiguous(int texIndex, ui8* pixels, int width, int height, int numTiles) {

    int bytesPerTileWidth = width * _resolution * BYTES_PER_PIXEL;

    int n = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width && n < numTiles; x++, n++) {
            // Get pointer to source data
            ui8* src = pixels + y * _bytesPerTileRow + x * bytesPerTileWidth;

            writeToAtlas(texIndex++, src, _resolution, _resolution);
        }
    }

}
