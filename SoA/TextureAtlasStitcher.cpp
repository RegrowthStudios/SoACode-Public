#include "stdafx.h"
#include "TextureAtlasStitcher.h"

BlockTextureIndex::BlockTextureIndex(i32 tileWidth, i32 tileHeight) {
    setSize(tileWidth, tileHeight);
    setIndex(0, 0);
    setPage(0);
}

struct BlockAtlasPage {
public:
    BlockAtlasPage(i32 index) :
        index(0),
        freeSlots(BLOCK_TEXTURE_ATLAS_TILES_PER_PAGE),
        firstFreeSlot(0) {
        memset(isInUse, 0, sizeof(isInUse));
    }

    // Location In The Texture Atlas
    i32 index;

    i32 freeSlots;
    i32 firstFreeSlot;

    bool isInUse[BLOCK_TEXTURE_ATLAS_TILES_PER_PAGE];
};


TextureAtlasStitcher::TextureAtlasStitcher() {

}
TextureAtlasStitcher::~TextureAtlasStitcher() {
    i32 pageCount = _pages.size();
    if (pageCount > 0) {
        for (i32 i = 0; i < pageCount; i++) {
            delete _pages[i];
        }
        _pages.swap(std::vector<BlockAtlasPage*>());
    }
}

bool predBlockTile(const BlockTextureIndex& b1, const BlockTextureIndex& b2) {
    i32 x1, y1, x2, y2;
    b1.getSize(&x1, &y1);
    b2.getSize(&x2, &y2);
    return (y1 < y2 && x1 < x2) || (x1 * y1 < x2 * y2);
}

void TextureAtlasStitcher::addTextures(std::vector<BlockTextureIndex>& textures) {
    // First Sort Based On Size So That The Largest Are Placed In First
    std::sort(textures.begin(), textures.end(), predBlockTile);

    // Add All Of The Textures
    i32 texCount = textures.size();
    for (i32 i = 0; i < texCount; i++) {
        // Find The Size Of The Texture
        i32v2 texSize, texIndex;
        textures[i].getSize(&texSize.x, &texSize.y);

        // Find A Suitable Page
        for (BlockAtlasPage* page : _pages) {
            if (tryAdd(texSize, page, texIndex)) {
                textures[i].setIndex(texIndex.x, texIndex.y);
                textures[i].setPage(page->index);
                return;
            }
        }

        // Create A New Atlas Page
        BlockAtlasPage* page = new BlockAtlasPage(_pages.size());
        _pages.push_back(page);
        addAsFirst(texSize, page, texIndex);
        textures[i].setIndex(texIndex.x, texIndex.y);
        textures[i].setPage(page->index);
    }
}

bool TextureAtlasStitcher::tryAdd(const i32v2& texSize, BlockAtlasPage* page, i32v2& index) {
    // Make Sure Enough Slots Are Available
    i32 requiredSlots = texSize.x * texSize.y;
    if (page->freeSlots < requiredSlots) return false;

    if (requiredSlots == 1) {
        index.x = page->firstFreeSlot & BLOCK_TEXTURE_ATLAS_BITMASK_X;
        index.y = page->firstFreeSlot >> BLOCK_TEXTURE_ATLAS_BITS_PER_INDEX;


    }
}
void TextureAtlasStitcher::addAsFirst(const i32v2& texSize, BlockAtlasPage* page, i32v2& index) {

}
