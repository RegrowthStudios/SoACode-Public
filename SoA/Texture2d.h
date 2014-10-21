#pragma once
#include "stdafx.h"
#include <SDL/SDL.h>

#include "Constants.h"
#include "ImageLoading.h"
#include "TextureCache.h"

// TODO: Remove This
using namespace std;

extern int screenWidth2d, screenHeight2d;

extern std::map <std::string, vg::Texture> textureMap;

vg::Texture getTexture(string source, struct Animation **anim = NULL);

struct BlockPack
{
    void initialize(vg::Texture texInfo);
    vg::Texture textureInfo;
    vector <GLubyte[256][3]> avgColors;
};

extern BlockPack blockPack; //TODO: Not global

extern vg::Texture markerTexture;
extern vg::Texture terrainTexture;
extern vg::Texture logoTexture;
extern vg::Texture sunTexture;
extern vg::Texture waterNormalTexture;
extern vg::Texture cloudTexture1;
extern vg::Texture WaterTexture;
extern vg::Texture normalLeavesTexture, pineLeavesTexture, mushroomCapTexture, treeTrunkTexture1;
extern vg::Texture ballMaskTexture;
extern vg::Texture starboxTextures[6];
extern vg::Texture BlankTextureID;
extern vg::Texture explosionTexture;
extern vg::Texture fireTexture;
extern vg::Texture waterNoiseTexture;

class Color{
public:
    Color() : color(1.0f), mod(1.0f){ dynCol[0] = dynCol[1] = dynCol[2] = NULL; }
    Color(glm::vec4 col) : color(col), mod(1.0f){ dynCol[0] = dynCol[1] = dynCol[2] = NULL; }

    void update(float Mod = 1.0f){
        if (dynCol[0]) color.r = (*(dynCol[0])) / 255.0f;
        if (dynCol[1]) color.g = (*(dynCol[1])) / 255.0f;
        if (dynCol[2]) color.b = (*(dynCol[2])) / 255.0f;
        mod = Mod;
    }

    glm::vec4 color;
    int *dynCol[3];
    float mod;
};

void bindBlockPacks();
void ReloadTextures();
void LoadTextures();
void FreeTextures();

void InitializeTTF();