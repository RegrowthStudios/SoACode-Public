///
/// ShaderAssetLoader.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 4 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles loading shader assets
///

#pragma once

#ifndef ShaderAssetLoader_h__
#define ShaderAssetLoader_h__


#include <Vorb/graphics/GLProgram.h>
#include <Vorb/AssetLoader.h>

class ShaderAsset : public vcore::Asset {
public:
    vg::GLProgram program;
};

template<>
struct vcore::AssetBuilder<ShaderAsset> {
public:
    void create(const vpath& p, OUT ShaderAsset* asset, vcore::RPCManager& rpc);
    void destroy(ShaderAsset* asset);
};

CONTEXTUAL_ASSET_LOADER(ShaderAssetLoader, ShaderAsset);

#endif // ShaderAssetLoader_h__
