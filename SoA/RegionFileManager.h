#pragma once
#include <deque>
#include <map>

#include <ZLIB/zconf.h>
#include <Vorb/Vorb.h>

#include "Constants.h"
#include "VoxelCoordinateSpaces.h"

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

#define CURRENT_REGION_VER REGION_VER_0

#define CHUNK_DATA_SIZE (CHUNK_SIZE * 4) //right now a voxel is 4 bytes

#define COMPRESSION_RLE 0x1
#define COMPRESSION_ZLIB 0x10

//All data is stored in byte arrays so we can force it to be saved in big-endian
class ChunkHeader {
public:
    ui8 compression[4];
    ui8 timeStamp[4];
    ui8 dataLength[4]; //length of the data
};

class RegionFileHeader {
public:
    ui8 lookupTable[REGION_SIZE * 4];
};

class RegionFile {
public:
    RegionFileHeader header;
    nString region;
    FILE* file;
    int fileDescriptor;
    i32 totalSectors;
    bool isHeaderDirty;
};

class SaveVersion {
public:
    ui8 regionVersion[4];
    ui8 chunkVersion[4];
};

class Chunk;

class RegionFileManager {
public:
    RegionFileManager(const nString& saveDir);
    ~RegionFileManager();

    void clear();

    bool openRegionFile(nString region, const ChunkPosition3D& gridPosition, bool create);

    bool tryLoadChunk(Chunk* chunk);
    bool saveChunk(Chunk* chunk);

    void flush();

    bool saveVersionFile();
    bool checkVersion();
private:
    void closeRegionFile(RegionFile* regionFile);

    bool readChunkHeader();
    bool readChunkData_v0();

    int rleUncompressArray(ui8* data, ui32& byteIndex, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc);
    int rleUncompressArray(ui16* data, ui32& byteIndex, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc);
    bool fillChunkVoxelData(Chunk* chunk);

    bool saveRegionHeader();
    bool loadRegionHeader();

    void rleCompressArray(ui8* data, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc);
    void rleCompressArray(ui16* data, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc);
    bool rleCompressChunk(Chunk* chunk);
    bool zlibCompress();

    bool tryConvertSave(ui32 regionVersion);

    bool writeSectors(ui8* srcBuffer, ui32 size);
    bool readSectors(ui8* dstBuffer, ui32 size);

    bool seek(ui32 byteOffset);
    bool seekToChunk(ui32 chunkSectorOffset);

    ui32 getChunkSectorOffset(Chunk* chunk, ui32* retTableOffset = nullptr);
    nString getRegionString(Chunk* chunk);
    
    //Byte buffer for reading chunk data
    ui32 _bufferSize;
    ui8 _chunkBuffer[CHUNK_DATA_SIZE];
    //Byte buffer for compressed data. It is slightly larger because of worst case with RLE
    uLongf _compressedBufferSize;
    ui8 _compressedByteBuffer[CHUNK_DATA_SIZE + CHUNK_SIZE * 4 + sizeof(ChunkHeader)];
    //Dynamic byte buffer used in copying contents of a file for resize
    ui32 _copySectorsBufferSize;
    ui8* _copySectorsBuffer;

    ui16 _blockIDBuffer[CHUNK_SIZE];
    ui8 _sunlightBuffer[CHUNK_SIZE];
    ui16 _lampLightBuffer[CHUNK_SIZE];
    ui16 _tertiaryDataBuffer[CHUNK_SIZE];

    ui8 _chunkHeaderBuffer[sizeof(ChunkHeader)];
    ui8 _regionFileHeaderBuffer[sizeof(RegionFileHeader)];
    
    ui32 _maxCacheSize;
    ui32 _chunkOffset; ///< Offset into the chunk data
    uLongf _chunkBufferSize;
    std::map <nString, RegionFile*> _regionFileCache;
    std::deque <RegionFile*> _regionFileCacheQueue;

    nString m_saveDir;
    RegionFile* _regionFile;
    ChunkHeader _chunkHeader;
};