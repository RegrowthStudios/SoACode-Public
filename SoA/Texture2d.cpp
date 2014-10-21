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

std::map <std::string, TextureInfo> textureMap;

TextureInfo getTextureInfo(string source, Animation **anim)
{
    auto it = textureMap.find(source);
    if (it != textureMap.end()){
        return it->second;
    }else{
        TextureInfo newTexInfo;
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
                loadPNG(newTexInfo, zipData, filesize, PNGLoadInfo(textureSamplers + 1, 12), true);

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
                loadPNG(newTexInfo, (fileName + s).c_str(), PNGLoadInfo(textureSamplers + 1, 12));

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
            loadPNG(newTexInfo, source.c_str(), PNGLoadInfo(textureSamplers + 1, 12));
        }
        textureMap.insert(make_pair(source, newTexInfo));
        return newTexInfo;
    }
}

BlockPack blockPack;

TextureInfo markerTexture;
TextureInfo terrainTexture;
TextureInfo sunTexture;
TextureInfo logoTexture;
TextureInfo cloudTexture1;
TextureInfo normalLeavesTexture, pineLeavesTexture, mushroomCapTexture, treeTrunkTexture1;
TextureInfo waterNormalTexture;
TextureInfo WaterTexture;
TextureInfo starboxTextures[6];
TextureInfo ballMaskTexture;
TextureInfo BlankTextureID;
TextureInfo explosionTexture;
TextureInfo fireTexture;
TextureInfo waterNoiseTexture;

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

void SetText(Texture2D &tex, const char * text, int x, int y, int size, int fontIndex, int justification, int maxWidth, int ycentered, glm::vec4 color){
    SDL_Color col;
    col.r = color.r*255;
    col.g = color.g*255;
    col.b = color.b*255;
    col.a = color.a*255;
    tex.textSize = size;
    

    if (strlen(text) == 0){
        tex.text = "";
        if (tex.texInfo.ID != 0){
            glDeleteTextures(1, &tex.texInfo.ID);
            tex.texInfo.ID = 0;
        }
        return; //dont render empty string
    }

    SDL_Surface *surface;
    
    if (maxWidth == 0){
        surface = TTF_RenderText_Blended(mainFont, text, col);
    }else{
        surface = TTF_RenderText_Blended_Wrapped(mainFont, text, col, maxWidth);
    }

    if (surface == NULL){
        pError("PrintText surface was NULL");
        return;
    }

    GLint colors = surface->format->BytesPerPixel;
    GLenum format;
    if (colors == 4) {   // alpha
        if (surface->format->Rmask == 0x000000ff)
            format = GL_RGBA;
        else
            format = GL_BGRA;
    } else if (colors == 3) {             // no alpha
        if (surface->format->Rmask == 0x000000ff)
            format = GL_RGB;
        else
            format = GL_BGR;
    }else if (colors == 1){
        format = GL_ALPHA;
    }else{
        pError("Font not valid truecolor font");
    }
    GLuint texture;

    if (tex.texInfo.ID != 0){
        texture = tex.texInfo.ID;
    }else{
        glGenTextures(1, &texture);
    }
    glBindTexture(GL_TEXTURE_2D, texture); 

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D(GL_TEXTURE_2D, 0, colors, surface->w, surface->h, 0,
                        format, GL_UNSIGNED_BYTE, surface->pixels);

    GLfloat width, height;
    width = surface->w*size/32.0f;
    height = surface->h*size / 32.0f;
    if (ycentered){
        y += height*0.5;
    }
    if (justification == 1){ //centered
        tex.Initialize(texture, x-width*0.5, y, width, -height);
    }else if (justification == 2){ //right justified
        tex.Initialize(texture, x-width, y, width, -height);
    }else{ //left justified
        tex.Initialize(texture, x, y, width, -height);
    }

    tex.text = text;
    tex.texInfo.width = width;
    tex.texInfo.height = height;

    SDL_FreeSurface(surface);
}

void PrintText(const char * text, int x, int y, int size, int fontIndex, int justification, int ycentered, glm::vec4 color){
    SDL_Color col;
    col.r = color.r*255;
    col.g = color.g*255;
    col.b = color.b*255;
    col.a = color.a*255;
    
    if (strlen(text) == 0) return; //dont render empty string

    SDL_Surface *surface = TTF_RenderText_Blended(mainFont, text, col);

    if (surface == NULL){
        pError("PrintText surface was NULL");
        return;
    }

    GLint colors = surface->format->BytesPerPixel;
    GLenum format;
    if (colors == 4) {   // alpha
        if (surface->format->Rmask == 0x000000ff)
            format = GL_RGBA;
        else
            format = GL_BGRA;
    } else if (colors == 3) {             // no alpha
        if (surface->format->Rmask == 0x000000ff)
            format = GL_RGB;
        else
            format = GL_BGR;
    }else if (colors == 1){
        format = GL_ALPHA;
    }else{
        pError("Font not valid truecolor font");
    }
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); 

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D(GL_TEXTURE_2D, 0, colors, surface->w, surface->h, 0,
                        format, GL_UNSIGNED_BYTE, surface->pixels);

    Texture2D tex;


    GLfloat width, height;
    width = surface->w*size / 32.0f;
    height = surface->h*size / 32.0f;
    if (ycentered){
        y -= (int)(height*0.5f);
    }
    if (justification == 1){ //centered
        tex.Initialize(texture, x-width*0.5, y+height, width, -height);
    }else if (justification == 2){ //right justified
        tex.Initialize(texture, x-width, y+height, width, -height);
    }else{ //left justified
        tex.Initialize(texture, x, y+height, width, -height);
    }
    glDisable(GL_CULL_FACE);
    tex.Draw();
    glEnable(GL_CULL_FACE);

    glDeleteTextures(1, &texture);
    SDL_FreeSurface(surface);
    
}

void ClearText(){
    vertices.clear();
    UVs.clear();
}

void BlockPack::initialize(TextureInfo texInfo)
{
    //GLubyte buffer[16][16][4];
    //int width = 0;
    //int mipLevel = 0;
    //int internalFormat;

    textureInfo = texInfo;


    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D_ARRAY, textureInfo.ID);
 //   glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &width);

    //while (width > 16){
    //    width = width / 2;
    //    mipLevel++;
    //}
 //   glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    //    
    //if (internalFormat != GL_RGBA){
    //    cout << "ERROR: Internal format for texture pack is not GL_RGBA. Tell Ben!\n";
    //    int a;
    //    cin >> a;
    //}
    //
 //   glGetTexImage(GL_TEXTURE_2D_ARRAY, mipLevel, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    //cout << endl << endl;
    //for (int i = 0; i < 16; i++){
    //    for (int j = 0; j < 16; j++){
    //        //note that the y axis is reversed
    //        avgColors[i*16+j][0] = buffer[15-i][j][0];
    //        avgColors[i*16+j][1] = buffer[15-i][j][1];
    //        avgColors[i*16+j][2] = buffer[15-i][j][2];

    //        //cout << (int)buffer[15-i][j][0] << " " << (int)buffer[15-i][j][1] << " " << (int)buffer[15-i][j][2] << "    ";
    //    }
    //}

    //convert rgb values into a hex int
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
    //TextureInfo blockPackInfo;
    //loadPNG(blockPackInfo, ("Textures/" + TexturePackString + "BlockPack1.png").c_str(), 4);
    //blockPacks.push_back(BlockPack(blockPackInfo));
    //loadPNG(blockPackInfo, ("Textures/" + TexturePackString + "BlockPack2.png").c_str(), 4);
    //blockPacks.push_back(BlockPack(blockPackInfo));
    logoTexture.freeTexture();
    loadPNG(logoTexture, "Textures/logo.png", PNGLoadInfo(textureSamplers + 1, 12));
    sunTexture.freeTexture();
    loadPNG(sunTexture, "Textures/sun_texture.png", PNGLoadInfo(textureSamplers + 1, 12));
    BlankTextureID.freeTexture();
    loadPNG(BlankTextureID, "Textures/blank.png", PNGLoadInfo(textureSamplers, 2));
    explosionTexture.freeTexture();
    loadPNG(explosionTexture, "Textures/explosion.png", PNGLoadInfo(textureSamplers + 1, 12));
    fireTexture.freeTexture();
    loadPNG(fireTexture, "Textures/fire.png", PNGLoadInfo(textureSamplers + 1, 12));
}

void FreeTextures()
{
    //for (int i = 0; i < blockPacks.size(); i++) blockPacks[i].textureInfo.freeTexture();
    //blockPacks.clear();

    markerTexture.freeTexture();
    terrainTexture.freeTexture();
    sunTexture.freeTexture();
    normalLeavesTexture.freeTexture();
    pineLeavesTexture.freeTexture();
    mushroomCapTexture.freeTexture();
    treeTrunkTexture1.freeTexture();
    waterNormalTexture.freeTexture();
    starboxTextures[0].freeTexture();
    starboxTextures[1].freeTexture();
    starboxTextures[2].freeTexture();
    starboxTextures[3].freeTexture();
    starboxTextures[4].freeTexture();
    starboxTextures[5].freeTexture();
    BlankTextureID.freeTexture();
    ballMaskTexture.freeTexture();
}

void Texture2D::Initialize(GLuint texID, GLfloat sx, GLfloat sy, GLfloat w, GLfloat h, Color color, GLfloat ustrt, GLfloat vstrt, GLfloat uwidth, GLfloat vwidth)
{
    fade = 1.0f;
    xpos = (int)sx;
    ypos = (int)sy;
    texInfo.ID = texID;
    width = w;
    height = h;
    uvs[0] = ustrt;
    uvs[1] = vstrt + vwidth;
    uvs[2] = ustrt;
    uvs[3] = vstrt;
    uvs[4] = ustrt + uwidth;
    uvs[5] = vstrt;
    uvs[6] = ustrt + uwidth;
    uvs[7] = vstrt + vwidth;


    lcolor = rcolor = color;

    vertices[0] = sx;
    vertices[1] = sy + h;
    vertices[2] = sx;
    vertices[3] = sy;
    vertices[4] = sx + w;
    vertices[5] = sy;
    vertices[6] = sx + w;
    vertices[7] = sy + h;
}

//for dynamic textures
void Texture2D::update(){
    if (texSource.size()){
        if (texSource == "TEXTUREPACK"){
            texInfo.ID = blockPack.textureInfo.ID;
        }
    }
}

void Texture2D::SetPosition(int x, int y, int w, int h)
{
    xpos = x;
    ypos = y;
    width = w;
    height = h;
    vertices[0] = x;
    vertices[1] = y + height;
    vertices[2] = x;
    vertices[3] = y;
    vertices[4] = x + width;
    vertices[5] = y;
    vertices[6] = x + width;
    vertices[7] = y + height;
}

void Texture2D::SetUvs(GLfloat ustrt, GLfloat vstrt, GLfloat uwidth, GLfloat vwidth)
{
    uvs[0] = ustrt;
    uvs[1] = vstrt + vwidth;
    uvs[2] = ustrt;
    uvs[3] = vstrt;
    uvs[4] = ustrt + uwidth;
    uvs[5] = vstrt;
    uvs[6] = ustrt + uwidth;
    uvs[7] = vstrt + vwidth;
}

GLfloat colorVertices2D[32];


// Jesus christ this is shit
// TODO(Ben): This is fucking terrible
void Texture2D::Draw(int xmod, int ymod, GLfloat xdim, GLfloat ydim)
{
    if (texInfo.ID == 0) return;

    // Buffers are lazily initialized
    static ui32 vboID = 0;
    static ui32 uvBufferID = 0;
    static ui32 elementBufferID = 0;
    static ui32 colorBufferID = 0;
    if (vboID == 0) {
        glGenBuffers(1, &vboID);
        glGenBuffers(1, &uvBufferID);
        glGenBuffers(1, &elementBufferID);
        glGenBuffers(1, &colorBufferID);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat), uvs, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLushort), boxDrawIndices, GL_STATIC_DRAW);

    lcolor.update(lcolor.mod);
    rcolor.update(rcolor.mod);

    int j = 0;
    for (int i = 0; i < 2; i++, j+=4){
        colorVertices[j] = lcolor.color[0] * lcolor.mod;
        colorVertices[j + 1] = lcolor.color[1] * lcolor.mod;
        colorVertices[j + 2] = lcolor.color[2] * lcolor.mod;
        colorVertices[j + 3] = lcolor.color[3] * fade;
    }
    for (int i = 0; i < 2; i++, j+=4){
        colorVertices[j] = rcolor.color[0] * rcolor.mod;
        colorVertices[j + 1] = rcolor.color[1] * rcolor.mod;
        colorVertices[j + 2] = rcolor.color[2] * rcolor.mod;
        colorVertices[j + 3] = rcolor.color[3] * fade;
    }

    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glBufferData(GL_ARRAY_BUFFER, 16*sizeof(GLfloat), colorVertices, GL_STATIC_DRAW);

    // Bind shader
    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("Texture2D");
    
    program->use();
    program->enableVertexAttribArrays();

    glUniform1f(program->getUniform("xdim"), xdim);
    glUniform1f(program->getUniform("ydim"), ydim);
    
    glUniform1i(program->getUniform("roundMaskTexture"), 1);
    glUniform1f(program->getUniform("isRound"), 0.0f);

    glUniform1f(program->getUniform("xmod"), (GLfloat)xmod);
    glUniform1f(program->getUniform("ymod"), (GLfloat)ymod);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texInfo.ID);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(program->getUniform("myTextureSampler"), 0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glVertexAttribPointer(program->getAttribute("vertexPosition_screenspace"), 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glVertexAttribPointer(program->getAttribute("vertexUV"), 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 3rd attribute buffer : Colors
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glVertexAttribPointer(program->getAttribute("vertexColor"), 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Draw call
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    program->disableVertexAttribArrays();
    program->unuse();
}

void Texture2D::DrawFixedSize3D(const glm::mat4 &VP, const glm::dvec3 &playerPos, const glm::dvec3 &pos, glm::vec2 uvMod, glm::vec4 color)
{
    
    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("FixedSizeBillboard");
 
    program->use();
    program->disableVertexAttribArrays();

    GlobalModelMatrix[3][0] = 0.0;
    GlobalModelMatrix[3][1] = 0.0;
    GlobalModelMatrix[3][2] = 0.0;

    if (texInfo.ID == 0) return;
    vector <FixedSizeBillboardVertex> billVerts;

    int n = 0;
    billVerts.resize(4);
    billVerts[n].pos = pos - playerPos;
    billVerts[n + 1].pos = pos - playerPos;
    billVerts[n + 2].pos = pos - playerPos;
    billVerts[n + 3].pos = pos - playerPos;
    billVerts[n].uv[0] = 255;
    billVerts[n].uv[1] = 255;
    billVerts[n + 1].uv[0] = 0;
    billVerts[n + 1].uv[1] = 255;
    billVerts[n + 2].uv[0] = 0;
    billVerts[n + 2].uv[1] = 0;
    billVerts[n + 3].uv[0] = 255;
    billVerts[n + 3].uv[1] = 0;

    // Buffers are lazily initialized
    static ui32 vertexBufferID = 0;
    static ui32 elementBufferID = 0;
    if (vertexBufferID == 0) {
        glGenBuffers(1, &vertexBufferID);
        glGenBuffers(1, &elementBufferID);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FixedSizeBillboardVertex)* billVerts.size(), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(FixedSizeBillboardVertex)* billVerts.size(), &(billVerts[0]));
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FixedSizeBillboardVertex), 0);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(FixedSizeBillboardVertex), (void *)12);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), boxDrawIndices, GL_STREAM_DRAW);

    glm::mat4 MVP = VP;
    glUniform2f(program->getUniform("UVstart"), 0.0f, 1.0f);
    float width = (float)texInfo.width / screenWidth2d;
    float height = ((float)texInfo.height * ((float)screenHeight2d / screenWidth2d)) / screenHeight2d;
    glUniform1f(program->getUniform("width"), width);
    glUniform1f(program->getUniform("height"), height);
    glUniform2f(program->getUniform("UVwidth"), 1.0f, -1.0f);
    glUniform4f(program->getUniform("particleColor"), color.r, color.g, color.b, color.a);
    glUniform1f(program->getUniform("textureUnitID"), 0.0f);
    glUniform2f(program->getUniform("UVmod"), uvMod.x, uvMod.y);

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texInfo.ID);

    glDisable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    glEnable(GL_CULL_FACE);
    
    program->disableVertexAttribArrays();
    program->unuse(); 
}

TextInfo::TextInfo():  text(""), justification(0), xm(0), ym(0){}
TextInfo::TextInfo(string txt): text(txt), justification(0), xm(0), ym(0){}

void TextInfo::Draw(int xmod , int ymod){
    texture.Draw(xmod+xm, ymod+ym);
}
void TextInfo::update(const char *ntext, int x, int y, int size, int justification){
    if (text != ntext){
        text = ntext;
        SetText(texture, ntext, x, y, size, 0, justification);
    }
}