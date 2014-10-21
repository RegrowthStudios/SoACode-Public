#include "stdafx.h"
#include "TextureCache.h"

#include <vector>

namespace vg {

TextureCache::TextureCache()
{
}


TextureCache::~TextureCache()
{
}


bool TextureCache::addTexture(nString filePath, TextureLoadParams texParams) {
    std::vector <ui8> pixelStore;
    ui32 width;
    ui32 height;

    ImageLoader::loadPng(filePath.c_str(), pixelStore, width, height, true);

    //TODO(Ben): Use GPUMemory

    ui32 textureID;

    _textures[filePath] = textureID;
    return true;
}

void TextureCache::addTexture(nString filePath, ui32 textureID) {
    _textures[filePath] = textureID;
}

}