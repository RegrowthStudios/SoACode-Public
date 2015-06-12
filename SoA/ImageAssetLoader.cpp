#include "stdafx.h"
#include "ImageAssetLoader.h"

#include <Vorb/graphics/GLStates.h>

void vcore::AssetBuilder<ImageAsset>::create(const vpath& p, OUT ImageAsset* asset, vcore::RPCManager& rpc) {
    vg::BitmapResource bmp = m_imageLoader.load(p, vg::ImageIOFormat::RGBA_UI8);
    if (!bmp.data) return;

    asset->texture.width = bmp.width;
    asset->texture.height = bmp.height;

    GLRPC so(asset);
    so.set([bmp](Sender, void* userData) {
        ImageAsset* asset = (ImageAsset*)userData;

        // Create the texture
        glGenTextures(1, &asset->texture.id);
        glBindTexture(GL_TEXTURE_2D, asset->texture.id);
        vg::SamplerState::LINEAR_WRAP.set(GL_TEXTURE_2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, asset->texture.width, asset->texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp.data);
        glBindTexture(GL_TEXTURE_2D, 0);
    });
    rpc.invoke(&so);

    vg::ImageIO::free(bmp);
}

void vcore::AssetBuilder<ImageAsset>::destroy(ImageAsset* asset) {
    glDeleteTextures(1, &asset->texture.id);
}
