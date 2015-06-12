///
/// ImageAssetLoader.h
///
/// Created by Cristian Zaloj on 13 Feb 2015
///
/// Summary:
/// Loads image assets.
///

#pragma once

#ifndef ImageAssetLoader_h__
#define ImageAssetLoader_h__

#include <Vorb/AssetLoader.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/Texture.h>

class ImageAsset : public vcore::Asset {
public:
    vg::Texture texture;
};

template<>
struct vcore::AssetBuilder<ImageAsset> {
public:
    void create(const vpath& p, OUT ImageAsset* asset, vcore::RPCManager& rpc);
    void destroy(ImageAsset* asset);
private:
    vg::ImageIO m_imageLoader;
};

CONTEXTUAL_ASSET_LOADER(ImageAssetLoader, ImageAsset);

#endif // ImageAssetLoader_h__
