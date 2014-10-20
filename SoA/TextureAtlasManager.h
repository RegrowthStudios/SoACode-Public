#pragma once
#include "Constants.h"
#include "FrameBuffer.h"
#include "ImageLoading.h"
#include <set>

struct BlockTexture;
class ZipFile;

static const int BLOCKS_PER_ROW = 16;

class TextureAtlasManager {
public:
    TextureAtlasManager();
    ~TextureAtlasManager();
    void loadBlockAtlas(std::string fileName);
    void addBlockTexture(std::string file);

    //Clears textures and blockTexturesToLoad
    void clearAll();

    bool getIsLoaded() const { return _isLoaded; }
    BlockTexture* getBlockTexture(const std::string& key);

private:

    //Refers to a page of the atlas array
    class Atlas {
    public:
        Atlas(int resolution) {
            memset(freeSlots, 1, sizeof(freeSlots));
            frameBuffer = new FrameBuffer(GL_RGBA, GL_UNSIGNED_BYTE, resolution * BLOCKS_PER_ROW, resolution * BLOCKS_PER_ROW);
        }
        ~Atlas() {
            delete frameBuffer;
        }
        //Available slots
        bool freeSlots[256];
        //The framebuffer for this atlas
        FrameBuffer *frameBuffer;
    };




    const BlockTexture* addTextureToAtlas(string tileFileName, ZipFile* zipFile = nullptr);
    ui32 makeBlockPackTexture(const std::vector <Atlas*> &atlasList, int imageWidth);

    void addRandomTexture(const Array<i32>& weights, int& texIndex, int& numTiles, int& totalWeight);

    void writeToAtlas(int &texIndex, f32 uStart = 0.0f, f32 vStart = 0.0f, f32 uWidth = 1.0f, f32 vWidth = 1.0f);
    void writeToAtlasRepeat(int &texIndex, int width, int height);
    void writeToAtlasContiguous(int &texIndex, int width, int height, int numTiles);

    void writeDebugAtlases();

    std::set <std::string> _blockTexturesToLoad;
    std::map <std::string, BlockTexture> _overlayTextureCache;
    std::map <std::string, BlockTexture> _blockTextureCache;

    std::vector <Atlas*> _atlasList;

    ZipFile* _defaultZipFile;
    std::string _defaultFileName;
    std::string _fileName;
    bool _isDefaultZip;
    bool _isLoaded;

    TextureInfo _readTexture;

    int _currentAtlas;
    int _oldestFreeSlot;
    int _resolution;
};