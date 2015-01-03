#include "stdafx.h"
#include "Texture2d.h"

#include <glm\gtc\quaternion.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <TTF/SDL_ttf.h>

#include "BlockData.h"
#include "ChunkManager.h"
#include "Errors.h"
#include "Options.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "ZipFile.h"

//TODO(Ben): This is bad
vg::Texture getTexture(nString source, Animation **anim) {
    vg::TextureCache* textureCache = GameManager::textureCache;

    vg::Texture texture = textureCache->findTexture(source);

    if (texture.id) {
        return texture;
    } else {
        vg::Texture newTexInfo;
        int pos = source.find("%TexturePack");
        // It's in the texture pack
        if (pos != nString::npos) {
            nString s = source;
            s.replace(pos, 13, "");
            nString fileName = graphicsOptions.currTexturePack;
            bool isZip = fileName.substr(fileName.size() - 4) == ".zip";
            if (!isZip) fileName += "/";

            if (isZip) {
                ZipFile zipFile(fileName); //maybe keep this open
                size_t filesize;
                unsigned char *zipData = zipFile.readFile(s, filesize);

                //loadPNG(newTexInfo, zipData, filesize, PNGLoadInfo(textureSamplers + 1, 12), true);

                if (anim) {

                    //change the extension to .anim and search for an animation
                    if (s.substr(s.size() - 4) == ".png") {
                        s.erase(s.size() - 4);
                        s += ".anim";
                        *anim = fileManager.loadAnimation(s, &zipFile);
                    }
                }

            } else {
                newTexInfo = textureCache->addTexture(fileName + s);

                if (anim) {

                    //change the extension to .anim and search for an animation
                    if (s.substr(s.size() - 4) == ".png") {
                        s.erase(s.size() - 4);
                        s += ".anim";
                        *anim = fileManager.loadAnimation(fileName + s);
                    }
                }
            }
        } else {
            newTexInfo = textureCache->addTexture(source);
        }
        return newTexInfo;
    }
}

// TODO: I will place a pinecone in my ass if this is not removed
vg::Texture markerTexture;
vg::Texture terrainTexture;
vg::Texture sunTexture;
vg::Texture logoTexture;
vg::Texture cloudTexture1;
vg::Texture normalLeavesTexture, pineLeavesTexture, mushroomCapTexture, treeTrunkTexture1;
vg::Texture waterNormalTexture;
vg::Texture WaterTexture;
vg::Texture skyboxTextures[6];
vg::Texture ballMaskTexture;
vg::Texture BlankTextureID;
vg::Texture explosionTexture;
vg::Texture fireTexture;
vg::Texture waterNoiseTexture;
vg::Texture crosshairTexture;

BlockTexturePack blockPack;

void BlockTexturePack::initialize(vg::Texture texInfo) {
    textureInfo = texInfo;
}