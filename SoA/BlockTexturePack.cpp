#include "stdafx.h"
#include "BlockTexturePack.h"

#include <SDL/SDL.h>

#include "Errors.h"

#define CONNECTED_TILES 47
#define GRASS_TILES 9
#define HORIZONTAL_TILES 4
#define VERTICAL_TILES 4

BlockTexturePack::~BlockTexturePack() {
    dispose();
}

void BlockTexturePack::init(ui32 resolution, ui32 maxTextures) {
    m_resolution = resolution;
    m_pageWidthPixels = m_resolution * m_stitcher.getTilesPerRow();
    m_maxTextures = maxTextures;
    m_textures = new BlockMaterial[maxTextures];
    m_nextFree = 0;
    // Calculate max mipmap level
    m_mipLevels = 0;
    int width = m_pageWidthPixels;
    while (width > m_stitcher.getTilesPerRow()) {
        width >>= 1;
        m_mipLevels++;
    }

    // Set up first page for default textures
    flagDirtyPage(0);
}

// TODO(Ben): Lock?
void BlockTexturePack::addLayer(BlockMaterialLayer& layer, color4* pixels) {
    // Map the texture
    int firstPageIndex;
    int lastPageIndex;
    switch (layer.method) {
        case ConnectedTextureMethods::CONNECTED:
            layer.index = m_stitcher.mapContiguous(CONNECTED_TILES);
            lastPageIndex = (layer.index + CONNECTED_TILES) / m_stitcher.getTilesPerPage();
            break;
        case ConnectedTextureMethods::RANDOM:
            layer.index = m_stitcher.mapContiguous(layer.numTiles);
            lastPageIndex = (layer.index + layer.numTiles) / m_stitcher.getTilesPerPage();
            break;
        case ConnectedTextureMethods::REPEAT:
            layer.index = m_stitcher.mapBox(layer.size.x, layer.size.y);
            lastPageIndex = (layer.index + (layer.size.y - 1) * m_stitcher.getTilesPerRow() + (layer.size.x - 1)) / m_stitcher.getTilesPerPage();
            break;
        case ConnectedTextureMethods::GRASS:
            layer.index = m_stitcher.mapContiguous(GRASS_TILES);
            lastPageIndex = (layer.index + GRASS_TILES) / m_stitcher.getTilesPerPage();
            break;
        case ConnectedTextureMethods::HORIZONTAL:
            layer.index = m_stitcher.mapContiguous(HORIZONTAL_TILES);
            lastPageIndex = (layer.index + HORIZONTAL_TILES) / m_stitcher.getTilesPerPage();
            break;
        case ConnectedTextureMethods::VERTICAL:
            layer.index = m_stitcher.mapContiguous(VERTICAL_TILES);
            lastPageIndex = (layer.index + VERTICAL_TILES) / m_stitcher.getTilesPerPage();
            break;
        case ConnectedTextureMethods::FLORA:
            layer.index = m_stitcher.mapContiguous(layer.numTiles);
            lastPageIndex = (layer.index + layer.numTiles) / m_stitcher.getTilesPerPage();
            break;
        default:
            layer.index = m_stitcher.mapSingle();
            lastPageIndex = (layer.index + 1) / m_stitcher.getTilesPerPage();
            break;
    }
    firstPageIndex = layer.index / m_stitcher.getTilesPerPage();
    flagDirtyPage(firstPageIndex);
    if (lastPageIndex != firstPageIndex) flagDirtyPage(lastPageIndex);

    // Copy data
    switch (layer.method) {
        case ConnectedTextureMethods::CONNECTED:
            writeToAtlasContiguous(layer.index, pixels, 12, 4, CONNECTED_TILES);
            break;
        case ConnectedTextureMethods::RANDOM:
            writeToAtlasContiguous(layer.index, pixels, layer.numTiles, 1, layer.numTiles);
            break;
        case ConnectedTextureMethods::REPEAT:
            writeToAtlas(layer.index, pixels, m_resolution * layer.size.x, m_resolution * layer.size.y, 1);
            break;
        case ConnectedTextureMethods::GRASS:
            writeToAtlasContiguous(layer.index, pixels, 3, 3, GRASS_TILES);
            break;
        case ConnectedTextureMethods::HORIZONTAL:
            writeToAtlasContiguous(layer.index, pixels, HORIZONTAL_TILES, 1, HORIZONTAL_TILES);
            break;
        case ConnectedTextureMethods::VERTICAL:
            writeToAtlasContiguous(layer.index, pixels, 1, VERTICAL_TILES, VERTICAL_TILES);
            break;
        case ConnectedTextureMethods::FLORA:
            writeToAtlasContiguous(layer.index, pixels, layer.size.x, layer.size.y, layer.numTiles);
            break;
        default:
            writeToAtlas(layer.index, pixels, m_resolution, m_resolution, 1);
            break;
    }

    // Cache the texture description
    AtlasTextureDescription tex;
    tex.index = layer.index;
    tex.size = layer.size;
    m_descLookup[layer.path] = tex;
}

AtlasTextureDescription BlockTexturePack::findLayer(const nString& filePath) {
    auto& it = m_descLookup.find(filePath);
    if (it != m_descLookup.end()) {
        return it->second;
    }
    return {};
}

BlockMaterial* BlockTexturePack::findTexture(const nString& filePath) {
    auto& it = m_textureLookup.find(filePath);
    if (it != m_textureLookup.end()) {
        return &m_textures[it->second];
    }
    return nullptr;
}
// Returns a pointer to the next free block texture and increments internal counter.
// Will crash if called more than m_maxTextures times.
BlockMaterial* BlockTexturePack::getNextFreeTexture() {
    if (m_nextFree >= m_maxTextures) pError("m_nextFree >= m_maxTextures in BlockTexturePack::getNextFreeTexture");
    return &m_textures[m_nextFree++];
}

void BlockTexturePack::update() {
    bool needsMipmapGen = false;
    if (m_needsRealloc) {
        allocatePages();
        m_needsRealloc = false;
        needsMipmapGen = true;
        // Upload all pages
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_atlasTexture);
        for (int i = 0; i < m_pages.size(); i++) {
            uploadPage(i);
        }
        std::vector<int>().swap(m_dirtyPages);
    } else if (m_dirtyPages.size()) {
        needsMipmapGen = true;
        // Upload dirty pages
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_atlasTexture);
        for (auto& i : m_dirtyPages) {
            uploadPage(i);
        }
        std::vector<int>().swap(m_dirtyPages);
    }

    if (needsMipmapGen) {
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
}

void BlockTexturePack::writeDebugAtlases() {
    int width = m_resolution * m_stitcher.getTilesPerRow();
    int height = width;

    int pixelsPerPage = width * height * 4;
    ui8 *pixels = new ui8[width * height * 4 * m_pages.size()];

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_atlasTexture);
    glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    for (int i = 0; i < m_pages.size(); i++) {
        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(pixels + i * pixelsPerPage, width, height, m_resolution, 4 * width, 0xFF, 0xFF00, 0xFF0000, 0x0);
        SDL_SaveBMP(surface, ("atlas" + std::to_string(i) + ".bmp").c_str());
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    delete[] pixels;
}

void BlockTexturePack::dispose() {
    if (m_atlasTexture) glDeleteTextures(1, &m_atlasTexture);
    m_atlasTexture = 0;
    for (auto& p : m_pages) {
        delete[] p.pixels;
    }
    std::vector<AtlasPage>().swap(m_pages);
    m_stitcher.dispose();
    std::map<nString, ui32>().swap(m_textureLookup);
    delete[] m_textures;
    m_textures = nullptr;
}

void BlockTexturePack::save() {

}

void BlockTexturePack::flagDirtyPage(ui32 pageIndex) {
    // If we need to allocate new pages, do so
    if (pageIndex >= m_pages.size()) {
        int i = m_pages.size();
        m_pages.resize(pageIndex + 1);
        for (; i < m_pages.size(); i++) {
            m_pages[i].pixels = new color4[m_pageWidthPixels * m_pageWidthPixels];
            memset(m_pages[i].pixels, 0, m_pageWidthPixels * m_pageWidthPixels * sizeof(color4));
        }
        m_needsRealloc = true;    
    }
    m_pages[pageIndex].dirty = true;
}

void BlockTexturePack::allocatePages() {
    // Set up the storage
    if (!m_atlasTexture) glGenTextures(1, &m_atlasTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_atlasTexture);

    // Set up all the mipmap storage
    ui32 width = m_pageWidthPixels;
    for (ui32 i = 0; i < m_mipLevels; i++) {
        glTexImage3D(GL_TEXTURE_2D_ARRAY, i, GL_RGBA8, width, width, m_pages.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        width >>= 1;
        if (width < 1) width = 1;
    }

    // Set up tex parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, (int)m_mipLevels);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD, (int)m_mipLevels);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    // Unbind
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    // Check if we had any errors
    checkGlError("BlockTexturePack::allocatePages");
}

void BlockTexturePack::uploadPage(ui32 pageIndex) {
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, pageIndex, m_pageWidthPixels, m_pageWidthPixels, 1, GL_RGBA, GL_UNSIGNED_BYTE, m_pages[pageIndex].pixels);
}

void BlockTexturePack::writeToAtlas(BlockTextureIndex texIndex, color4* pixels, ui32 pixelWidth, ui32 pixelHeight, ui32 tileWidth) {

    // Get the location in the array
    ui32 i = texIndex % m_stitcher.getTilesPerPage();
    ui32 dx = i % m_stitcher.getTilesPerRow();
    ui32 dy = i / m_stitcher.getTilesPerRow();
    ui32 pageIndex = texIndex / m_stitcher.getTilesPerPage();

    // Temp variables to reduce multiplications
    ui32 destOffset;
    ui32 pixelsOffset;
    ui32 yDestOffset;
    ui32 yPixelsOffset;
    ui32 pixelsPerRow = pixelWidth * tileWidth;

    // Start of destination
    color4* dest = m_pages[pageIndex].pixels + dx * m_resolution + dy * m_resolution * m_pageWidthPixels;
    float alpha;

    // Copy the block of pixels
    for (ui32 y = 0; y < pixelHeight; y++) {
        // Calculate Y offsets
        yDestOffset = y * m_pageWidthPixels;
        yPixelsOffset = y * pixelsPerRow;
        // Need to do alpha blending for every pixel against a black background
        for (ui32 x = 0; x < pixelWidth; x++) {
            // Calculate offsets
            destOffset = yDestOffset + x;
            pixelsOffset = yPixelsOffset + x;
            // Convert 0-255 to 0-1 for alpha mult
            alpha = (float)pixels[pixelsOffset].a / 255.0f;

            // Set the colors. Add  + 0.01f to make sure there isn't any rounding error when we truncate
            dest[destOffset].r = (ui8)((float)pixels[pixelsOffset].r * alpha + 0.01f); // R
            dest[destOffset].g = (ui8)((float)pixels[pixelsOffset].g * alpha + 0.01f); // G
            dest[destOffset].b = (ui8)((float)pixels[pixelsOffset].b * alpha + 0.01f); // B
            dest[destOffset].a = pixels[pixelsOffset].a; // A 
        }
    }
}

void BlockTexturePack::writeToAtlasContiguous(BlockTextureIndex texIndex, color4* pixels, ui32 width, ui32 height, ui32 numTiles) {
    ui32 pixelsPerTileRow = m_resolution * m_resolution * width;

    ui32 n = 0;
    for (ui32 y = 0; y < height; y++) {
        for (ui32 x = 0; x < width && n < numTiles; x++, n++) {
            // Get pointer to source data
            color4* src = pixels + y * pixelsPerTileRow + x * m_resolution;

            writeToAtlas(texIndex++, src, m_resolution, m_resolution, width);
        }
    }
}
