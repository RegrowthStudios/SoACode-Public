#include "stdafx.h"
#include "TextureCache.h"

#include "GpuMemory.h"
#include "Errors.h"

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

Texture TextureCache::findTexture(const nString& filePath) {
    auto it = _textureStringMap.find(filePath);
    if (it != _textureStringMap.end()) {
        return it->second;
    }

    return Texture();
}

nString TextureCache::getTexturePath(ui32 textureID) {
    auto it = _textureIdMap.find(textureID);
    if (it != _textureIdMap.end()) {
        return it->second->first;
    }
    return "";
}

Texture TextureCache::addTexture(const nString& filePath,
                                SamplerState* samplingParameters /* = &SamplerState::LINEAR_CLAMP_MIPMAP */,
                                i32 mipmapLevels /* = INT_MAX */) {

    // Check if its already cached
    Texture texture = findTexture(filePath);
    if (texture.ID) return texture;

    // Buffer for the pixels
    std::vector <ui8> pixelStore;

    // Load the pixel data
    if (!ImageLoader::loadPng(filePath.c_str(), pixelStore, texture.width, texture.height, true)) {
        return Texture();
    }

    // Upload the texture through GpuMemory
    texture.ID = GpuMemory::uploadTexture(pixelStore.data(), texture.width, texture.height, samplingParameters, mipmapLevels);

    // Store the texture in the cache
    insertTexture(filePath, texture);
    return texture;
}

Texture TextureCache::addTexture(const nString& filePath,
                              const ui8* pixels,
                              ui32 width,
                              ui32 height,
                              SamplerState* samplingParameters /* = &SamplerState::LINEAR_CLAMP_MIPMAP */,
                              i32 mipmapLevels /* = INT_MAX */) {
    // Check if its already cached
    Texture texture = findTexture(filePath);
    if (texture.ID) return texture;

    // Upload the texture through GpuMemory
    texture.ID = GpuMemory::uploadTexture(pixels, texture.width, texture.height, samplingParameters, mipmapLevels);

    // Store the texture in the cache
    insertTexture(filePath, texture);
    return texture;
}

void TextureCache::addTexture(const nString& filePath, const Texture& texture) {
    insertTexture(filePath, texture);
}

void TextureCache::freeTexture(const nString& filePath) {
    // Check the cache for the texture
    auto it = _textureStringMap.find(filePath);
    if (it != _textureStringMap.end()) {
        // Erase from the set ( must be done before freeTexture )
        _textureIdMap.erase(it->second.ID);
        // If we have the texture, free it
        GpuMemory::freeTexture(it->second.ID);
        // Remove from the map
        _textureStringMap.erase(it);
       
    }
}

void TextureCache::freeTexture(ui32& textureID) {
    auto it = _textureIdMap.find(textureID);
    if (it != _textureIdMap.end()) {
        // Free the texture
        GpuMemory::freeTexture(textureID);
        // Quickly erase from the string map since we have the iterator
        _textureStringMap.erase(it->second);
     
        _textureIdMap.erase(it);
    }
}

void TextureCache::freeTexture(Texture& texture) {
    auto it = _textureIdMap.find(texture.ID);
    if (it != _textureIdMap.end()) {
        // Free the texture
        GpuMemory::freeTexture(texture.ID);
        // Quickly erase from the string map since we have the iterator
        _textureStringMap.erase(it->second);

        _textureIdMap.erase(it);
    }
}

void TextureCache::destroy() {
    for (auto tex : _textureStringMap) {
        GpuMemory::freeTexture(tex.second.ID);
    }
    std::unordered_map <nString, Texture>().swap(_textureStringMap); ///< Textures store here keyed on filename
    std::map <ui32, std::unordered_map <nString, Texture>::iterator>().swap(_textureIdMap);
}

void TextureCache::insertTexture(const nString& filePath, const Texture& texture) {
    // We store an iterator to the map node in the _textureIdMap
    // so that we can quickly remove textures from the cache
    _textureIdMap[texture.ID] = _textureStringMap.insert(std::make_pair(filePath, texture)).first;
}

}
}
}