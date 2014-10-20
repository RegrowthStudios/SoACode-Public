#include "stdafx.h"
#include "TextureAtlasStitcher.h"

#include "Errors.h"

#define CONNECTED_TILES 47
#define GRASS_TILES 9
#define HORIZONTAL_TILES 4
#define VERTICAL_TILES 4

struct BlockAtlasPage {
    bool slots[BLOCK_TEXTURE_ATLAS_SIZE];
};


TextureAtlasStitcher::TextureAtlasStitcher() :
    _oldestFreeSlot(0) {

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
    return index;
}

void TextureAtlasStitcher::destroy() {

    for (int i = 0; i < _pages.size(); i++) {
        delete _pages[i];
    }
    std::vector<BlockAtlasPage*>().swap(_pages);

    _oldestFreeSlot = 0;
}

i32 TextureAtlasStitcher::mapSingle() {
 
    int i;
    int pageIndex;

    // Find the next free slot
    do {
        i = _oldestFreeSlot % 256;
        pageIndex = _oldestFreeSlot / 256;

        // If we need to allocate a new page
        if (pageIndex >= _pages.size()) {
            _pages.push_back(new BlockAtlasPage({}));
        }

        // Since we are mapping single textures, we know this is the oldest free
        _oldestFreeSlot++;
    } while (_pages[pageIndex]->slots[i] == false);

    //mark this slot as not free
    _pages[pageIndex]->slots[i] = false;

    return pageIndex * 256 + i;
}

i32 TextureAtlasStitcher::mapBox(int width, int height) {

    if (width > 16 || height > 16) {
        pError("Repeat texture width or height > 16. Must be <= 16!");
        return 0;
    }

    int i;
    int atlasIndex;
    int x, y;

    // Start the search at the oldest known free spot.
    int searchIndex = _oldestFreeSlot;
    bool fits;
    // Find the next free slot that is large enough
    while (true) {
        i = searchIndex % 256;
        atlasIndex = searchIndex / 256;
        x = i % 16;
        y = i / 16;
        fits = true;

        // If we need to alocate a new atlas
        if (atlasIndex >= _pages.size()) {
            _pages.push_back(new BlockAtlasPage({}));
        }

        //if it doesn't fit in Y direction, go to next page
        if (y + height > 16) {
            searchIndex += 256 - i;
            continue;
        }
        //if it doesnt fit in X direction, go to next row
        if (x + width > 16) {
            searchIndex += 16 - x;
            continue;
        }

        searchIndex++;

        //Search to see if all needed slots are free
        for (int j = y; j < y + height; j++) {
            for (int k = x; k < x + width; k++) {
                if (_pages[atlasIndex]->slots[j * 16 + k] == false) {
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
            _pages[atlasIndex]->slots[j * 16 + k] = false;
        }
    }

    return i + atlasIndex * 256;
}

i32 TextureAtlasStitcher::mapContiguous(int numTiles) {

    int i;
    int atlasIndex;
    int numContiguous = 0;
    //Start the search at the oldest known free spot.
    int searchIndex = _oldestFreeSlot;
    bool passedFreeSlot = false;

    //Find the next free slot that is large enough
    while (true) {
        i = searchIndex % 256;
        atlasIndex = searchIndex / 256;

        //If we need to alocate a new atlas
        if (atlasIndex >= _pages.size()) {
            _pages.push_back(new BlockAtlasPage({}));
        }

        searchIndex++;
        if (_pages[atlasIndex]->slots[i] == false) {
            if (numContiguous) {
                passedFreeSlot = true;
            }
            numContiguous = 0;
        } else {
            numContiguous++;
        }

        //Stop searching if we have found a contiguous block that is large enough
        if (numContiguous == numTiles) {
            i = searchIndex % 256;
            atlasIndex = searchIndex / 256;
            break;
        }
    }

    //Move the oldest known free slot forward if we havent passed a free spot
    if (passedFreeSlot == false) {
        _oldestFreeSlot = i + atlasIndex * 256;
    }

    return i + atlasIndex * 256 - numTiles;
}
