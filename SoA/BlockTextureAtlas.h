#pragma once

// Number Of Tiles Per Side Of The Block Texture Atlas
const i32 BLOCK_TEXTURE_ATLAS_TILES_PER_SIDE = 16;
const i32 BLOCK_TEXTURE_ATLAS_TILES_PER_PAGE = BLOCK_TEXTURE_ATLAS_TILES_PER_SIDE * BLOCK_TEXTURE_ATLAS_TILES_PER_SIDE;

struct BlockTextureIndex {
public:
    BlockTextureIndex(ui16 blockID, i32 tileWidth, i32 tileHeight);
    BlockTextureIndex() : BlockTextureIndex(0, 0, 0) {}

    void setIndex(const i32& x, const i32& y) {
        atlasUVRect[0] = (ui8)((x << 4) | y);
    }
    void setPage(const i32& page) {
        atlasUVRect[1] = (ui8)page;
    }
    void setSize(const i32& x, const i32& y) {
        atlasUVRect[2] = (ui8)((x << 4) | y);
    }

    // The Location And Size In The Atlas
    ui8 atlasUVRect[3];

    // Texturing Method
    ui8 textureMethod;
};

struct BlockAtlasPage;

class BlockTextureAtlas {
public:
    BlockTextureAtlas(i32 tileResolution);

    void addTextures(std::vector<BlockTextureIndex>& textures);
private:
    i32v3 _atlasDimensions;
    i32 _resolution;

    std::vector<BlockAtlasPage*> _atlasPages;
    std::vector<BlockTextureIndex*> _sortedOrderTiles;
};