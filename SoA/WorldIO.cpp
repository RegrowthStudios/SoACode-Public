#include "stdafx.h"
#include "WorldIO.h"

#include <direct.h> //for mkdir windows
#include <dirent.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ZLIB/zlib.h>

#include "BlockData.h"
#include "Chunk.h"
#include "Errors.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "Options.h"
#include "Planet.h"
#include "Player.h"

//#define EXTRACTINT(a, i) ( (((GLuint)((GLubyte *)(a))[(i)]) << 24 ) | (((GLuint)((GLubyte *)(a))[(i)+1]) << 16) | (((GLuint)((GLubyte *)(a))[(i)+2]) << 8) | ((GLuint)((GLubyte *)(a))[(i)+3]) )

void ThreadError(string msg){
    cout << msg << endl;
    pError(msg.c_str());
}

WorldIO::WorldIO()
{
    _maxLocationBufferCacheSize = 15;
    _currReg = "";
    _isThreadFinished = 0;
    _isDirtyLocationBuffer = 0;
    _currLocationBuffer = NULL;
    readWriteThread = NULL;
    _threadFile = NULL;
    _shouldDisableLoading = 0;
}

WorldIO::~WorldIO()
{
    onQuit();
}

void WorldIO::compressBlockData(Chunk *ch)
{
    GLushort *blockData = ch->data;
    GLubyte *lightData = ch->lightData[0];

    _bufferSize = 0;
    
    GLushort curr;
    GLubyte currb;
    GLuint count = 1;
    int c;
    int jStart, jEnd, jInc;
    int kStart, kEnd, kInc;
    int jMult, kMult;

    switch(ch->faceData.rotation){ //we use rotation value to un-rotate the chunk data
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
            jStart = CHUNK_WIDTH-1;
            jEnd = -1;
            jInc = -1;
            kStart = 0;
            kEnd = CHUNK_WIDTH;
            kInc = 1;
            kMult = CHUNK_WIDTH;
            break;
        case 2: //up is down
            jMult = CHUNK_WIDTH;
            jStart = CHUNK_WIDTH-1;
            kStart = CHUNK_WIDTH-1;
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
            kStart = CHUNK_WIDTH-1;
            kEnd = -1;
            kInc = -1;
            break;
    }

    curr = blockData[jStart*jMult + kStart*kMult];

    bool first = 1;
    //compress and store block ID data
    for (int i = 0; i < CHUNK_WIDTH; i++){ //y
        for (int j = jStart; j != jEnd; j+=jInc){ //z
            for (int k = kStart; k != kEnd; k+=kInc){ //x 
                if (!first){ //have to ignore the first one since we set it above
                    c = i*CHUNK_LAYER + j*jMult + k*kMult; //sometimes x is treated like z and visa versa when rotating
                    if (blockData[c] != curr){
                        _byteBuffer[_bufferSize++] = (GLubyte)((count & 0xFF00) >> 8);
                        _byteBuffer[_bufferSize++] = (GLubyte)(count & 0xFF);
                        _byteBuffer[_bufferSize++] = (GLubyte)((curr & 0xFF00) >> 8);
                        _byteBuffer[_bufferSize++] = (GLubyte)(curr & 0xFF);

                        curr = blockData[c];
                        count = 1;
                    }else{
                        count++;
                    }
                }else{
                    first = 0;
                }
            }
        }
    }
    _byteBuffer[_bufferSize++] = (GLubyte)((count & 0xFF00) >> 8);
    _byteBuffer[_bufferSize++] = (GLubyte)(count & 0xFF);
    _byteBuffer[_bufferSize++] = (GLubyte)((curr & 0xFF00) >> 8);
    _byteBuffer[_bufferSize++] = (GLubyte)(curr & 0xFF);

    //compress and store artificial light
    count = 1;
    currb = lightData[jStart*jMult + kStart*kMult];
    first = 1;
    for (int i = 0; i < CHUNK_WIDTH; i++){ //y
        for (int j = jStart; j != jEnd; j+=jInc){ //z
            for (int k = kStart; k != kEnd; k+=kInc){ //x 
                if (!first){ //have to ignore the first one since we set it above
                    c = i*CHUNK_LAYER + j*jMult + k*kMult;
                    if (lightData[c] != currb){ //remove the count ==???
                        _byteBuffer[_bufferSize++] = (GLubyte)((count & 0xFF00)>>8);
                        _byteBuffer[_bufferSize++] = (GLubyte)(count & 0xFF);
                        _byteBuffer[_bufferSize++] = currb;

                        currb = lightData[c];
                        count = 1;
                    }else{
                        count++;
                    }
                }else{
                    first = 0;
                }
            }
        }
    }
    _byteBuffer[_bufferSize++] = (GLubyte)((count & 0xFF00)>>8);
    _byteBuffer[_bufferSize++] = (GLubyte)(count & 0xFF);
    _byteBuffer[_bufferSize++] = currb;

    //compress and store voxel sunlight
    count = 1;
    currb = lightData[CHUNK_SIZE + jStart*jMult + kStart*kMult];
    first = 1;
    for (int i = 0; i < CHUNK_WIDTH; i++){ //y
        for (int j = jStart; j != jEnd; j+=jInc){ //z
            for (int k = kStart; k != kEnd; k+=kInc){ //x 
                if (!first){ //have to ignore the first one since we set it above
                    c = i*CHUNK_LAYER + j*jMult + k*kMult;
                    if (lightData[CHUNK_SIZE + c] != currb){ //remove the count ==???
                        _byteBuffer[_bufferSize++] = (GLubyte)((count & 0xFF00)>>8);
                        _byteBuffer[_bufferSize++] = (GLubyte)(count & 0xFF);
                        _byteBuffer[_bufferSize++] = currb;

                        currb = lightData[CHUNK_SIZE + c];
                        count = 1;
                    }else{
                        count++;
                    }
                }else{
                    first = 0;
                }
            }
        }
    }
    _byteBuffer[_bufferSize++] = (GLubyte)((count & 0xFF00)>>8);
    _byteBuffer[_bufferSize++] = (GLubyte)(count & 0xFF);
    _byteBuffer[_bufferSize++] = currb;

    for (int j = jStart; j != jEnd; j+=jInc){ //z
        for (int k = kStart; k != kEnd; k+=kInc){ //x 
            c = j*jMult + k*kMult;
            _byteBuffer[_bufferSize++] = ((GLubyte *)ch->sunLight)[c];
        }
    }

    if (_bufferSize >= 524288){
        cout << "ITS THE BUFFER SIZE DAMMIT ";
        cout << _bufferSize << endl;
        int a;
        cin >> a;
    }
    
    _compressedSize = CRW_BYTE_BUFSIZE + 16 + 52430;
    int zresult = compress2(&(_compressedByteBuffer[4]), &_compressedSize, _byteBuffer, _bufferSize, 6);
    
    //set the size bytes
    _compressedByteBuffer[0] = (GLubyte)((_compressedSize & 0xFF000000) >> 24);
    _compressedByteBuffer[1] = (GLubyte)((_compressedSize & 0x00FF0000) >> 16);
    _compressedByteBuffer[2] = (GLubyte)((_compressedSize & 0x0000FF00) >> 8);
    _compressedByteBuffer[3] = (GLubyte)(_compressedSize & 0x000000FF);
    _compressedSize += 4; //add size of size bytes

    switch( zresult )
    {
    case Z_MEM_ERROR:
        ThreadError("zlib compression: out of memory\n");
        exit(1);    // quit.
    case Z_BUF_ERROR:
        ThreadError("zlib compression: output buffer wasn't large enough\n");
        exit(1);    // quit.
    }
}

int WorldIO::saveToFile(Chunk *ch)
{
    GLuint offset;
    GLuint bufSize = 0;
    GLubyte *endDataBuffer = NULL;
    int sizeDiff = 0;
    GLuint padLength;
    GLuint endDataBufferSize = 0;

    compressBlockData(ch);

    _chunkLength = _compressedSize; //size of our new chunk
    padLength = SECTOR_SIZE - _chunkLength%SECTOR_SIZE;
    if (padLength == SECTOR_SIZE) padLength = 0;
    _chunkLength += padLength; //pad the length

    //try to seek to location
    seekToChunkOffset(ch);

    GLuint num;
    GLuint newBlockSize = _chunkLength/SECTOR_SIZE;
    if (newBlockSize > _chunkBlockSize || newBlockSize < _chunkBlockSize) // if we need more or fewer blocks
    {
        sizeDiff = newBlockSize - _chunkBlockSize;
        bufSize = _fileSize/SECTOR_SIZE - (_chunkOffset + _chunkBlockSize); //number of blocks to copy
        if (bufSize != 0){ //check if there is stuff after us that will be displaced
            endDataBuffer = new GLubyte[bufSize*SECTOR_SIZE]; //for storing all the data that will need to be copied in the end
            endDataBufferSize = bufSize*SECTOR_SIZE;

            if (fseek(_threadFile, _fileSize - bufSize*SECTOR_SIZE, SEEK_SET) != 0){
                cout << "Region: Chunk data fseek C error! " << _fileSize << " " << bufSize << " " << _fileSize - bufSize*SECTOR_SIZE << endl;
            }

            num = 8192;
            for (GLuint i = 0; i < endDataBufferSize; i+= 8192){
                if (endDataBufferSize - i < num){
                    padLength = SECTOR_SIZE - (endDataBufferSize-i)%SECTOR_SIZE;
                    if (padLength == SECTOR_SIZE) padLength = 0;
                    num = endDataBufferSize-i + padLength;
                }
                if (fread(&(endDataBuffer[i]), 1, num, _threadFile) != num){
                    cout << "Did not read enough bytes at A\n";
                }
            }
        }
    }
    
    if (fseek(_threadFile, _chunkOffset*SECTOR_SIZE, SEEK_SET) != 0){
        cout << "Region: Chunk data fseek D error! " << _fileSize << " " << _chunkOffset << " " << _chunkOffset*SECTOR_SIZE << endl;
        int a;
        cin >> a;
    }
    
    //write data
    num = 8192;
    for (GLuint i = 0; i < _compressedSize; i += 8192){
        if (_compressedSize - i < num){
            padLength = SECTOR_SIZE - (_compressedSize-i)%SECTOR_SIZE;
            if (padLength == SECTOR_SIZE) padLength = 0;
            num = _compressedSize-i + padLength;
        }
        if (fwrite(&(_compressedByteBuffer[i]), 1, num, _threadFile) != num){
            cout << "Did not write enough bytes at A\n";
        }
    }

    //write leftover data
    num = 8192;
    for (GLuint i = 0; i < endDataBufferSize; i+= 8192){
        if (endDataBufferSize - i < num){
            padLength = SECTOR_SIZE - (endDataBufferSize-i)%SECTOR_SIZE;
            if (padLength == SECTOR_SIZE) padLength = 0;
            num = endDataBufferSize-i + padLength;
        }
        if (fwrite(&(endDataBuffer[i]), 1, num, _threadFile) != num){
            cout << "Did not write enough bytes at B\n";
        }
            
    }

    if (sizeDiff < 0){ //if the file got smaller
        if (truncate(_fileSize + sizeDiff*SECTOR_SIZE) != 0){ //truncate the size
            cout << "Region: Truncate error!\n";
            perror(" region file ");
        }
    }

    _fileSize = _fileSize + sizeDiff*SECTOR_SIZE;

    GLuint location;
    location = extractInt(_currLocationBuffer->buffer, _locOffset);
    offset = (location >> 8);
    if (offset != _chunkOffset){
        _currLocationBuffer->buffer[_locOffset] = (GLubyte)((_chunkOffset & 0xFF0000) >> 16);
        _currLocationBuffer->buffer[_locOffset + 1] = (GLubyte)((_chunkOffset & 0xFF00) >> 8);
        _currLocationBuffer->buffer[_locOffset + 2] = (GLubyte)(_chunkOffset & 0xFF);
        _isDirtyLocationBuffer = 1;
    }
    if (_chunkBlockSize != newBlockSize){
        _currLocationBuffer->buffer[_locOffset + 3] = (GLubyte)newBlockSize; //set the blockSize
        _isDirtyLocationBuffer = 1;
    }

    //update the table
    if (bufSize != 0){
        for (int i = 0; i < 16384; i+=4){
            location = extractInt(_currLocationBuffer->buffer, i);
            offset = (location >> 8);
            if (offset > _chunkOffset){ //if its an in use chunk
                offset += sizeDiff;
                _currLocationBuffer->buffer[i] = (GLubyte)((offset & 0xFF0000) >> 16);
                _currLocationBuffer->buffer[i + 1] = (GLubyte)((offset & 0xFF00) >> 8);
                _currLocationBuffer->buffer[i + 2] = (GLubyte)(offset & 0xFF);
                _isDirtyLocationBuffer = 1;
            }
        }
    }

    if (_isDirtyLocationBuffer){
        fseek(_threadFile, 0, SEEK_SET); //go back to beginning of file to save the table
        fwrite(_currLocationBuffer->buffer, 1, 16384, _threadFile);
        _isDirtyLocationBuffer = 0;
    }

    fflush(_threadFile);
    
    if (endDataBuffer){
        delete[] endDataBuffer; //no longer need the buffer
    }

    return 0;
}

int WorldIO::loadFromFile(Chunk *ch)
{
    _locOffset = getLocOffset(ch);

    GLuint location = extractInt(_currLocationBuffer->buffer, _locOffset);
    _chunkOffset = (location >> 8);//grab the 3 offset bytes
    _chunkBlockSize = (location & 0xFF);//grab the block size byte
    if (_chunkOffset == 0){
        //Error(("TRIED TO LOAD CHUNK WITH NO OFFSET " + to_string(locOffset) + " " + to_string(location) + " " + currReg).c_str());
        cout << "Nonfatal error: TRIED TO LOAD CHUNK WITH NO OFFSET " + to_string(_locOffset) + " " + to_string(location) + " " + _currReg << endl;
        return 1;
    }
    if (fseek(_threadFile, _chunkOffset*SECTOR_SIZE, SEEK_SET) != 0){
        cout << "Region: Chunk data fseek C error! " << _fileSize << " " << _chunkOffset << " " << _chunkOffset*SECTOR_SIZE << " " << _threadFile << endl;
        return 1;
    }

    int num = 8192;
    int padLength;
    int size = _chunkBlockSize*SECTOR_SIZE;
    if (size >= 262144)pError("Region input Byte Buffer overflow");
    for (int i = 0; i < size; i+=8192){
        if (size - i < num){
            padLength = SECTOR_SIZE - (size-i)%SECTOR_SIZE;
            if (padLength == SECTOR_SIZE) padLength = 0;
            num = size-i + padLength;
        }
        if (fread(&(_compressedByteBuffer[i]), 1, num, _threadFile) != num){
            cout << "Did not read enough bytes at Z\n";
            return 1;
        }
    }

    _bufferSize = CRW_BYTE_BUFSIZE;
    _compressedSize = extractInt(_compressedByteBuffer, 0); //grab the size int
    int zresult = uncompress(_byteBuffer, &_bufferSize, &(_compressedByteBuffer[4]), _compressedSize);

    switch( zresult )
    {
    case Z_MEM_ERROR:
        ThreadError("zlib uncompression: out of memory\n");
        exit(1);    // quit.
    case Z_BUF_ERROR:
        ThreadError("zlib uncompression: output buffer wasn't large enough\n");
        exit(1);    // quit.
    }

    unsigned long b = 0;
    int step = 0;//0 = data, 1 = lightdata

    //cout << "READSIZE " << size << endl;

    GLuint bindex = 0;
    GLushort blockID;
    GLushort runSize;
    GLubyte lightVal;

    int c;
    int jStart, jEnd, jInc;
    int kStart, kEnd, kInc;
    int jMult, kMult;

    switch(ch->faceData.rotation){ //we use rotation value to un-rotate the chunk data
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
            jStart = CHUNK_WIDTH-1;
            jEnd = -1;
            jInc = -1;
            kStart = 0;
            kEnd = CHUNK_WIDTH;
            kInc = 1;
            kMult = CHUNK_WIDTH;
            break;
        case 2: //up is down
            jMult = CHUNK_WIDTH;
            jStart = CHUNK_WIDTH-1;
            kStart = CHUNK_WIDTH-1;
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
            kStart = CHUNK_WIDTH-1;
            kEnd = -1;
            kInc = -1;
            break;
        default:
            cout << "ERROR Chunk Loading: Rotation value not 0-3";
            int a;
            cin >> a;
            return 1;
            break;
    }

    int i = 0; //y
    int j = jStart; //z
    int k = kStart; //x
    int sunLightAdd = 0;

    ch->num = 0;
    while (b < _bufferSize){
        //blockData
        if (step == 0){
            if (b >= 524280) { cout << "ERROR: Chunk File Corrupted! :( " << _bufferSize << " " << bindex << " " << _locOffset << endl; return 1; }
             runSize = (((GLushort)_byteBuffer[b]) << 8) | ((GLushort)_byteBuffer[b+1]);
            blockID = (((GLushort)_byteBuffer[b+2]) << 8) | ((GLushort)_byteBuffer[b+3]);
            if (blockID != 0) ch->num += runSize;

            for (int q = 0; q < runSize; q++){
                c = i*CHUNK_LAYER + j*jMult + k*kMult;
                if (c >= CHUNK_SIZE){ cout << "Chunk File Corrupted!\n"; return 1; }
                ch->data[c] = blockID;
                if (GETBLOCK(ch->data[c]).spawnerVal || GETBLOCK(ch->data[c]).sinkVal){
                    ch->activeBlocks.push_back(c);
                }
                if (blockID >= LOWWATER && blockID <= FULLWATER) ch->hasWater = 1;
                bindex++;
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
            if (bindex == CHUNK_SIZE){
                i = 0;
                j = jStart;
                k = kStart;
                step = 1;
                bindex = 0;
            }
            b += 4;
        }else if (step == 1){ //lightData
            runSize = (((GLushort)_byteBuffer[b]) << 8) | ((GLushort)_byteBuffer[b+1]);
            lightVal = _byteBuffer[b+2];
        
            for (int q = 0; q < runSize; q++){
                c = i*CHUNK_LAYER + j*jMult + k*kMult;
                if (sunLightAdd + c >= CHUNK_SIZE * 2){
                    cout << "Corruption when filling light data from loaded chunk.";
                    return 1;
                }
                ((GLubyte *)(ch->lightData))[sunLightAdd+c] = lightVal;
                bindex++;
                k += kInc;
                if (k == kEnd){
                    k = kStart;
                    j += jInc;
                    if (j == jEnd){
                        j = jStart;
                        i++;
                    }
                }
                if (bindex == CHUNK_SIZE){ //start over for sunLight
                    i = 0;
                    j = jStart;
                    k = kStart;
                    sunLightAdd = CHUNK_SIZE;
                }
            }
            if (bindex == CHUNK_SIZE*2){
                step = 2;
                bindex = 0;
            }
            b += 3;
        }else{ //sunlights data
            c = j*jMult + k*kMult;
            if (c >= 1024){ cout << "Chunk File Corrupted!\n"; return 1; }
            ch->sunLight[c] = ((GLbyte *)_byteBuffer)[b];
            bindex++;
            k += kInc;
            if (k == kEnd){
                k = kStart;
                j += jInc;
            }
            b++;
        //    if (bindex == 1024){
        //        break;
        //    }
        }
    }
    if (bindex != 1024){
        cout << "ERROR Chunk Loading: bindex went too far! " << bindex << " " << step << endl;
        return 1;
    }
    return 0;
}

int WorldIO::deleteChunkFile(Chunk *ch)
{
    _locOffset = getLocOffset(ch);
    GLubyte *endDataBuffer = NULL;
    GLuint endDataBufferSize = 0;
    GLuint num;
    int padLength;
    GLuint location = extractInt(_currLocationBuffer->buffer, _locOffset);
    _chunkOffset = (location >> 8);//grab the 3 offset bytes
    _chunkBlockSize = (location & 0xFF);//grab the block size byte

    //clear the spot in the lookup table
    setInt(_currLocationBuffer->buffer, _locOffset, 0);
    _isDirtyLocationBuffer = 1;

    int sizeDiff = -((int)_chunkBlockSize);
    int bufSize = _fileSize / SECTOR_SIZE - (_chunkOffset + _chunkBlockSize); //number of blocks to copy
    if (bufSize < 0) {
        cout << "Save file is corrupted! :( Attempting to recover... but your save may be ruined. I am sorry..." << endl;
        bufSize = 0;
    }
    if (bufSize != 0){ //check if there is stuff after us that will be displaced
        endDataBuffer = new GLubyte[bufSize*SECTOR_SIZE]; //for storing all the data that will need to be copied in the end
        endDataBufferSize = bufSize*SECTOR_SIZE;

        if (fseek(_threadFile, _fileSize - bufSize*SECTOR_SIZE, SEEK_SET) != 0){
            cout << "Region: Chunk data fseek C error! " << _fileSize << " " << bufSize << " " << _fileSize - bufSize*SECTOR_SIZE << endl;
            int a;
            cin >> a;
        }

        num = 8192;
        for (GLuint i = 0; i < endDataBufferSize; i += 8192){
            if (endDataBufferSize - i < num){
                padLength = SECTOR_SIZE - (endDataBufferSize - i) % SECTOR_SIZE;
                if (padLength == SECTOR_SIZE) padLength = 0;
                num = endDataBufferSize - i + padLength;
            }
            if (fread(&(endDataBuffer[i]), 1, num, _threadFile) != num){
                cout << "Did not read enough bytes at A delete\n";
            }
        }
    }
     //this is the same as save?
    if (fseek(_threadFile, _chunkOffset*SECTOR_SIZE, SEEK_SET) != 0){
        cout << "Region: Chunk data fseek D error! " << _fileSize << " " << _chunkOffset << " " << _chunkOffset*SECTOR_SIZE << endl;
        int a;
        cin >> a;
    }

    //write leftover data
    num = 8192;
    for (GLuint i = 0; i < endDataBufferSize; i += 8192){
        if (endDataBufferSize - i < num){
            padLength = SECTOR_SIZE - (endDataBufferSize - i) % SECTOR_SIZE;
            if (padLength == SECTOR_SIZE) padLength = 0;
            num = endDataBufferSize - i + padLength;
        }
        if (fwrite(&(endDataBuffer[i]), 1, num, _threadFile) != num){
            cout << "Did not write enough bytes at B delete\n";
        }

    }

    if (sizeDiff < 0){ //if the file got smaller
        if (truncate(_fileSize + sizeDiff*SECTOR_SIZE) != 0){ //truncate the size
            cout << "Region: Truncate error!\n";
            perror(" region file ");
            int a;
            cin >> a;
        }
    }

    if (_isDirtyLocationBuffer){
        fseek(_threadFile, 0, SEEK_SET); //go back to beginning of file to save the table
        fwrite(_currLocationBuffer->buffer, 1, 16384, _threadFile);
        _isDirtyLocationBuffer = 0;
    }


    fflush(_threadFile);
    return 0;
}

int WorldIO::tryReadFromFile()
{
    FILE *f;
    string filePath = "Saves/Save1/test.soas";
    f = fopen(filePath.c_str(), "rb");
    if (f == NULL){
        pError("Could not open test.soas for reading");
        return -1;
    }

    fclose(f);
    return 0;
}

void WorldIO::addToSaveList(Chunk *ch)
{
    if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
        ch->dirty = 0;
        string rs = getRegionString(ch);
        _queueLock.lock();
        ch->inSaveThread = 1;
        chunksToSave.push(ch);
        _queueLock.unlock();
        _cond.notify_one();
    }
}

void WorldIO::addToSaveList(vector <Chunk *> &chunks)
{
    _queueLock.lock();
    Chunk *ch;
    for (size_t i = 0; i < chunks.size(); i++){
        ch = chunks[i];
        if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
            ch->inSaveThread = 1;
            ch->dirty = 0;
            chunksToSave.push(ch);
        }
    }
    _queueLock.unlock();
    _cond.notify_one();
}

void WorldIO::addToLoadList(Chunk *ch)
{
    if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
        _queueLock.lock();
        ch->loadStatus = 0;
        ch->inLoadThread = 1;
        chunksToLoad.push(ch);
        _queueLock.unlock();
        _cond.notify_one();
    }
}

void WorldIO::addToLoadList(vector <Chunk *> &chunks)
{
    _queueLock.lock();
    Chunk *ch;

    if (_shouldDisableLoading) {
        flcLock.lock();
        for (size_t i = 0; i < chunks.size(); i++){
            chunks[i]->loadStatus = 2;
            finishedLoadChunks.push_back(chunks[i]);
        }
        flcLock.unlock();
        _queueLock.unlock();
        return;
    }

    for (size_t i = 0; i < chunks.size(); i++){
        ch = chunks[i];
       
        if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
            ch->inLoadThread = 1;
            chunksToLoad.push(ch);
        }
        else{
            cout << "ERROR: Tried to add chunk to load list and its in a thread! : " << ch->position.x << " " << ch->position.y << " " << ch->position.z << endl;
        }
    }
    _queueLock.unlock();
    _cond.notify_one();
}

string WorldIO::getRegionString(Chunk *ch)
{
    int rot = ch->faceData.rotation;
    int face = ch->faceData.face;
    int idir = FaceOffsets[face][rot][0];
    int jdir = FaceOffsets[face][rot][1];
    int ip = (ch->faceData.ipos - GameManager::planet->radius/CHUNK_WIDTH)*idir;
    int jp = (ch->faceData.jpos - GameManager::planet->radius/CHUNK_WIDTH)*jdir;
    if (rot%2){ //when rot%2 i and j must switch
        return "r." + to_string(ip >> 4) + "." + to_string((int)floor(ch->position.y / 32.0f) >> 4) + "." + to_string(jp >> 4);
    }else{
        return "r." + to_string(jp >> 4) + "." + to_string((int)floor(ch->position.y / 32.0f) >> 4) + "." + to_string(ip >> 4);
    }
}

int WorldIO::openRegionFile(string reg, int face, bool create, FILE **file, int &fd)
{
    string filePath;
    struct stat statbuf;

    if (*file != NULL){
        if (reg != _currReg){
            if (_isDirtyLocationBuffer){

                if (fseek(*file, 0, SEEK_SET) != 0){ //go back to beginning of file to save the table
                    ThreadError("Fseek error G could not seek to start\n");
                }
                if (fwrite(_currLocationBuffer->buffer, 1, 16384, *file) != 16384){
                    ThreadError("Region write error G could not write loc buffer\n");
                }

                _isDirtyLocationBuffer = 0;
            }
            fclose(*file);
            *file = NULL;
        }else{
            return 0; //the file is already open
        }
    }

    _currReg = reg;
    _isDirtyLocationBuffer = 0;

    //Check to see if the save file has 

    filePath = saveFilePath + "/Region/f" + to_string(face) + "/" + _currReg + ".soar";
//    cout << filePath << endl;
    
    (*file) = fopen(filePath.c_str(), "rb+"); //open file if it exists
    if (*file == NULL){
        if (create){
            (*file) = fopen(filePath.c_str(), "wb+"); //create the file
            if (*file == NULL){
                return 1;
            }
        }else{
            return 1 ;
        }
    }
    fd = fileno(*file); //get file descriptor for truncate if needed

    if (fstat(fd, &statbuf) != 0) ThreadError("Stat call failed for region file open"); //get the file stats
    _fileSize = statbuf.st_size;
    if (_fileSize % SECTOR_SIZE){
        ThreadError((filePath + ": Save File size must be multiple of " + to_string(SECTOR_SIZE) + ". Remainder = " + to_string(_fileSize%SECTOR_SIZE)));
    }
    if (_fileSize == 0){ //set up the initial location data
        _fileSize = 16384;

        _iterLocationCache = _locationCache.find(_currReg);
        if (_iterLocationCache != _locationCache.end()){
            _currLocationBuffer = _iterLocationCache->second;
        }else{
            _currLocationBuffer = newLocationBuffer();
        }
         //initialize to 0
        memset(_currLocationBuffer->buffer, 0, 16384);
        if (fwrite(_currLocationBuffer->buffer, 1, 16384, *file) != 16384){
            ThreadError("Region write error F could not write loc buffer\n");
        }
        
        fflush(*file);
    }else{ //load inital location data into memory 
        _iterLocationCache = _locationCache.find(_currReg);
        if (_iterLocationCache != _locationCache.end()){
            _currLocationBuffer = _iterLocationCache->second;
        }else{
            _currLocationBuffer = newLocationBuffer();

            if (fseek(*file, 0, SEEK_SET) != 0){
                ThreadError("Region fseek error F could not seek to start\n");
            }
            if (fread(_currLocationBuffer->buffer, 1, 16384, *file) != 16384){ //read the whole buffer in
                ThreadError("Region read error H could not read locbuffer\n");
            }
        }
    }

    return 0;
}

int WorldIO::isChunkSaved(Chunk *ch)
{
    //cout << "RS:" << reg << " ";
    GLuint sLocOffset = getLocOffset(ch);

    int location = extractInt(_currLocationBuffer->buffer, sLocOffset);
    if (location != 0){
        return 1;
    }


    return 0;
}

int WorldIO::seekToChunkOffset(Chunk *ch)
{
    GLuint location;
    GLuint offset;

    _locOffset = getLocOffset(ch);
        
    location = extractInt(_currLocationBuffer->buffer, _locOffset);
    offset = (location >> 8);

    if (offset == 0){ //check to see if the 4 bytes are empty//endianness?
        _chunkOffset = _fileSize/SECTOR_SIZE; //if its empty, set our offset to the end of the file.
        _chunkBlockSize = 0; //it has no size
    }else{
        _chunkOffset = offset; //grab the 3 offset bytes
        _chunkBlockSize = _currLocationBuffer->buffer[_locOffset + 3]; //grab the block size byte
    }

    if (fseek(_threadFile, _chunkOffset*SECTOR_SIZE, SEEK_SET) != 0){  //seek to the chunk location
        cout << "Region: Chunk data fseek B error! " << _fileSize << " " << _chunkOffset << " " << _chunkOffset*SECTOR_SIZE << " " << _threadFile << endl;
        int a;
        cin >> a;
    }
    return 0;
}

void WorldIO::closeFile()
{
    if (_threadFile != NULL) fclose(_threadFile);
    _threadFile = NULL;
}

i32 WorldIO::truncate(i64 size)
{
#if defined(_WIN32) || defined(_WIN64) 
    return _chsize(_threadFileDescriptors, size);
#else
  #ifdef POSIX
    return ftruncate(fd, size);
  #else
    // code for other OSes
  #endif
#endif
}

GLuint WorldIO::getLocOffset(Chunk *ch)
{
    int idir = FaceOffsets[ch->faceData.face][ch->faceData.rotation][0];
    int jdir = FaceOffsets[ch->faceData.face][ch->faceData.rotation][1];
    int ip = (ch->faceData.ipos - GameManager::planet->radius/CHUNK_WIDTH)*idir;
    int jp = (ch->faceData.jpos - GameManager::planet->radius/CHUNK_WIDTH)*jdir;

    if (ch->faceData.rotation%2){ //when rot%2 i and j must switch
        int tmp = ip;
        ip = jp;
        jp = tmp;
    }
    
    int ym = ((int)floor(ch->position.y / (float)CHUNK_WIDTH) % 16);
    int im = ip % 16;
    int jm = jp % 16;

    if (ym < 0) ym += 16;//modulus is weird in c++ for negative numbers
    if (im < 0) im += 16;
    if (jm < 0) jm += 16;
    GLuint lc = 4*(jm + im * 16 + ym * 256);
    if (lc >= 16384){
        cout << "WRONG LOC OFFSET " << jm << " " << (im) * 16 << " " << ym * 256 << " " << lc << endl;
        int a;
        cin >> a;
    }
    return lc;
}

LocationBuffer *WorldIO::newLocationBuffer()
{
    LocationBuffer *locBuf;
    if (_locationCacheQueue.size() >= _maxLocationBufferCacheSize){
        locBuf = _locationCacheQueue.front();
        _locationCacheQueue.pop();
        auto i = _locationCache.find(locBuf->reg);
        if (i != _locationCache.end()){
            _locationCache.erase(locBuf->reg);
            delete locBuf;
        }

    }

    locBuf = new LocationBuffer;
    locBuf->reg = _currReg; //maybe reg should be parameter
    _locationCache.insert(make_pair(locBuf->reg, locBuf));
    _locationCacheQueue.push(locBuf);
    return locBuf;
}

void WorldIO::clearLoadList()
{
    _queueLock.lock();
    queue<Chunk*>().swap(chunksToLoad); //clear the queue
    _queueLock.unlock();
}

int WorldIO::getLoadListSize()
{    
    return chunksToLoad.size();
}

int WorldIO::getSaveListSize()
{
    int rv;
    _queueLock.lock();
    rv = chunksToSave.size();
    _queueLock.unlock();
    return rv;
}

void WorldIO::readWriteChunks()
{
    unique_lock<mutex> ulock(_queueLock);
    Chunk *ch;
    bool failed;
    string reg;
    while (!_isDone){
        if (_isDone){
            ulock.unlock();
            _isThreadFinished = 1;
            return;
        }
        _cond.wait(ulock); //wait for a notification that queue is not empty

        if (_isDone){
            ulock.unlock();
            _isThreadFinished = 1;
            return;
        }
        while (chunksToLoad.size() || chunksToSave.size()){ //loops through the load and save queues
            if (chunksToLoad.size()){ //do load list first

                ch = chunksToLoad.front();
                chunksToLoad.pop();
                ulock.unlock();

                reg = getRegionString(ch);

                if ((openRegionFile(reg, ch->faceData.face, 0, &_threadFile, _threadFileDescriptors) == 0) && isChunkSaved(ch)){
                    failed = loadFromFile(ch);

                    if (failed){
                        ch->loadStatus = 1;
                        deleteChunkFile(ch);
                    }
                }
                else{
                    ch->loadStatus = 2; //it isn't saved, so main thread will give it to the generate list.
                }
                flcLock.lock();
                finishedLoadChunks.push_back(ch);
                flcLock.unlock();

                ulock.lock();
            }
            else if (chunksToSave.size()){
                ch = chunksToSave.front();
                ulock.unlock();

                reg = getRegionString(ch);

                if (openRegionFile(reg, ch->faceData.face, 1, &_threadFile, _threadFileDescriptors)){
                    ThreadError("OPEN REG ERROR 2\n");
                };

                saveToFile(ch);
                
                //dont always do this?
                if (_isDirtyLocationBuffer){
                    fseek(_threadFile, 0, SEEK_SET); //go back to beginning of file to save the table
                    fwrite(_currLocationBuffer->buffer, 1, 16384, _threadFile);
                    _isDirtyLocationBuffer = 0;
                }
                
                ulock.lock();
                chunksToSave.pop();
                ch->inSaveThread = 0; //race condition! make a new queue!
            }
        }
    }
}

void WorldIO::beginThread()
{
    _isDone = 0;
    _currReg = "";
    _isThreadFinished = 0;
    _isDirtyLocationBuffer = 0;
    _currLocationBuffer = NULL;
    readWriteThread = NULL;
    _threadFile = NULL;
    readWriteThread = new std::thread(&WorldIO::readWriteChunks, this);
}

void WorldIO::onQuit()
{

    clearLoadList();
    while (getSaveListSize() != 0);

    _queueLock.lock();
    _isDone = 1;
    _queueLock.unlock();
    _cond.notify_one();
    if (readWriteThread != NULL && readWriteThread->joinable()) readWriteThread->join();
    delete readWriteThread;
    readWriteThread = NULL;

    if (_threadFile != NULL) fclose(_threadFile);
    _currLocationBuffer = NULL;
    _threadFile = NULL;
    _isDirtyLocationBuffer = 0;

    while (_locationCacheQueue.size()){
        delete _locationCacheQueue.front();
        _locationCacheQueue.pop();
    }
    _locationCache.clear();

    
    finishedLoadChunks.clear();
}