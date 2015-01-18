#include "stdafx.h"
#include "WSOAtlas.h"

#include <Vorb/io/IOManager.h>

#include "WSOData.h"

// This Information Is Found At The Beginning Of The WSO File
struct WSOFileHeader {
public:
    // The Size Of DATA Segment
    i32 dataSegmentSize;
    // Amount Of WSO Index Information To Read
    i32 wsoCount;
};
// This Is Information About A WSO
struct WSOIndexInformation {
public:
    // Location In The File
    i32 fileOffset;
    // Amount Of Data To Read
    i32v3 size;

    // The Length Of The String Of The WSO's Name
    i32 lenName;
    // The Length Of The String Of The WSO's Model File (If Any)
    i32 lenModelFile;
};

WSOAtlas::WSOAtlas() {
    // Guess What... I Don't Have To Do Anything
}
WSOAtlas::~WSOAtlas() {
    // Dang... Now I Have Work To Do
    clear();
}

void WSOAtlas::add(WSOData* data) {
    // Set Its Index
    data->index = _data.size();

    // Place In The Data
    _data.push_back(data);

    // Add By Name
    _mapName[data->name] = data;
}

void WSOAtlas::load(const cString file) {
    vio::IOManager iom;

    // Attempt To Open The File
    vfstream f = iom.openFile(file, vio::FileOpenFlags::READ_ONLY_EXISTING | vio::FileOpenFlags::BINARY);
    if (!f.isOpened()) return;

    // Read The Header
    WSOFileHeader header;
    f.read(1, sizeof(WSOFileHeader), &header);

    // Read All The Index Information
    WSOIndexInformation* indices = new WSOIndexInformation[header.wsoCount];
    f.read(header.wsoCount, sizeof(WSOIndexInformation), indices);

    // Read The DATA Segment
    ubyte* data = new ubyte[header.dataSegmentSize];
    f.read(header.dataSegmentSize, sizeof(ubyte), data);

    // Close The File
    f.close();

    // Allocate Memory For All The WSO Data
    WSOData* wsoData = new WSOData[header.wsoCount]();
    _allocatedMem.push_back(wsoData);

    // Calculate Block Sizes For WSOs
    i32 nameBlockSize = 0, modelFileBlockSize = 0, idBlockSize = 0;
    for (i32 i = 0; i < header.wsoCount; i++) {
        nameBlockSize += indices[i].lenName;
        modelFileBlockSize += indices[i].lenModelFile;
        idBlockSize += indices[i].size.x * indices[i].size.y * indices[i].size.z;
    }

    // Allocate Memory For Names
    nameBlockSize += header.wsoCount;
    cString names = new char[nameBlockSize];
    _allocatedMem.push_back(names);

    // Allocate Memory For Model Files
    modelFileBlockSize += header.wsoCount;
    cString modelFiles = new char[modelFileBlockSize];
    _allocatedMem.push_back(modelFiles);
    
    // Allocate Memory For IDs
    i16* ids = new i16[idBlockSize];
    _allocatedMem.push_back(ids);

    // Create All The WSOs
    for (i32 i = 0; i < header.wsoCount; i++) {
        // Find Location In DATA
        ubyte* wso = data + indices[i].fileOffset;
        wsoData[i].size = indices[i].size;

        // Copy Over The Name
        wsoData[i].name = names;
        memcpy_s(wsoData[i].name, indices[i].lenName, wso, indices[i].lenName);
        wsoData[i].name[indices[i].lenName] = 0;
        names += indices[i].lenName + 1;

        // Move To Model File Portion
        wso += indices[i].lenName;

        // Copy Over The File (Otherwise Use Voxels As The Default)
        if (indices[i].lenModelFile > 0) {
            wsoData[i].modelFile = names;
            memcpy_s(wsoData[i].modelFile, indices[i].lenModelFile, wso, indices[i].lenModelFile);
            wsoData[i].modelFile[indices[i].lenModelFile] = 0;
            names += indices[i].lenModelFile + 1;

            // Move To ID Portion
            wso += indices[i].lenModelFile;
        }
        else {
            wsoData[i].modelFile = nullptr;
        }

        // Copy Over ID Information
        wsoData[i].wsoIDs = ids;
        i32 idSize = wsoData[i].getBlockCount() * sizeof(i16);
        memcpy_s(wsoData[i].wsoIDs, idSize, wso, idSize);
        ids += wsoData[i].getBlockCount();

        // Add This Into The Atlas
        add(wsoData + i);
    }

    // Delete File Info
    delete[] indices;
    delete[] data;
}

void WSOAtlas::clear() {
    // Free All The Allocated Memory
    for (i32 i = _allocatedMem.size() - 1; i >= 0; i--) {
        delete[] _allocatedMem[i];
    }

    // Clear ADT Memory
    std::vector<WSOData*>().swap(_data);
    std::map<nString, WSOData*>().swap(_mapName);
    std::vector<void*>().swap(_allocatedMem);
}