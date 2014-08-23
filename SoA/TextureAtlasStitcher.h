#pragma once

// Number Of Tiles Per Side Of The Block Texture Atlas
const i32 BLOCK_TEXTURE_ATLAS_BITS_PER_INDEX = 4;
const i32 BLOCK_TEXTURE_ATLAS_BITMASK_X = (1 << BLOCK_TEXTURE_ATLAS_BITS_PER_INDEX) - 1;
const i32 BLOCK_TEXTURE_ATLAS_BITMASK_Y = BLOCK_TEXTURE_ATLAS_BITMASK_X << BLOCK_TEXTURE_ATLAS_BITS_PER_INDEX;
const i32 BLOCK_TEXTURE_ATLAS_TILES_PER_SIDE = 1 << BLOCK_TEXTURE_ATLAS_BITS_PER_INDEX;
const i32 BLOCK_TEXTURE_ATLAS_TILES_PER_PAGE = BLOCK_TEXTURE_ATLAS_TILES_PER_SIDE * BLOCK_TEXTURE_ATLAS_TILES_PER_SIDE;

struct BlockTextureIndex {
public:
    BlockTextureIndex(i32 tileWidth, i32 tileHeight);
    BlockTextureIndex() : BlockTextureIndex(0, 0) {}

    void setSize(const i32& x, const i32& y) {
        atlasUVRect[2] = (ui8)((y << BLOCK_TEXTURE_ATLAS_BITS_PER_INDEX) | x);
    }

    void setIndex(const i32& x, const i32& y) {
        atlasUVRect[0] = (ui8)((y << BLOCK_TEXTURE_ATLAS_BITS_PER_INDEX) | x);
    }
    void setPage(const i32& page) {
        atlasUVRect[1] = (ui8)page;
    }

    void getSize(i32* x, i32* y) const {
        *y = atlasUVRect[2] >> BLOCK_TEXTURE_ATLAS_BITS_PER_INDEX;
        *x = atlasUVRect[2] & BLOCK_TEXTURE_ATLAS_BITMASK_X;
    }

    // The Location And Size In The Atlas
    ui8 atlasUVRect[3];

    // Texturing Method
    ui8 textureMethod;
};

// Stores Information About An Atlas Page For Construction Purposes
struct BlockAtlasPage;

class TextureAtlasStitcher {
public:
    TextureAtlasStitcher();
    ~TextureAtlasStitcher();

    void addTextures(std::vector<BlockTextureIndex>& textures);
private:
    bool tryAdd(const i32v2& texSize, BlockAtlasPage* page, i32v2& index);
    void addAsFirst(const i32v2& texSize, BlockAtlasPage* page, i32v2& index);

    std::vector<BlockAtlasPage*> _pages;
};