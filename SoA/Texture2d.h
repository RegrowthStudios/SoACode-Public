#pragma once
#include <SDL/SDL.h>
#include <Windows.h>

#include "Constants.h"
#include "ImageLoading.h"

// TODO: Remove This
using namespace std;

extern int screenWidth2d, screenHeight2d;

extern std::map <std::string, TextureInfo> textureMap;

struct TextureInfo getTextureInfo(string source, struct Animation **anim = NULL);

struct BlockPack
{
    void initialize(TextureInfo texInfo);
    TextureInfo textureInfo;
    vector <GLubyte[256][3]> avgColors;
};

extern BlockPack blockPack; //TODO: Not global

extern TextureInfo markerTexture;
extern TextureInfo terrainTexture;
extern TextureInfo logoTexture;
extern TextureInfo sunTexture;
extern TextureInfo waterNormalTexture;
extern TextureInfo cloudTexture1;
extern TextureInfo WaterTexture;
extern TextureInfo normalLeavesTexture, pineLeavesTexture, mushroomCapTexture, treeTrunkTexture1;
extern TextureInfo ballMaskTexture;
extern TextureInfo starboxTextures[6];
extern TextureInfo BlankTextureID;
extern TextureInfo explosionTexture;
extern TextureInfo fireTexture;
extern TextureInfo waterNoiseTexture;

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

void InitializeText2D(const char *texturePath, const char *dataPath);
void SetText(class Texture2D &tex, const char * text, int x, int y, int size, int fontIndex, int justification = 0, int maxWidth = 0, int ycentered = 1, glm::vec4 color = glm::vec4(1.0));
void PrintText(const char * text, int x, int y, int size, int fontIndex, int justification=0, int ycentered = 1, glm::vec4 color = glm::vec4(1.0));
//void ClearText();
//void AddText2D(const char * text, int x, int y, int size, int fontIndex, int justification=0, int centered = 1, glm::vec4 color = glm::vec4(1.0));
void CleanupText2D();
//    TEXTURE2D

extern void DrawImage2D(GLfloat *vertices, int sizeOfvertices, GLfloat *uvs, int sizeOfuvs, GLushort *indices, int sizeOfindices, GLuint textureID, glm::vec4 color, bool roundMask = 0, float xdim = 1360.0f, float ydim = 768.0f);

class Texture2D
{
public:
    Texture2D() : text(""), texSource(""), label(""), maxWidth(0), textSize(10){
        Initialize(0, 0, 0, 0, 0); //initialize to defaults
    }

    void Initialize(GLuint texID, GLfloat sx, GLfloat sy, GLfloat w, GLfloat h, Color color = Color(), GLfloat ustrt = 0.0f, GLfloat vstrt = 0.0f, GLfloat uwidth = 1.0f, GLfloat vwidth = 1.0f);
    void update();
    void SetPosition(int x, int y, int w, int h);
    void SetUvs(GLfloat ustrt = 0.0f, GLfloat vstrt = 0.0f, GLfloat uwidth = 1.0f, GLfloat vwidth = 1.0f);
    void SetColor(Color color){
        lcolor = rcolor = color;
    }
    void SetFade(float fad){fade = fad;}
    void SetHalfColor(Color color, int side)
    {
        if (side == 0){
            lcolor = color;
        }else{
            rcolor = color;
        }
    }

    void Draw(int xmod = 0, int ymod = 0, GLfloat xdim = 1360.0f, GLfloat ydim = 768.0f);
    void DrawFixedSize3D(const glm::mat4 &VP, const glm::dvec3 &playerPos, const glm::dvec3 &pos, glm::vec2 uvMod, glm::vec4 color);

    int xpos, ypos;
    TextureInfo texInfo;
    string texSource;
    GLfloat width, height;
    GLfloat uvs[8];
    GLfloat vertices[8];
    int textSize;
    int maxWidth;
    string text;
    string label;
    float fade;
    Color lcolor;
    Color rcolor;
};

struct TextInfo
{
    TextInfo();
    TextInfo(string txt);
    void Draw(int xmod = 0, int ymod = 0);
    void update(const char *ntext, int x, int y, int size, int justification=0);
    void SetPosMod(int Xm, int Ym){ xm = Xm; ym = Ym; }
    string text;
    Texture2D texture;
    int justification;
    int xm, ym;
};