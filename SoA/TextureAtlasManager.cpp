#include "stdafx.h"
#include "TextureAtlasManager.h"

#include "BlockData.h"
#include "FileSystem.h"
#include "Options.h"
#include "Texture2d.h"
#include "ZipFile.h"

SamplerState BLOCK_PACK_SAMPLING_STATE(TextureMinFilter::LINEAR_MIPMAP_LINEAR, TextureMagFilter::NEAREST,
    TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);

TextureAtlasManager::TextureAtlasManager() : _currentAtlas(0), _oldestFreeSlot(0), _isLoaded(false) {
    _defaultFileName = "Textures/TexturePacks/" + graphicsOptions.defaultTexturePack;

    _isDefaultZip = (_defaultFileName.substr(_defaultFileName.size() - 4) == ".zip");

    if (!_isDefaultZip) {
        _defaultFileName += "/";
    }
}

TextureAtlasManager::~TextureAtlasManager() {
    clearAll();
}

//Loads a texture pack and generates a block atlas texture array
void TextureAtlasManager::loadBlockAtlas(string fileName) {
    
    blockPack.textureInfo.freeTexture();
    _overlayTextureCache.clear();
    _blockTextureCache.clear();
    _atlasList.clear();
    _isLoaded = false;

    _oldestFreeSlot = 0;

    _defaultZipFile = nullptr;
    if (_isDefaultZip) {
        _defaultZipFile = new ZipFile(_defaultFileName);

        //if the zipfile doesnt exist, recover
        if (_defaultZipFile->isFailure()) {
            _defaultFileName = _defaultFileName.substr(0, _defaultFileName.size() - 4);
            _isDefaultZip = 0;
            delete _defaultZipFile;
            _defaultZipFile = nullptr;
        }
    }

    _fileName = fileName;

    TextureInfo atlasTex;

    ZipFile *zipFile = nullptr;

    if (_fileName.substr(_fileName.size() - 4) == ".zip") {
        zipFile = new ZipFile(_fileName);
    }

    //Get Resolution
    _resolution = graphicsOptions.currTextureRes;

    atlasTex.width = atlasTex.height = _resolution * BLOCKS_PER_ROW;

    _atlasList.push_back(new Atlas(_resolution));
    _atlasList.back()->frameBuffer->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //add missing texture image
    loadPNG(_readTexture, "Textures/missing_texture.png", PNGLoadInfo(&BLOCK_PACK_SAMPLING_STATE, 12), true);

    if (_readTexture.ID != 0){
        int tmp;
        writeToAtlas(tmp);
        _readTexture.freeTexture();
    }
    //skip missing texture and default overlay
    _atlasList[0]->freeSlots[1] = false;
    _oldestFreeSlot = 2;

    //make atlas
    for (auto it = _blockTexturesToLoad.begin(); it != _blockTexturesToLoad.end(); it++) {
        addTextureToAtlas(*it, zipFile);
    }

    ui32v2 viewport(graphicsOptions.windowWidth, graphicsOptions.windowHeight);
    _atlasList.back()->frameBuffer->unBind(viewport);

    atlasTex.ID = makeBlockPackTexture(_atlasList, _resolution * BLOCKS_PER_ROW);
    blockPack.initialize(atlasTex);

    //TEMPORARY DEBUG CODE
    writeDebugAtlases();

    for (int i = 0; i < _atlasList.size(); i++) {
        delete _atlasList[i];
    }
    _atlasList.clear();

    delete zipFile;
    delete _defaultZipFile;

    _isLoaded = true;
}

const BlockTexture* TextureAtlasManager::addTextureToAtlas(string tileFileName, ZipFile* zipFile) {

    BlockTexture blockTexture = {};
    //Set base path. This can be overwritten
    size_t filesize;
    unsigned char *zipData;

    bool isDefault = (_defaultFileName == _fileName);

    //Check if its already cached
    auto it = _blockTextureCache.find(tileFileName);
    if (it != _blockTextureCache.end()) {
        return &(it->second);
    }

    //Load the texture
    _readTexture.ID = 0;
    if (zipFile) {
        zipData = zipFile->readFile(tileFileName, filesize);
        if (zipData == nullptr) {
            cout << "Error: " << tileFileName << " not found in texture pack " + _fileName << endl;
        } else {
            loadPNG(_readTexture, zipData, filesize, PNGLoadInfo(&BLOCK_PACK_SAMPLING_STATE, 12), true);

            //TODO: Zip tex support
        }
    } else {

        string texFileName = tileFileName;
     
        if (texFileName.substr(texFileName.size() - 4) == ".png") {
            texFileName.erase(texFileName.size() - 4);
            texFileName += ".tex";

            fileManager.loadTexFile(_fileName + texFileName, nullptr, &blockTexture);
          
            char tmp;
            if (blockTexture.overlay.path.empty()) {
                blockTexture.overlay.textureIndex = 1;
            } else {
                //Overlay Texture
                auto it = _overlayTextureCache.find(blockTexture.overlay.path);
                //It isn't in the cache
                if (it == _overlayTextureCache.end()) {

                    //Load the texture from file
                    loadPNG(_readTexture, (_fileName + blockTexture.overlay.path).c_str(), PNGLoadInfo(&BLOCK_PACK_SAMPLING_STATE, 12), true);

                    if (_readTexture.ID != 0) {

                        switch (blockTexture.overlay.method) {
                        case ConnectedTextureMethods::CTM_CONNECTED:
                            writeToAtlasContiguous(blockTexture.overlay.textureIndex, 12, 4, 47);
                            break;
                        case ConnectedTextureMethods::CTM_RANDOM:
                            addRandomTexture(blockTexture.overlay.weights, blockTexture.overlay.textureIndex,
                                             blockTexture.overlay.numTiles, blockTexture.overlay.totalWeight);
                            break;
                        case ConnectedTextureMethods::CTM_GRASS:
                            writeToAtlasContiguous(blockTexture.overlay.textureIndex, 3, 3, 9);
                            break;
                        case ConnectedTextureMethods::CTM_REPEAT:
                            writeToAtlasRepeat(blockTexture.overlay.textureIndex, blockTexture.overlay.size.x, blockTexture.overlay.size.y);
                            break;
                        case ConnectedTextureMethods::CTM_HORIZONTAL:
                            writeToAtlasContiguous(blockTexture.overlay.textureIndex, 4, 1, 4);
                            break;
                        case ConnectedTextureMethods::CTM_VERTICAL:
                            writeToAtlasContiguous(blockTexture.overlay.textureIndex, 1, 4, 4);
                            break;
                        default:
                            writeToAtlas(blockTexture.overlay.textureIndex);
                            break;
                        }
                        _readTexture.freeTexture();
                        //Add overlay texture info to cache
                        _overlayTextureCache[blockTexture.overlay.path] = blockTexture;
                    }

                } else {
                    //Grab the overlay stuff from the cached texture
                    blockTexture.overlay = it->second.overlay;
                    //blockTexture.overlayHeight = it->second.overlayHeight;
                    //blockTexture.overlayWidth = it->second.overlayWidth;
                    //blockTexture.overlayNumTiles = it->second.overlayNumTiles;
                    //blockTexture.overlayMethod = it->second.overlayMethod;
                    //blockTexture.overlayTotalWeight = it->second.overlayTotalWeight;
                    //blockTexture.overlayWeights = it->second.overlayWeights;
                    //blockTexture.overlayInnerSeams = it->second.overlayInnerSeams;
                    //blockTexture.overlaySymmetry = it->second.overlaySymmetry;
                    //blockTexture.overlayUseMapColor = it->second.overlayUseMapColor;
                }
            }

            //If we have an explicit base path, we can just use its base info and add to cache
            if (!blockTexture.base.path.empty()) {
                const BlockTexture* baseTexture = addTextureToAtlas(blockTexture.base.path);
                blockTexture.base = baseTexture->base;
                //blockTexture.innerSeams = baseTexture->innerSeams;
                //blockTexture.width = baseTexture->width;
                //blockTexture.height = baseTexture->height;
                //blockTexture.numTiles = baseTexture->numTiles;
                //blockTexture.method = baseTexture->method;
                //blockTexture.totalWeight = baseTexture->totalWeight;
                //blockTexture.weights = baseTexture->weights;
                //blockTexture.symmetry = baseTexture->symmetry;
                //blockTexture.useMapColor = baseTexture->useMapColor;
                blockTexture.blendMode = baseTexture->blendMode;

                _blockTextureCache[tileFileName] = blockTexture;
                return &(_blockTextureCache[tileFileName]);
            }
        }

        //Check if its already cached
        auto it = _blockTextureCache.find(tileFileName);
        if (it != _blockTextureCache.end()) {
            return &(it->second);
        }
        
        loadPNG(_readTexture, (_fileName + tileFileName).c_str(), PNGLoadInfo(&BLOCK_PACK_SAMPLING_STATE, 12), true);

    }

    //load it from the default pack if its missing
    if (_readTexture.ID == 0 && isDefault == 0) {
        if (_isDefaultZip) {
            zipData = _defaultZipFile->readFile(tileFileName, filesize);
            if (zipData == nullptr) {
                cout << "Error: " << tileFileName << " not found in texture pack " + _defaultFileName << endl;
            } else {
                loadPNG(_readTexture, zipData, filesize, PNGLoadInfo(&BLOCK_PACK_SAMPLING_STATE, 12), true);
            }
        } else {
            string texFileName = tileFileName;

            if (texFileName.substr(texFileName.size() - 4) == ".png") {
                texFileName.erase(texFileName.size() - 4);
                texFileName += ".tex";

                blockTexture = {};
                fileManager.loadTexFile(_defaultFileName + texFileName, nullptr, &blockTexture);
            }

            loadPNG(_readTexture, (_defaultFileName + tileFileName).c_str(), PNGLoadInfo(&BLOCK_PACK_SAMPLING_STATE, 12), true);
        }
    }

    //check for missing texture
    if (_readTexture.ID != 0) {

        switch (blockTexture.base.method) {
        case ConnectedTextureMethods::CTM_CONNECTED:
            writeToAtlasContiguous(blockTexture.base.textureIndex, 12, 4, 47);
            break;
        case ConnectedTextureMethods::CTM_RANDOM:
            addRandomTexture(blockTexture.base.weights, blockTexture.base.textureIndex, blockTexture.base.numTiles, blockTexture.base.totalWeight);
            break;
        case ConnectedTextureMethods::CTM_REPEAT:
            writeToAtlasRepeat(blockTexture.base.textureIndex, blockTexture.base.size.x, blockTexture.base.size.y);
            break;
        case ConnectedTextureMethods::CTM_GRASS:
            writeToAtlasContiguous(blockTexture.base.textureIndex, 3, 3, 9);
            break;
        case ConnectedTextureMethods::CTM_HORIZONTAL:
            writeToAtlasContiguous(blockTexture.base.textureIndex, 4, 1, 4);
            break;
        case ConnectedTextureMethods::CTM_VERTICAL:
            writeToAtlasContiguous(blockTexture.base.textureIndex, 1, 4, 4);
            break;
        default:
            writeToAtlas(blockTexture.base.textureIndex);
            break;
        }
 
        _blockTextureCache[tileFileName] = blockTexture;

        _readTexture.freeTexture();

        return &(_blockTextureCache[tileFileName]);
    }

    return NULL;
}

void TextureAtlasManager::addRandomTexture(const Array<i32>& weights, int& texIndex, int& numTiles, int& totalWeight) {
    int NumTiles = _readTexture.width / _readTexture.height;
    numTiles = NumTiles;
    if (weights.length() == 0) {
       totalWeight = numTiles;
    }
    writeToAtlasContiguous(texIndex, numTiles, 1, numTiles);
}

void TextureAtlasManager::writeToAtlas(int &texIndex, f32 uStart, f32 vStart, f32 uWidth, f32 vWidth) {
    Texture2D tempTexture2D;

    int i;
    int atlasIndex;

    //Find the next free slot
    do {
        i = _oldestFreeSlot % 256;
        atlasIndex = _oldestFreeSlot / 256;

        //If we need to alocate a new atlas
        if (atlasIndex >= _atlasList.size()) {
            _atlasList.push_back(new Atlas(_resolution));
            _atlasList.back()->frameBuffer->bind();
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        _oldestFreeSlot++;
    } while (_atlasList[atlasIndex]->freeSlots[i] == false);

    //If we need to switch atlases, bind it
    if (_currentAtlas != atlasIndex) {
        _currentAtlas = atlasIndex;
        _atlasList[atlasIndex]->frameBuffer->bind();
    }

    //mark this slot as not free
    _atlasList[_currentAtlas]->freeSlots[i] = false;

    texIndex = atlasIndex * 256 + i;

    //write to the atlas using a temporary Texture2D
    tempTexture2D.Initialize(_readTexture.ID, 0, 0, _resolution, _resolution, Color(glm::vec4(1.0)), uStart, vStart, uWidth, vWidth);
    tempTexture2D.Draw((i % BLOCKS_PER_ROW) * _resolution, (15 - (i % 256) / BLOCKS_PER_ROW) * _resolution, _resolution*BLOCKS_PER_ROW, _resolution*BLOCKS_PER_ROW);
}

void TextureAtlasManager::writeToAtlasRepeat(int &texIndex, int width, int height) {

    Texture2D tempTexture2D;

    if (width > 16 || height > 16) {
        pError("Repeat texture width or height > 16. Must be <= 16!");
        return;
    }

    int i;
    int atlasIndex;
    int x, y;
    
    //Start the search at the oldest known free spot.
    int searchIndex = _oldestFreeSlot;
    bool fits;
    //Find the next free slot that is large enough
    while (true) {
        i = searchIndex % 256;
        atlasIndex = searchIndex / 256;
        x = i % 16;
        y = i / 16;
        fits = true;

        //If we need to alocate a new atlas
        if (atlasIndex >= _atlasList.size()) {
            _atlasList.push_back(new Atlas(_resolution));
            _atlasList.back()->frameBuffer->bind();
            _currentAtlas = atlasIndex;
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        //if it doesn't fit in Y direction, go to next page
        if (y + height > 16) {
            searchIndex += 256 - i;
            continue;
        }
        //if it doesnt fit in X direction, go to next row
        if (x + width > 16) {
            searchIndex += 16 - x;
            continue;
        }

        searchIndex++;

        //Search to see if all needed slots are free
        for (int j = y; j < y + height; j++) {
            for (int k = x; k < x + width; k++) {
                if (_atlasList[atlasIndex]->freeSlots[j * 16 + k] == false) {
                    fits = false;
                    j = 100; //force to fall out of loop
                    break;
                }
            }
        }

        if (fits) {
            //if we reach here, it will fit at this position
            break;
        }
    }

    //If we need to switch atlases, bind it
    if (_currentAtlas != atlasIndex) {
        _currentAtlas = atlasIndex;
        _atlasList[atlasIndex]->frameBuffer->bind();
    }

    //Set all free slots to false
    for (int j = y; j < y + height; j++) {
        for (int k = x; k < x + width; k++) {
            _atlasList[atlasIndex]->freeSlots[j * 16 + k] = false;
        }
    }

    texIndex = i + atlasIndex * 256;

    //write to the atlas using a temporary Texture2D
    tempTexture2D.Initialize(_readTexture.ID, 0, 0, _resolution * width, _resolution * height, Color(glm::vec4(1.0)));
    tempTexture2D.Draw((i % BLOCKS_PER_ROW) * _resolution, (15 - ((i % 256) / BLOCKS_PER_ROW + (height - 1))) * _resolution, _resolution*BLOCKS_PER_ROW, _resolution*BLOCKS_PER_ROW);
}

void TextureAtlasManager::writeToAtlasContiguous(int &texIndex, int width, int height, int numTiles) {

    float tileResU = 1.0f / width;
    float tileResV = 1.0f / height;

    int i;
    int atlasIndex;
    int numContiguous = 0;
    //Start the search at the oldest known free spot.
    int searchIndex = _oldestFreeSlot;
    bool passedFreeSlot = false;

    //Find the next free slot that is large enough
    while (true) {
        i = searchIndex % 256;
        atlasIndex = searchIndex / 256;
      
        //If we need to alocate a new atlas
        if (atlasIndex >= _atlasList.size()) {
            _atlasList.push_back(new Atlas(_resolution));
            _atlasList.back()->frameBuffer->bind();
            _currentAtlas = atlasIndex;
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        searchIndex++;
        if (_atlasList[atlasIndex]->freeSlots[i] == false) {
            if (numContiguous) {
                passedFreeSlot = true;
            }
            numContiguous = 0;
        } else {
            numContiguous++;
        }

        //Stop searching if we have found a contiguous block that is large enough
        if (numContiguous == numTiles) {
            i = searchIndex % 256;
            atlasIndex = searchIndex / 256;
            break;
        }
    }

    //Move the oldest known free slot forward if we havent passed a free spot
    if (passedFreeSlot == false) {
        _oldestFreeSlot = i + atlasIndex * 256;
    }

    int index = i + atlasIndex * 256 - numTiles;

    texIndex = index;
    float uStart, vStart;
    int n = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width && n < numTiles; x++, n++) {

            i = index % 256;
            atlasIndex = index / 256;
            uStart = x * tileResU;
            vStart = ((height - 1) - y) * tileResV;

            Texture2D tempTexture2D;

            //If we need to switch atlases, bind it
            if (_currentAtlas != atlasIndex) {
                _currentAtlas = atlasIndex;
                _atlasList[atlasIndex]->frameBuffer->bind();
            }

            _atlasList[atlasIndex]->freeSlots[i] = false;

            //write to the atlas using a temporary Texture2D
            tempTexture2D.Initialize(_readTexture.ID, 0, 0, _resolution, _resolution, Color(glm::vec4(1.0)), uStart, vStart, tileResU, tileResV);
            tempTexture2D.Draw((i % BLOCKS_PER_ROW) * _resolution, (15 - (i % 256) / BLOCKS_PER_ROW) * _resolution, _resolution*BLOCKS_PER_ROW, _resolution*BLOCKS_PER_ROW);
            index++;
        }
    }
}

void TextureAtlasManager::addBlockTexture(string file) {
    _blockTexturesToLoad.insert(file);
}

void TextureAtlasManager::clearAll() {
    blockPack.textureInfo.freeTexture();
    _blockTexturesToLoad.clear();
    _overlayTextureCache.clear();
    _blockTextureCache.clear();
    _atlasList.clear();
    _isLoaded = false;
}

ui32 TextureAtlasManager::makeBlockPackTexture(const vector <Atlas*> &atlasList, int imageWidth) {

    const ui32v2 viewport(graphicsOptions.windowWidth, graphicsOptions.windowHeight);
    _atlasList.back()->frameBuffer->unBind(viewport);

    printf("Creating texture atlas that is %u pages.\n", atlasList.size());

    GLuint textureID;
    glGenTextures(1, &textureID);

    int level = 0;
    int width = imageWidth;
    while (width > 16) {
        width >>= 1;
        level++;
    }

    // Set up the storage
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    width = imageWidth;
    for (i32 i = 0; i < level; i++) {
        glTexImage3D(GL_TEXTURE_2D_ARRAY, i, GL_RGBA8, width, width, atlasList.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        width >>= 1;
        if (width < 1) width = 1;
    }

    ui8* data = new GLubyte[_resolution * _resolution * BLOCKS_PER_ROW * BLOCKS_PER_ROW * 4];
    //Pull all the atlases in the array texture
    for (int i = 0; i < atlasList.size(); i++) {

        glBindTexture(GL_TEXTURE_2D, _atlasList[i]->frameBuffer->renderedTextureIDs[FB_DRAW]);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, imageWidth, imageWidth, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
        checkGlError("Adding Atlas Page " + to_string(i));
    }

    delete[] data;


    //TODO: Should this be above glGenerateMipmap?
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, level);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD, level);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    checkGlError("makeBlockPackTexture()");

    return textureID;
}

BlockTexture* TextureAtlasManager::getBlockTexture(const std::string& key) {

    auto it = _blockTextureCache.find(key);
    if (it == _blockTextureCache.end()) {
        return nullptr;
    } else {
        return &(it->second);
    }
}

//Will dump all atlases to files for debugging
void TextureAtlasManager::writeDebugAtlases() {
    int width = _atlasList.back()->frameBuffer->getWidth();
    int height = _atlasList.back()->frameBuffer->getHeight();
    GLubyte *pixels = new GLubyte[width * height * 4];
    GLubyte *flip = new GLubyte[width * height * 4];
    for (int i = 0; i < _atlasList.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, _atlasList[i]->frameBuffer->renderedTextureIDs[FB_DRAW]);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        int k = height - 1;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width * 4; x++){
                flip[y * 4 * width + x] = pixels[k * 4 * width + x];
            }
            k--;
        }

        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(flip, width, height, 32, 4 * width, 0xFF, 0xFF00, 0xFF0000, 0x0);
        SDL_SaveBMP(surface, ("atlas" + to_string(i) + ".bmp").c_str());

    }
    delete[] pixels;
    delete[] flip;
}