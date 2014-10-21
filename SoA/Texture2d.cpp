#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "Texture2d.h"

#include <TTF/SDL_ttf.h>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "BlockData.h"
#include "ChunkManager.h"
#include "Errors.h"

#include "Options.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "ZipFile.h"

std::vector<glm::vec2> vertices;
std::vector<glm::vec2> UVs;

GLfloat textColorVertices[16384];

std::map <std::string, vg::Texture> textureMap;

//TODO(Ben): This is bad
vg::Texture getTexture(string source, Animation **anim)
{
    vg::TextureCache* textureCache = GameManager::textureCache;

    auto it = textureMap.find(source);
    
    if (it != textureMap.end()){
        return it->second;
    }else{
        vg::Texture newTexInfo;
        int pos = source.find("%TexturePack");
        //its in the texture pack
        if (pos != string::npos){
            string s = source;
            s.replace(pos, 13, "");
            string fileName = graphicsOptions.currTexturePack;
            bool isZip = fileName.substr(fileName.size() - 4) == ".zip";
            if (!isZip) fileName += "/";

            if (isZip){
                ZipFile zipFile(fileName); //maybe keep this open
                size_t filesize;
                unsigned char *zipData = zipFile.readFile(s, filesize);
                
                //loadPNG(newTexInfo, zipData, filesize, PNGLoadInfo(textureSamplers + 1, 12), true);

                if (anim){
                    
                    //change the extension to .anim and search for an animation
                    if (s.substr(s.size() - 4) == ".png"){
                        s.erase(s.size() - 4);
                        s += ".anim";
                        *anim = fileManager.loadAnimation(s, &zipFile);
                    }
                }

            }
            else{
                newTexInfo = textureCache->addTexture(fileName + s);

                if (anim){
                
                    //change the extension to .anim and search for an animation
                    if (s.substr(s.size() - 4) == ".png"){
                        s.erase(s.size() - 4);
                        s += ".anim";
                        *anim = fileManager.loadAnimation(fileName + s);
                    }
                }
            }
        }
        else{
            newTexInfo = textureCache->addTexture(source);
        }
        textureMap.insert(make_pair(source, newTexInfo));
        return newTexInfo;
    }
}

BlockPack blockPack;

vg::Texture markerTexture;
vg::Texture terrainTexture;
vg::Texture sunTexture;
vg::Texture logoTexture;
vg::Texture cloudTexture1;
vg::Texture normalLeavesTexture, pineLeavesTexture, mushroomCapTexture, treeTrunkTexture1;
vg::Texture waterNormalTexture;
vg::Texture WaterTexture;
vg::Texture starboxTextures[6];
vg::Texture ballMaskTexture;
vg::Texture BlankTextureID;
vg::Texture explosionTexture;
vg::Texture fireTexture;
vg::Texture waterNoiseTexture;

//const GLushort boxDrawIndices[6] = {0,1,2,2,3,0};
//const GLfloat boxUVs[8] = {0, 1, 0, 0, 1, 0, 1, 1};

int screenWidth2d = 1366, screenHeight2d = 768;


TTF_Font *mainFont;
void InitializeTTF()
{
    if(TTF_Init()==-1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        pError("TTF COULD NOT INIT!");
        exit(331);
    }

    mainFont = TTF_OpenFont("Fonts/orbitron_bold-webfont.ttf", 32);
    if(!mainFont) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        pError("TTF could not open font!");
        exit(331);
    }
    // handle error
}

void BlockPack::initialize(vg::Texture texInfo)
{
    textureInfo = texInfo;
}

void bindBlockPacks()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.ID);
}

void ReloadTextures()
{
    printf("Reloading Textures...\n");
    LoadTextures();
    graphicsOptions.currTexturePack = "";
    fileManager.loadTexturePack("Textures/TexturePacks/" + graphicsOptions.texturePackString);
}

void LoadTextures()
{
    vg::TextureCache* textureCache = GameManager::textureCache;

    logoTexture = textureCache->addTexture("Textures/logo.png");
    sunTexture = textureCache->addTexture("Textures/sun_texture.png");
    BlankTextureID = textureCache->addTexture("Textures/blank.png", &SamplerState::POINT_CLAMP);
    explosionTexture = textureCache->addTexture("Textures/explosion.png");
    fireTexture = textureCache->addTexture("Textures/fire.png");
}

void FreeTextures()
{
    //for (int i = 0; i < blockPacks.size(); i++) blockPacks[i].textureInfo.freeTexture();
    //blockPacks.clear();
    vg::TextureCache* textureCache = GameManager::textureCache;

    textureCache->freeTexture(markerTexture);
    textureCache->freeTexture(terrainTexture);
    textureCache->freeTexture(sunTexture);
    textureCache->freeTexture(normalLeavesTexture);
    textureCache->freeTexture(pineLeavesTexture);
    textureCache->freeTexture(mushroomCapTexture);
    textureCache->freeTexture(treeTrunkTexture1);
    textureCache->freeTexture(waterNormalTexture);
    textureCache->freeTexture(starboxTextures[0]);
    textureCache->freeTexture(starboxTextures[1]);
    textureCache->freeTexture(starboxTextures[2]);
    textureCache->freeTexture(starboxTextures[3]);
    textureCache->freeTexture(starboxTextures[4]);
    textureCache->freeTexture(starboxTextures[5]);
    textureCache->freeTexture(BlankTextureID);
    textureCache->freeTexture(ballMaskTexture);
}