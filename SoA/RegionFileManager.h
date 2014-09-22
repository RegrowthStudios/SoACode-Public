#pragma once
#include "Constants.h"

#include <deque>
#include <map>

//Size of a sector in bytes
#define SECTOR_SIZE 512

//Make sure REGION_WIDTH is 2 ^ RSHIFT and
//REGION_SIZE is REGION_WIDTH ^ 3 and
//REGION_LAYER is REGION_WIDTH ^ 2
#define RSHIFT 4
#define REGION_WIDTH 16
#define REGION_LAYER 256
#define REGION_SIZE 4096


#define REGION_VER_0 1000
#define CHUNK_VER_0 1000

#define CURRENT_REGION_VER REGION_VER_0
#define CURRENT_CHUNK_VER CHUNK_VER_0

#define CHUNK_DATA_SIZE (CHUNK_SIZE * 4) //right now a voxel is 4 bytes

#define COMPRESSION_RLE 0x1
#define COMPRESSION_ZLIB 0x10

//All data is stored in byte arrays so we can force it to be saved in big-endian
struct ChunkHeader {
    ui8 compression[4];
    ui8 timeStamp[4];
    ui8 voxelDataSize[4]; //size including the header
    ui8 auxDataSize[4]; //size of all other data, i.e. particles, entities, ect.
};

struct RegionFileHeader {
    ui8 lookupTable[REGION_SIZE * 4];
};

struct RegionFile {
    RegionFileHeader header;
    nString region;
    FILE* file;
    int fileDescriptor;
    i32 totalSectors;
    bool isHeaderDirty;
};

struct SaveVersion {
    ui8 regionVersion[4];
    ui8 chunkVersion[4];
};

class Chunk;

class RegionFileManager {
public:
    RegionFileManager();
    ~RegionFileManager();

    void clear();

    bool openRegionFile(nString region, i32 face, bool create);

    bool tryLoadChunk(Chunk* chunk);
    bool saveChunk(Chunk* chunk);

    void flush();

    bool saveVersionFile();
    bool checkVersion();
private:
    void closeRegionFile(RegionFile* regionFile);

    bool readChunkHeader();
    bool readVoxelData_v0();

    void getIterationConstantsFromRotation(int rotation, int& jStart, int& jMult, int& jEnd, int& jInc, int& kStart, int& kMult, int& kEnd, int& kInc);

    int rleUncompressArray(ui8* data, ui32& byteIndex, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc);
    int rleUncompressArray(ui16* data, ui32& byteIndex, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc);
    bool fillChunkVoxelData(Chunk* chunk);

    bool saveRegionHeader();
    bool loadRegionHeader();

    void rleCompressArray(ui8* data, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc);
    void rleCompressArray(ui16* data, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc);
    bool rleCompressChunk(Chunk* chunk);
    bool zlibCompress();

    bool tryConvertSave(ui32 regionVersion, ui32 chunkVersion);

    bool writeSectors(ui8* srcBuffer, ui32 size);
    bool readSectors(ui8* dstBuffer, ui32 size);

    bool seek(ui32 byteOffset);
    bool seekToChunk(ui32 chunkSectorOffset);

    ui32 getChunkSectorOffset(Chunk* chunk, ui32* retTableOffset = nullptr);
    nString getRegionString(Chunk* chunk);
    
    //Byte buffer for reading chunk data
    ui32 _bufferSize;
    ui8 _byteBuffer[CHUNK_DATA_SIZE];
    //Byte buffer for compressed data. It is slighly larger because of worst case with RLE
    uLongf _compressedBufferSize;
    ui8 _compressedByteBuffer[CHUNK_DATA_SIZE + CHUNK_SIZE * 4 + sizeof(ChunkHeader)];
    //Dynamic byte buffer used in copying contents of a file for resize
    ui32 _copySectorsBufferSize;
    ui8* _copySectorsBuffer;

    ui16 blockIDBuffer[CHUNK_SIZE];
    ui8 sunlightBuffer[CHUNK_SIZE];
    ui16 lampLightBuffer[CHUNK_SIZE];

    ui8 _chunkHeaderBuffer[sizeof(ChunkHeader)];
    ui8 _regionFileHeaderBuffer[sizeof(RegionFileHeader)];

    ui32 _maxCacheSize;
    std::map <nString, RegionFile*> _regionFileCache;
    std::deque <RegionFile*> _regionFileCacheQueue;

    RegionFile* _regionFile;
    ChunkHeader _chunkHeader;
};