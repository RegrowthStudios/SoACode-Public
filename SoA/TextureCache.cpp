#include "stdafx.h"
#include "TextureCache.h"

#include "GpuMemory.h"

#include <vector>

namespace vorb {
namespace core {
namespace graphics {

TextureCache::TextureCache() {
    // Empty
}


TextureCache::~TextureCache() {
    destroy();
}


ui32 TextureCache::addTexture(const nString& filePath,
                                SamplerState* samplingParameters,
                                i32 mipmapLevels /* = INT_MAX */) {

    // Buffer for the pixels
    std::vector <ui8> pixelStore;
    ui32 width;
    ui32 height;

    // Load the pixel data
    if (!ImageLoader::loadPng(filePath.c_str(), pixelStore, width, height, true)) {
        return 0;
    }

    // Upload the texture through GpuMemory
    ui32 textureID = GpuMemory::uploadTexture(pixelStore, width, height, samplingParameters, mipmapLevels);

    // Store the texture in the cache
    _textures[filePath] = textureID;
    return textureID;
}

ui32 TextureCache::addTexture(const nString& filePath,
                              const std::vector<ui8>& pixels,
                              ui32 width,
                              ui32 height,
                              SamplerState* samplingParameters,
                              i32 mipmapLevels /* = INT_MAX */) {

    // Upload the texture through GpuMemory
    ui32 textureID = GpuMemory::uploadTexture(pixels, width, height, samplingParameters, mipmapLevels);

    // Store the texture in the cache
    _textures[filePath] = textureID;
    return textureID;
}

ui32 TextureCache::addTexture(const nString& filePath,
                              const ui8* pixels,
                              ui32 width,
                              ui32 height,
                              SamplerState* samplingParameters,
                              i32 mipmapLevels /* = INT_MAX */) {

    // Upload the texture through GpuMemory
    ui32 textureID = GpuMemory::uploadTexture(pixels, width, height, samplingParameters, mipmapLevels);

    // Store the texture in the cache
    _textures[filePath] = textureID;
    return textureID;
}

void TextureCache::addTexture(const nString& filePath, ui32 textureID) {
    _textures[filePath] = textureID;
}

void TextureCache::freeTexture(const nString& filePath) {
    // Check the cache for the texture
    auto it = _textures.find(filePath);
    if (it != _textures.end()) {
        // If we have the texture, free it
        GpuMemory::freeTexture(it->second);
        // Remove from the cache
        _textures.erase(it);
    }
}

void TextureCache::destroy() {
    for (auto tex : _textures) {
        GpuMemory::freeTexture(tex.second);
    }
    std::unordered_map <nString, ui32>().swap(_textures);
}

}
}
}