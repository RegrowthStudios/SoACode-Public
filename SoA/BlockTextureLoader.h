///
/// BlockTextureLoader.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 16 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Loads block textures
///

#pragma once

#ifndef BlockTextureLoader_h__
#define BlockTextureLoader_h__

class ModPathResolver;
class BlockTexturePack;

#include <Vorb/io/IOManager.h>

class BlockTextureLoader {
public:
    void init(ModPathResolver* texturePathResolver, BlockTexturePack* texturePack);

    void loadTexture(OUT BlockTexture& texture, const nString& filePath);

private:
    bool loadLayer(BlockTextureLayer& layer);
    bool loadTexFile(const nString& imagePath, BlockTexture& texture);
    bool postProcessLayer(vg::ScopedBitmapResource& bitmap, BlockTextureLayer& layer);

    ModPathResolver* m_texturePathResolver = nullptr;
    BlockTexturePack* m_texturePack = nullptr;
    vio::IOManager m_iom;
};

#endif // BlockTextureLoader_h__
