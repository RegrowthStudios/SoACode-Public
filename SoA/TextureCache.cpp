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

ui32 TextureCache::findTexture(const nString& filePath) {
    auto it = _textureStringMap.find(filePath);
    if (it != _textureStringMap.end()) {
        return it->second;
    }
    return 0;
}

nString TextureCache::getTexturePath(ui32 textureID) {
    auto it = _textureIdMap.find(textureID);
    if (it != _textureIdMap.end()) {
        return it->second->first;
    }
    return "";
}

ui32 TextureCache::addTexture(const nString& filePath,
                                SamplerState* samplingParameters /* = &SamplerState::LINEAR_WRAP_MIPMAP */,
                                i32 mipmapLevels /* = INT_MAX */) {

    // Check if its already cached
    ui32 textureID = findTexture(filePath);
    if (textureID) return textureID;

    // Buffer for the pixels
    std::vector <ui8> pixelStore;
    ui32 width;
    ui32 height;

    // Load the pixel data
    if (!ImageLoader::loadPng(filePath.c_str(), pixelStore, width, height, true)) {
        return 0;
    }

    // Upload the texture through GpuMemory
    textureID = GpuMemory::uploadTexture(pixelStore.data(), width, height, samplingParameters, mipmapLevels);

    // Store the texture in the cache
    insertTexture(filePath, textureID);
    return textureID;
}

ui32 TextureCache::addTexture(const nString& filePath,
                              const ui8* pixels,
                              ui32 width,
                              ui32 height,
                              SamplerState* samplingParameters /* = &SamplerState::LINEAR_WRAP_MIPMAP */,
                              i32 mipmapLevels /* = INT_MAX */) {
    // Check if its already cached
    ui32 textureID = findTexture(filePath);
    if (textureID) return textureID;

    // Upload the texture through GpuMemory
    textureID = GpuMemory::uploadTexture(pixels, width, height, samplingParameters, mipmapLevels);

    // Store the texture in the cache
    insertTexture(filePath, textureID);
    return textureID;
}

void TextureCache::addTexture(const nString& filePath, ui32 textureID) {
    insertTexture(filePath, textureID);
}

void TextureCache::freeTexture(const nString& filePath) {
    // Check the cache for the texture
    auto it = _textureStringMap.find(filePath);
    if (it != _textureStringMap.end()) {
        // Erase from the set ( must be done before freeTexture )
        _textureIdMap.erase(it->second);
        // If we have the texture, free it
        GpuMemory::freeTexture(it->second);
        // Remove from the map
        _textureStringMap.erase(it);
       
    }
}

void TextureCache::freeTexture(ui32 textureID) {
    auto it = _textureIdMap.find(textureID);
    if (it != _textureIdMap.end()) {
        // Free the texture
        GpuMemory::freeTexture(textureID);
        // Quickly erase from the string map since we have the iterator
        _textureStringMap.erase(it->second);
     
        _textureIdMap.erase(it);
    }
}

void TextureCache::destroy() {
    for (auto tex : _textureStringMap) {
        GpuMemory::freeTexture(tex.second);
    }
    std::unordered_map <nString, ui32>().swap(_textureStringMap); ///< Textures store here keyed on filename
    std::map <ui32, std::unordered_map <nString, ui32>::iterator>().swap(_textureIdMap);
}

void TextureCache::insertTexture(const nString& filePath, ui32 textureID) {
    // We store an iterator to the map node in the _textureIdMap
    // so that we can quickly remove textures from the cache
    _textureIdMap[textureID] = _textureStringMap.insert(std::make_pair(filePath, textureID)).first;
}

}
}
}