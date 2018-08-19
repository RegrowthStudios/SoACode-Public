///
/// BlockTexturePack.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 16 Jun 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// A texture pack for blocks
///

#pragma once

#ifndef BlockTexturePack_h__
#define BlockTexturePack_h__

#include "BlockTexture.h"

#include <map>
#include <Vorb/graphics/gtypes.h>
#include <Vorb/voxel/VoxelTextureStitcher.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/ecs/Entity.h>

struct SphericalVoxelComponent;

DECL_VG(class BitmapResource; class Texture);

struct AtlasTextureDescription {
    BlockTextureLayer temp;
    BlockTextureIndex index;
    ui32v2 size;
};

#define BLOCK_COLOR_MAP_WIDTH 256

struct BlockColorMap {
    color3 pixels[BLOCK_COLOR_MAP_WIDTH][BLOCK_COLOR_MAP_WIDTH];
};

class BlockTexturePack {
public:
    BlockTexturePack():
    m_atlasTexture(0),
    m_resolution(0),
    m_pageWidthPixels(0),
    m_mipLevels(0),
    m_needsRealloc(false),
    m_textures(nullptr),
    m_nextFree(0),
    m_maxTextures(0)
    {}

    ~BlockTexturePack();

    void init(ui32 resolution, ui32 maxTextures);
    // Maps the texture layer to the atlas and updates the index in layer.
    // Does not check if texture already exists
    BlockTextureIndex addLayer(const BlockTextureLayer& layer, const nString& path, color4* pixels);
    // Tries to find the texture index. Returns empty description on fail.
    AtlasTextureDescription findLayer(const nString& filePath);

    BlockTexture* findTexture(const nString& filePath);
    // Returns a pointer to the next free block texture and increments internal counter.
    // Will crash if called more than m_maxTextures times.
    BlockTexture* getNextFreeTexture(const nString& filePath);

    BlockTexture* getDefaultTexture() { return &m_defaultTexture; }

    // Gets existing color map or loads from file
    BlockColorMap* getColorMap(const nString& path);

    BlockColorMap* setColorMap(const nString& name, const vg::BitmapResource* rs);
    BlockColorMap* setColorMap(const nString& name, const ui8v3* pixels);

    // Call on GL thread. Will upload any textures that aren't yet uploaded.
    void update();

    void writeDebugAtlases();

    void dispose();

    const VGTexture& getAtlasTexture() const { return m_atlasTexture; }
    const ui32& getResolution() const { return m_resolution; }
private:
    VORB_NON_COPYABLE(BlockTexturePack);

    void flagDirtyPage(ui32 pageIndex);

    void allocatePages();

    void uploadPage(ui32 pageIndex);

    void writeToAtlas(BlockTextureIndex texIndex, color4* pixels, ui32 pixelWidth, ui32 pixelHeight, ui32 tileWidth);

    void writeToAtlasContiguous(BlockTextureIndex texIndex, color4* pixels, ui32 width, ui32 height, ui32 numTiles);

    /************************************************************************/
    /* Event Handlers                                                       */
    /************************************************************************/
    void onAddSphericalVoxelComponent(Sender s, SphericalVoxelComponent& cmp, vecs::EntityID e);

    struct AtlasPage {
        AtlasPage():pixels(nullptr), dirty(true){}
        color4* pixels;
        bool dirty;
    };

    vvox::VoxelTextureStitcher m_stitcher;

    VGTexture m_atlasTexture;
    std::vector<AtlasPage> m_pages; ///< Cached pixel data
    std::vector<int> m_dirtyPages; ///< List of dirty pages TODO(Ben): Maybe bad for multithreading
    std::unordered_map<nString, AtlasTextureDescription> m_descLookup;
    ui32 m_resolution;
    ui32 m_pageWidthPixels;
    ui32 m_mipLevels;
    bool m_needsRealloc;

    // For cache friendly caching of textures
    std::map<nString, ui32> m_textureLookup;
    std::map<nString, BlockColorMap> m_colorMaps;
    BlockTexture* m_textures; ///< Storage of all block textures
    BlockTexture m_defaultTexture;
    ui32 m_nextFree;
    ui32 m_maxTextures; ///< Maximum possible number of unique textures with this mod config
};

#endif // BlockTexturePack_h__