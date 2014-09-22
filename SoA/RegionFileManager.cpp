#include "stdafx.h"

#include <direct.h> //for mkdir windows
#include <dirent.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ZLIB/zlib.h>

#include "global.h"

#include "RegionFileManager.h"

#include "Chunk.h"
#include "Errors.h"
#include "GameManager.h"
#include "Planet.h"
#include "utils.h"

//Bytes to read at a time. 8192 is a good size
#define READ_SIZE 8192
#define WRITE_SIZE 8192

inline i32 fileTruncate(i32 fd, i64 size)
{
#if defined(_WIN32) || defined(_WIN64) 
    return _chsize(fd, size);
#else
#ifdef POSIX
    return ftruncate(fd, size);
#else
    // code for other OSes
#endif
#endif
}

inline i32 sectorsFromBytes(ui32 bytes) {
    return ceil(bytes / (float)SECTOR_SIZE);
}

//returns true on error
bool checkZlibError(string message, int zerror) {
    switch (zerror) {
    case Z_OK:
        return false;
    case Z_STREAM_END:
        pError("Zlib " + message + " error Z_STREAM_END");
        return true;
    case Z_NEED_DICT:
        pError("Zlib " + message + " error Z_NEED_DICT");
        return true;
    case Z_ERRNO:
        pError("Zlib " + message + " error Z_ERRNO");
        return true;
    case Z_STREAM_ERROR:
        pError("Zlib " + message + " error Z_STREAM_ERROR");
        return true;
    case Z_DATA_ERROR:
        pError("Zlib " + message + " error Z_DATA_ERROR");
        return true;
    case Z_MEM_ERROR:
        pError("Zlib " + message + " error Z_MEM_ERROR");
        return true;
    case Z_BUF_ERROR:
        pError("Zlib " + message + " error Z_BUF_ERROR");
        return true;
    case Z_VERSION_ERROR:
        pError("Zlib " + message + " error Z_VERSION_ERROR");
        return true;
    }
    return false;
}

RegionFileManager::RegionFileManager() :_regionFile(nullptr), _copySectorsBuffer(nullptr), _maxCacheSize(8) {
}

RegionFileManager::~RegionFileManager() {
    clear();
}

void RegionFileManager::clear() {
    for (int i = 0; i < _regionFileCacheQueue.size(); i++) {
        closeRegionFile(_regionFileCacheQueue[i]);
    }

    if (_copySectorsBuffer) {
        delete[] _copySectorsBuffer;
        _copySectorsBuffer = nullptr;
    }

    _regionFileCache.clear();
    _regionFileCacheQueue.clear();
    _regionFile = nullptr;
}

bool RegionFileManager::openRegionFile(nString region, i32 face, bool create) {

    nString filePath;
    struct stat statbuf;
    RegionFile* rf;

    if (_regionFile && _regionFile->file && region == _regionFile->region) {
        return true;
    }

    flush();

    if (_regionFileCache.size() == _maxCacheSize) {
        //Remove the oldest region file from the cache
        rf = _regionFileCacheQueue.front();
        _regionFileCacheQueue.pop_front();
        _regionFileCache.erase(rf->region);
        closeRegionFile(rf);
    }
    
    //Check if it is cached
    auto rit = _regionFileCache.find(region);
    if (rit != _regionFileCache.end()) {
        for (auto it = _regionFileCacheQueue.begin(); it != _regionFileCacheQueue.end(); it++) {
            if ((*it)->region == region) {
                RegionFile* rf = (*it);
                _regionFileCacheQueue.erase(it);
                _regionFileCacheQueue.push_back(rf);
                break;
            }
        }
        _regionFile = rit->second;
        return true;
    } 


    filePath = GameManager::saveFilePath + "/Region/f" + std::to_string(face) + "/" + region + ".soar";
   
    //open file if it exists
    FILE* file = fopen(filePath.c_str(), "rb+");

    //If it doesn't exist
    if (file == nullptr){
        //Check if we should create a new region or return false
        if (create){
            file = fopen(filePath.c_str(), "wb+"); //create the file
            if (file == nullptr){
                perror(filePath.c_str());
                pError("Failed to create region file ");
                return false;
            }
        } else{
            return false;
        }
    }

    _regionFile = new RegionFile;
    memset(_regionFile, 0, sizeof(RegionFile));

    _regionFile->region = region;
    _regionFile->file = file;

    _regionFileCache[region] = _regionFile;
    _regionFileCacheQueue.push_back(_regionFile);

    _regionFile->fileDescriptor = fileno(_regionFile->file); //get file descriptor for truncate if needed

    if (fstat(_regionFile->fileDescriptor, &statbuf) != 0) {
        pError("Stat call failed for region file open"); //get the file stats
        return false;
    }
    
    _off_t fileSize = statbuf.st_size;

    //If the file is new, write an empty header
    if (fileSize == 0){ 
        //Save the empty header
        if (saveRegionHeader() == false) return false;

        _regionFile->totalSectors = 0;
        fflush(_regionFile->file);
    } else{ //load header data into the header struct 

        if (loadRegionHeader() == false) return false;

        if ((fileSize - sizeof(RegionFileHeader)) % SECTOR_SIZE){
            pError(filePath + ": Region file chunk storage must be multiple of " + std::to_string(SECTOR_SIZE) + ". Remainder = " + std::to_string(sectorsFromBytes(fileSize - sizeof(RegionFileHeader))));
            return false;
        }

        _regionFile->totalSectors = sectorsFromBytes(fileSize - sizeof(RegionFileHeader));
    }

    return true;
}

void RegionFileManager::closeRegionFile(RegionFile* regionFile) {

    if (regionFile->file == nullptr) return;

    if (_regionFile->isHeaderDirty) {
        saveRegionHeader();
    }

    fclose(regionFile->file);
    delete regionFile;
}

//Attempt to load a chunk. Returns false on failure
bool RegionFileManager::tryLoadChunk(Chunk* chunk) {

    nString regionString = getRegionString(chunk);

    //Open the region file
    if (!openRegionFile(regionString, chunk->faceData.face, false)) return false;

    //Get the chunk sector offset
    ui32 chunkSectorOffset = getChunkSectorOffset(chunk);
    //If chunkOffset is zero, it hasnt been saved
    if (chunkSectorOffset == 0) {
        return false;
    }

    //Location is not stored zero indexed, so that 0 indicates that it hasnt been saved
    chunkSectorOffset -= 1;

    //Seek to the chunk header
    if (!seekToChunk(chunkSectorOffset)){
        pError("Region: Chunk data fseek C error! " + to_string(sizeof(RegionFileHeader)+chunkSectorOffset * SECTOR_SIZE) + " size: " + to_string(_regionFile->totalSectors));
        return false;
    }

    //Get the chunk header
    if (!readChunkHeader()) return false;

    if (!readVoxelData_v0()) return false;
   
    //Fill the chunk with the aquired data
    if (!fillChunkVoxelData(chunk)) return false;

    return true;
}

//Saves a chunk to a region file
bool RegionFileManager::saveChunk(Chunk* chunk) {
    //Used for copying sectors if we need to resize the file
    if (_copySectorsBuffer) {
        delete[] _copySectorsBuffer;
        _copySectorsBuffer = nullptr;
    }

    nString regionString = getRegionString(chunk);

    if (!openRegionFile(regionString, chunk->faceData.face, true)) return false;

    ui32 tableOffset;
    ui32 chunkSectorOffset = getChunkSectorOffset(chunk, &tableOffset);

    ui32 oldVoxelDataSize = 0;
    ui32 oldAuxDataSize = 0;

    ui32 padLength;

    i32 numOldSectors;

    //If chunkOffset is zero, then we need to add the entry
    if (chunkSectorOffset == 0) {

        //Set the sector offset in the table
        BufferUtils::setInt(_regionFile->header.lookupTable, tableOffset, _regionFile->totalSectors + 1); //we add 1 so that 0 can indicate not saved
        _regionFile->isHeaderDirty = true;

        chunkSectorOffset = _regionFile->totalSectors;

        numOldSectors = 0;
      
    } else {
        //Convert sector offset from 1 indexed to 0 indexed
        chunkSectorOffset--;
        //seek to the chunk
        if (!seekToChunk(chunkSectorOffset)){
            pError("Region: Chunk data fseek save error BB! " + to_string(chunkSectorOffset));
            return false;
        }

        //Get the chunk header
        if (!readChunkHeader()) return false;
        oldVoxelDataSize = BufferUtils::extractInt(_chunkHeader.voxelDataSize);
        oldAuxDataSize = BufferUtils::extractInt(_chunkHeader.auxDataSize);
        numOldSectors = sectorsFromBytes(oldVoxelDataSize + oldAuxDataSize + sizeof(ChunkHeader));

        if (numOldSectors > _regionFile->totalSectors) {
            cout << (to_string(chunkSectorOffset) + " " + to_string(tableOffset) + "Chunk Header Corrupted\n");
            return false;
        }
    }

    //Compress the chunk data
    rleCompressChunk(chunk);
    zlibCompress();

    i32 numSectors = sectorsFromBytes(_compressedBufferSize);
    i32 sectorDiff = numSectors - numOldSectors;

    //If we need to resize the number of sectors in the file and this chunk is not at the end of file,
    //then we should copy all sectors at the end of the file so we can resize it. This operation should be
    //fairly rare.
    if ((sectorDiff != 0) && ((chunkSectorOffset + numOldSectors) != _regionFile->totalSectors)) {
        if (!seekToChunk(chunkSectorOffset + numOldSectors)){
            pError("Region: Failed to seek for sectorCopy " + to_string(chunkSectorOffset) + " " + to_string(numOldSectors) + " " + to_string(_regionFile->totalSectors));
            return false;
        }

        _copySectorsBufferSize = (_regionFile->totalSectors - (chunkSectorOffset + numOldSectors)) * SECTOR_SIZE;
        _copySectorsBuffer = new ui8[_copySectorsBufferSize]; //for storing all the data that will need to be copied in the end
       
        readSectors(_copySectorsBuffer, _copySectorsBufferSize);
    }

    //Set the header data
    BufferUtils::setInt(_chunkHeader.compression, COMPRESSION_RLE | COMPRESSION_ZLIB);
    BufferUtils::setInt(_chunkHeader.voxelDataSize, _compressedBufferSize - sizeof(ChunkHeader));
    BufferUtils::setInt(_chunkHeader.auxDataSize, 0);
    BufferUtils::setInt(_chunkHeader.timeStamp, 0);

    //Copy the header data to the write buffer
    memcpy(_compressedByteBuffer, &_chunkHeader, sizeof(ChunkHeader));

    //seek to the chunk
    if (!seekToChunk(chunkSectorOffset)){
        pError("Region: Chunk data fseek save error GG! " + to_string(chunkSectorOffset));
        return false;
    }

    //Write the header and data
    writeSectors(_compressedByteBuffer, (ui32)_compressedBufferSize);

    //Keep track of total sectors in file so we can infer filesize
    _regionFile->totalSectors += sectorDiff;

    //If we need to move some sectors around
    if (_copySectorsBuffer) {
   
        if (!seekToChunk(chunkSectorOffset + numSectors)){
            pError("Region: Chunk data fseek save error GG! " + to_string(chunkSectorOffset));
            return false;
        }
        //Write the buffer of sectors
        writeSectors(_copySectorsBuffer, _copySectorsBufferSize);
        delete[] _copySectorsBuffer;
        _copySectorsBuffer = nullptr;

        //if the file got smaller
        if (sectorDiff < 0){
            //truncate the file
            if (fileTruncate(_regionFile->fileDescriptor, sizeof(RegionFileHeader)+_regionFile->totalSectors * SECTOR_SIZE) != 0) {
                perror("Region file: Truncate error!\n");
            }
        }

        //Update the table
        ui32 nextChunkSectorOffset;
        for (int i = 0; i < REGION_SIZE * 4; i += 4){
            nextChunkSectorOffset = BufferUtils::extractInt(_regionFile->header.lookupTable, i);
            //See if the 1 indexed nextChunkSectorOffset is > the 0 indexed chunkSectorOffset
            if (nextChunkSectorOffset > (chunkSectorOffset + 1)){ 
                BufferUtils::setInt(_regionFile->header.lookupTable, i, nextChunkSectorOffset + sectorDiff);
            } 
        }
        _regionFile->isHeaderDirty = true;
    }
    fflush(_regionFile->file);
    return true;
}

void RegionFileManager::flush() {
    if (_regionFile && _regionFile->file) {
        if (_regionFile->isHeaderDirty) {
            saveRegionHeader();
            fflush(_regionFile->file);
        }  
    }
}

bool RegionFileManager::saveVersionFile() {
    FILE* file;
    file = fopen((GameManager::saveFilePath + "/Region/version.dat").c_str(), "wb");

    if (!file) return false;

    SaveVersion currentVersion;
    BufferUtils::setInt(currentVersion.regionVersion, CURRENT_REGION_VER);
    BufferUtils::setInt(currentVersion.chunkVersion, CURRENT_CHUNK_VER);

    fwrite(&currentVersion, 1, sizeof(SaveVersion), file);
    fclose(file);
    return true;
}

bool RegionFileManager::checkVersion() {
    FILE* file;
    file = fopen((GameManager::saveFilePath + "/Region/version.dat").c_str(), "rb");

    if (!file) {
        pError(GameManager::saveFilePath + "/Region/version.dat not found. Game will assume the version is correct, but it is "
               + "probable that this save will not work if the version is wrong. If this is a save from 0.1.6 or earlier, then it is "
               + "only compatable with version 0.1.6 of the game. In that case, please make a new save or download 0.1.6 to play this save.");
        return saveVersionFile();
    }
    SaveVersion version;

    fread(&version, 1, sizeof(SaveVersion), file);

    ui32 regionVersion = BufferUtils::extractInt(version.regionVersion);
    ui32 chunkVersion = BufferUtils::extractInt(version.chunkVersion);

    if (chunkVersion != CURRENT_CHUNK_VER || regionVersion != CURRENT_REGION_VER) {
        return tryConvertSave(regionVersion, chunkVersion);
    }
    return true;
}

bool RegionFileManager::readChunkHeader() {
    if (fread(&(_chunkHeader), 1, sizeof(ChunkHeader), _regionFile->file) != sizeof(ChunkHeader)) {
        return false;
    }
    return true;
}

bool RegionFileManager::readVoxelData_v0() {

    ui32 voxelDataSize = BufferUtils::extractInt(_chunkHeader.voxelDataSize);

    if (voxelDataSize > sizeof(_compressedByteBuffer)) {
        pError("Region voxel input buffer overflow");
        return false;
    }

    int readSize = READ_SIZE;
    for (int i = 0; i < voxelDataSize; i += READ_SIZE){
        //If the number of bytes we have left to read is less than the readSize, only read what we need
        if (readSize > voxelDataSize - i){
            readSize = voxelDataSize - i;
        }
        if (fread(&(_compressedByteBuffer[i]), 1, readSize, _regionFile->file) != readSize){
            cout << "Did not read enough bytes at Z\n";
            return false;
        }
    }

    uLongf bufferSize = CHUNK_DATA_SIZE + CHUNK_SIZE * 2;
    int zresult = uncompress(_byteBuffer, &bufferSize, _compressedByteBuffer, voxelDataSize);

    return (!checkZlibError("decompression", zresult));
 
}

void RegionFileManager::getIterationConstantsFromRotation(int rotation, int& jStart, int& jMult, int& jEnd, int& jInc, int& kStart, int& kMult, int& kEnd, int& kInc) {
    switch (rotation){ //we use rotation value to un-rotate the chunk data
        case 0: //no rotation
            jStart = 0;
            kStart = 0;
            jEnd = kEnd = CHUNK_WIDTH;
            jInc = kInc = 1;
            jMult = CHUNK_WIDTH;
            kMult = 1;
            break;
        case 1: //up is right
            jMult = 1;
            jStart = CHUNK_WIDTH - 1;
            jEnd = -1;
            jInc = -1;
            kStart = 0;
            kEnd = CHUNK_WIDTH;
            kInc = 1;
            kMult = CHUNK_WIDTH;
            break;
        case 2: //up is down
            jMult = CHUNK_WIDTH;
            jStart = CHUNK_WIDTH - 1;
            kStart = CHUNK_WIDTH - 1;
            jEnd = kEnd = -1;
            jInc = kInc = -1;
            kMult = 1;
            break;
        case 3: //up is left
            jMult = 1;
            jStart = 0;
            jEnd = CHUNK_WIDTH;
            jInc = 1;
            kMult = CHUNK_WIDTH;
            kStart = CHUNK_WIDTH - 1;
            kEnd = -1;
            kInc = -1;
            break;
        default:
            pError("ERROR Chunk Loading: Rotation value not 0-3");
            break;
    }
}

int RegionFileManager::rleUncompressArray(ui8* data, ui32& byteIndex, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc) {

    ui8 value;
    ui16 runSize;
    int index;

    int i = 0; //y
    int j = jStart; //z
    int k = kStart; //x
    int blockCounter = 0;

    //Read block data
    while (blockCounter < CHUNK_SIZE){
        //Grab a run of RLE data
        runSize = BufferUtils::extractShort(_byteBuffer, byteIndex);
        value = _byteBuffer[byteIndex + 2];

        for (int q = 0; q < runSize; q++){
            index = i * CHUNK_LAYER + j * jMult + k * kMult;

            if (index >= CHUNK_SIZE){
                pError("Chunk File Corrupted! Index >= 32768. (" + to_string(index) + ") " + to_string(q) + " " + to_string(runSize) + " " + to_string(blockCounter) );
                return 1;
            }

            data[index] = value;

            blockCounter++;
            k += kInc;
            if (k == kEnd){
                k = kStart;
                j += jInc;
                if (j == jEnd){
                    j = jStart;
                    i++;
                }
            }
        }
        byteIndex += 3;
    }
    return 0;
}

int RegionFileManager::rleUncompressArray(ui16* data, ui32& byteIndex, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc) {
    
    ui16 value;
    ui16 runSize;
    int index;

    int i = 0; //y
    int j = jStart; //z
    int k = kStart; //x
    int blockCounter = 0;

    //Read block data
    while (blockCounter < CHUNK_SIZE){
        //Grab a run of RLE data
        runSize = BufferUtils::extractShort(_byteBuffer, byteIndex);
        value = BufferUtils::extractShort(_byteBuffer, byteIndex + 2);

        for (int q = 0; q < runSize; q++){
            index = i * CHUNK_LAYER + j * jMult + k * kMult;

            if (index >= CHUNK_SIZE){
                pError("Chunk File Corrupted! Index >= 32768. (" + to_string(index) + ") " + to_string(q) + " " + to_string(runSize) + " " + to_string(blockCounter));
                return 1;
            }

            data[index] = value;

            blockCounter++;
            k += kInc;
            if (k == kEnd){
                k = kStart;
                j += jInc;
                if (j == jEnd){
                    j = jStart;
                    i++;
                }
            }
        }
        byteIndex += 4;
    }
    return 0;
}

bool RegionFileManager::fillChunkVoxelData(Chunk* chunk) {
   // return false;
    ui32 byteIndex = 0;

    ui8 lightVal;

    int blockIndex;
    int jStart, jEnd, jInc;
    int kStart, kEnd, kInc;
    int jMult, kMult;

    getIterationConstantsFromRotation(chunk->faceData.rotation, jStart, jMult, jEnd, jInc, kStart, kMult, kEnd, kInc);

    chunk->numBlocks = 0;

    if (rleUncompressArray(blockIDBuffer, byteIndex, jStart, jMult, jEnd, jInc, kStart, kMult, kEnd, kInc)) return false;

    if (rleUncompressArray(lampLightBuffer, byteIndex, jStart, jMult, jEnd, jInc, kStart, kMult, kEnd, kInc)) return false;

    if (rleUncompressArray(sunlightBuffer, byteIndex, jStart, jMult, jEnd, jInc, kStart, kMult, kEnd, kInc)) return false;

    //Node buffers, reserving maximum memory so we don't ever need to reallocate. Static so that the memory persists.
    static vector<VoxelIntervalTree<ui16>::LightweightNode> blockIDNodes(CHUNK_SIZE, VoxelIntervalTree<ui16>::LightweightNode(0, 0, 0));
    static vector<VoxelIntervalTree<ui16>::LightweightNode> lampLightNodes(CHUNK_SIZE, VoxelIntervalTree<ui16>::LightweightNode(0, 0, 0));
    static vector<VoxelIntervalTree<ui8>::LightweightNode> sunlightNodes(CHUNK_SIZE, VoxelIntervalTree<ui8>::LightweightNode(0, 0, 0));
    //Make the size 0
    blockIDNodes.clear();
    lampLightNodes.clear();
    sunlightNodes.clear();
     //   chunk->_blockIDContainer.initFromSortedArray()

    ui16 blockID;
    ui16 lampLight;
    ui8 sunlight;

    //Add first nodes
    blockIDNodes.push_back(VoxelIntervalTree<ui16>::LightweightNode(0, 1, blockIDBuffer[0]));
    lampLightNodes.push_back(VoxelIntervalTree<ui16>::LightweightNode(0, 1, lampLightBuffer[0]));
    sunlightNodes.push_back(VoxelIntervalTree<ui8>::LightweightNode(0, 1, sunlightBuffer[0]));

    //Construct the node vectors
    for (int i = 1; i < CHUNK_SIZE; i++) {
        blockID = blockIDBuffer[i];
        lampLight = lampLightBuffer[i];
        sunlight = sunlightBuffer[i];

        if (blockID != 0) chunk->numBlocks++;
        
        if (GETBLOCK(blockID).spawnerVal || GETBLOCK(blockID).sinkVal){
            chunk->spawnerBlocks.push_back(i);
        }

        if (blockID == blockIDNodes.back().data) {
            blockIDNodes.back().length++;
        } else {
            blockIDNodes.push_back(VoxelIntervalTree<ui16>::LightweightNode(i, 1, blockID));
        }
        if (lampLight == lampLightNodes.back().data) {
            lampLightNodes.back().length++;
        } else {
            lampLightNodes.push_back(VoxelIntervalTree<ui16>::LightweightNode(i, 1, lampLight));
        }
        if (sunlight == sunlightNodes.back().data) {
            sunlightNodes.back().length++;
        } else {
            sunlightNodes.push_back(VoxelIntervalTree<ui8>::LightweightNode(i, 1, sunlight));
        }
    }
  
    chunk->_blockIDContainer.initFromSortedArray(blockIDNodes);
    chunk->_lampLightContainer.initFromSortedArray(lampLightNodes);
    chunk->_sunlightContainer.initFromSortedArray(sunlightNodes);

    return true;
}

//Saves the header for the region file
bool RegionFileManager::saveRegionHeader() {
    //Go back to beginning of file to save the header
    if (!seek(0)) {
        pError("Fseek error: could not seek to start. Save file is corrupted!\n");
        return false;
    }
    //Save the header
    if (fwrite(&(_regionFile->header), 1, sizeof(RegionFileHeader), _regionFile->file) != sizeof(RegionFileHeader)){
        pError("Region write error: could not write loc buffer. Save file is corrupted!\n");
        return false;
    }

    _regionFile->isHeaderDirty = false;

    return true;
}

//Loads the header for the region file and stores it in the region file struct
bool RegionFileManager::loadRegionHeader() {

    if (!seek(0)){
        pError("Region fseek error F could not seek to start\n");
        return false;
    }
    if (fread(&(_regionFile->header), 1, sizeof(RegionFileHeader), _regionFile->file) != sizeof(RegionFileHeader)){ //read the whole buffer in
        pError("Region read error: could not read region header\n");
        return false;
    }

    return true;
}

void RegionFileManager::rleCompressArray(ui8* data, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc) {
    int count = 1;
    int tot = 0;
    ui16 curr = data[jStart*jMult + kStart*kMult];
    int index;
    int z = 0;
    bool first = true;
    for (int i = 0; i < CHUNK_WIDTH; i++){ //y
        for (int j = jStart; j != jEnd; j += jInc){ //z
            for (int k = kStart; k != kEnd; k += kInc){ //x 
                z++;
                if (!first) {
                    index = i*CHUNK_LAYER + j*jMult + k*kMult;
                    if (data[index] != curr){
                        _byteBuffer[_bufferSize++] = (ui8)((count & 0xFF00) >> 8);
                        _byteBuffer[_bufferSize++] = (ui8)(count & 0xFF);
                        _byteBuffer[_bufferSize++] = curr;
                        tot += count;

                        curr = data[index];
                        count = 1;
                    } else{
                        count++;
                    }
                } else {
                    first = false;
                }
            }
        }
    }
    _byteBuffer[_bufferSize++] = (ui8)((count & 0xFF00) >> 8);
    _byteBuffer[_bufferSize++] = (ui8)(count & 0xFF);
    tot += count;
    _byteBuffer[_bufferSize++] = curr;
}

void RegionFileManager::rleCompressArray(ui16* data, int jStart, int jMult, int jEnd, int jInc, int kStart, int kMult, int kEnd, int kInc) {
    int count = 1;
    ui16 curr = data[jStart*jMult + kStart*kMult];
    int index;
    int tot = 0;
    bool first = true;
    for (int i = 0; i < CHUNK_WIDTH; i++){ //y
        for (int j = jStart; j != jEnd; j += jInc){ //z
            for (int k = kStart; k != kEnd; k += kInc){ //x 
                if (!first){
                    index = i*CHUNK_LAYER + j*jMult + k*kMult;
                    if (data[index] != curr){
                        _byteBuffer[_bufferSize++] = (ui8)((count & 0xFF00) >> 8);
                        _byteBuffer[_bufferSize++] = (ui8)(count & 0xFF);
                        _byteBuffer[_bufferSize++] = (ui8)((curr & 0xFF00) >> 8);
                        _byteBuffer[_bufferSize++] = (ui8)(curr & 0xFF);
                        tot += count;
                        curr = data[index];
                        count = 1;
                    } else{
                        count++;
                    }
                } else{
                    first = false;
                }
            }
        }
    }
    _byteBuffer[_bufferSize++] = (ui8)((count & 0xFF00) >> 8);
    _byteBuffer[_bufferSize++] = (ui8)(count & 0xFF);
    _byteBuffer[_bufferSize++] = (ui8)((curr & 0xFF00) >> 8);
    _byteBuffer[_bufferSize++] = (ui8)(curr & 0xFF);
    tot += count;
}

bool RegionFileManager::rleCompressChunk(Chunk* chunk) {
    ui16* blockIDData;
    ui8* sunlightData;
    ui16* lampLightData;
    if (chunk->_blockIDContainer.getState() == VoxelStorageState::INTERVAL_TREE) {
        blockIDData = blockIDBuffer;
        chunk->_blockIDContainer.uncompressIntoBuffer(blockIDData);
    } else {
        blockIDData = chunk->_blockIDContainer.getDataArray();
    }
    if (chunk->_lampLightContainer.getState() == VoxelStorageState::INTERVAL_TREE) {
        lampLightData = lampLightBuffer;
        chunk->_lampLightContainer.uncompressIntoBuffer(lampLightData);
    } else {
        lampLightData = chunk->_lampLightContainer.getDataArray();
    }
    if (chunk->_sunlightContainer.getState() == VoxelStorageState::INTERVAL_TREE) {
        sunlightData = sunlightBuffer;
        chunk->_sunlightContainer.uncompressIntoBuffer(sunlightData);
    } else {
        sunlightData = chunk->_sunlightContainer.getDataArray();
    }

    _bufferSize = 0;

    int jStart, jEnd, jInc;
    int kStart, kEnd, kInc;
    int jMult, kMult;

    getIterationConstantsFromRotation(chunk->faceData.rotation, jStart, jMult, jEnd, jInc, kStart, kMult, kEnd, kInc);

    rleCompressArray(blockIDData, jStart, jMult, jEnd, jInc, kStart, kMult, kEnd, kInc);
    rleCompressArray(lampLightData, jStart, jMult, jEnd, jInc, kStart, kMult, kEnd, kInc);
    rleCompressArray(sunlightData, jStart, jMult, jEnd, jInc, kStart, kMult, kEnd, kInc);

    return true;
}

bool RegionFileManager::zlibCompress() {
    _compressedBufferSize = CHUNK_DATA_SIZE + CHUNK_SIZE * 2;
    //Compress the data, and leave space for the uncompressed chunk header
    int zresult = compress2(_compressedByteBuffer + sizeof(ChunkHeader), &_compressedBufferSize, _byteBuffer, _bufferSize, 6);
    _compressedBufferSize += sizeof(ChunkHeader);

    return (!checkZlibError("compression", zresult));
}

//TODO: Implement this
bool RegionFileManager::tryConvertSave(ui32 regionVersion, ui32 chunkVersion) {
    pError("Invalid region file version!");
    return false;
}

//Writes sector data, be sure to fseek to the correct position first
bool RegionFileManager::writeSectors(ui8* srcBuffer, ui32 size) {
    ui32 writeSize = WRITE_SIZE;
    ui32 padLength = 0;
    for (ui32 i = 0; i < size; i += WRITE_SIZE){
        if (writeSize > size - i){
            padLength = SECTOR_SIZE - (size - i) % SECTOR_SIZE;
            if (padLength == SECTOR_SIZE) padLength = 0;
            writeSize = size - i + padLength;
        }

        if (fwrite(&(srcBuffer[i]), 1, writeSize, _regionFile->file) != writeSize){
            pError("Chunk Saving: Did not write enough bytes at A " + to_string(writeSize) + " " + to_string(size));
            return false;
        }
    }
}

//Read sector data, be sure to fseek to the correct position first
bool RegionFileManager::readSectors(ui8* dstBuffer, ui32 size) {
    ui32 readSize = READ_SIZE;

    for (ui32 i = 0; i < size; i += READ_SIZE){
        if (readSize > size - i){    
            readSize = size - i;
        }
        if (fread(dstBuffer + i, 1, readSize, _regionFile->file) != readSize){
            pError("Chunk Loading: Did not read enough bytes at A " + to_string(readSize) + " " + to_string(_regionFile->totalSectors));
            return false;
        }
    }
}

bool RegionFileManager::seek(ui32 byteOffset) {
    return (fseek(_regionFile->file, byteOffset, SEEK_SET) == 0);
}

bool RegionFileManager::seekToChunk(ui32 chunkSectorOffset) {
    //seek to the chunk
    return seek(sizeof(RegionFileHeader) + chunkSectorOffset * SECTOR_SIZE);
}

ui32 RegionFileManager::getChunkSectorOffset(Chunk* chunk, ui32* retTableOffset) {
    int idir = FaceOffsets[chunk->faceData.face][chunk->faceData.rotation][0];
    int jdir = FaceOffsets[chunk->faceData.face][chunk->faceData.rotation][1];
    int ip = (chunk->faceData.ipos - GameManager::planet->radius / CHUNK_WIDTH)*idir;
    int jp = (chunk->faceData.jpos - GameManager::planet->radius / CHUNK_WIDTH)*jdir;

    if (chunk->faceData.rotation % 2){ //when rot%2 i and j must switch
        int tmp = ip;
        ip = jp;
        jp = tmp;
    }

    int ym = ((int)floor(chunk->position.y / (float)CHUNK_WIDTH) % REGION_WIDTH); 
    int im = ip % REGION_WIDTH;
    int jm = jp % REGION_WIDTH;

    //modulus is weird in c++ for negative numbers
    if (ym < 0) ym += REGION_WIDTH;
    if (im < 0) im += REGION_WIDTH;
    if (jm < 0) jm += REGION_WIDTH;
    ui32 tableOffset = 4 * (jm + im * REGION_WIDTH + ym * REGION_LAYER);

    //If the caller asked for the table offset, return it
    if (retTableOffset) *retTableOffset = tableOffset;

    return BufferUtils::extractInt(_regionFile->header.lookupTable, tableOffset);
}

nString RegionFileManager::getRegionString(Chunk *ch)
{
    int rot = ch->faceData.rotation;
    int face = ch->faceData.face;
    int idir = FaceOffsets[face][rot][0];
    int jdir = FaceOffsets[face][rot][1];
    int ip = (ch->faceData.ipos - GameManager::planet->radius / CHUNK_WIDTH)*idir;
    int jp = (ch->faceData.jpos - GameManager::planet->radius / CHUNK_WIDTH)*jdir;

    if (rot % 2){ //when rot%2 i and j must switch
        return "r." + to_string(ip >> RSHIFT) + "." + to_string((int)floor(ch->position.y / CHUNK_WIDTH) >> RSHIFT) + "." + to_string(jp >> RSHIFT);
    } else{
        return "r." + to_string(jp >> RSHIFT) + "." + to_string((int)floor(ch->position.y / CHUNK_WIDTH) >> RSHIFT) + "." + to_string(ip >> RSHIFT);
    }
}