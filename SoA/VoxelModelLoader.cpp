#include "stdafx.h"
#include "VoxelModelLoader.h"

#include <iostream>
#include <fstream>

VoxelModelLoader::VoxelModelLoader() {
    //Empty
}


VoxelModelLoader::~VoxelModelLoader() {
    //Empty
}

std::vector<VoxelMatrix*> VoxelModelLoader::loadModel(const nString& filePath) {
    /*
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if(!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return std::vector<VoxelMatrix*>();
    } else {
        std::cout << "Filed Open Successfully!" << std::endl;
    }*/
    FILE* file = NULL;
    fopen_s(&file, filePath.c_str(), "rb");

    int ok = 0;
    ui8* version = new ui8[4];
    ok = fread(&version[0], sizeof(char)*4, 1, file) == 1;
    //ui32 version;
    //file.read((char*)&version, sizeof(ui32));
    //file >> version;
    ui32 colorFormat;
    ok = fread(&colorFormat, sizeof(ui32), 1, file) == 1;
    //file.read((char*)&colorFormat, sizeof(ui32));
    //file >> colorFormat;
    ui32 zAxisOrientation;
    ok = fread(&zAxisOrientation, sizeof(ui32), 1, file) == 1;
    //file.read((char*)&zAxisOrientation, sizeof(ui32));
    //file >> zAxisOrientation;
    ui32 compressed;
    ok = fread(&compressed, sizeof(ui32), 1, file) == 1;
    //file.read((char*)&compressed, sizeof(ui32));
    //file >> compressed;
    ui32 visibilityMaskEncoded;
    ok = fread(&visibilityMaskEncoded, sizeof(ui32), 1, file) == 1;
    //file.read((char*)&visibilityMaskEncoded, sizeof(ui32));
    //file >> visibilityMaskEncoded;
    ui32 numMatrices;
    ok = fread(&numMatrices, sizeof(ui32), 1, file) == 1;
    //file.read((char*)&numMatrices, sizeof(ui32));
    //file >> numMatrices;
    
    std::vector<VoxelMatrix*> matrices;

    for(int i = 0; i < numMatrices; i++) {
        char nameLength = 0;
        ok = fread((char*)&nameLength, sizeof(char), 1, file) == 1;
        //file >> nameLength;
        char* name = new char[nameLength + 1];
        ok = fread(name, sizeof(char)*nameLength, 1, file) == 1;
        name[nameLength] = 0;
        //file.read(name, nameLength);
        printf(name);
        //std::cout << name << std::endl;

        VoxelMatrix* matrix = new VoxelMatrix();

        ok = fread(&matrix->size.x, sizeof(ui32), 1, file) == 1;
        ok = fread(&matrix->size.y, sizeof(ui32), 1, file) == 1;
        ok = fread(&matrix->size.z, sizeof(ui32), 1, file) == 1;

        ok = fread(&matrix->position.x, sizeof(ui32), 1, file) == 1;
        ok = fread(&matrix->position.x, sizeof(ui32), 1, file) == 1;
        ok = fread(&matrix->position.x, sizeof(ui32), 1, file) == 1;

        //file >> matrix->size.x >> matrix->size.y >> matrix->size.z;
        //file >> matrix->position.x >> matrix->position.y >> matrix->position.z;

        matrix->data = new ColorRGBA8[matrix->size.x * matrix->size.y * matrix->size.z];
        matrices.push_back(matrix);

        if(compressed == 0) { // Uncompressed Data
            for(i32 z = 0; z < matrix->size.x; z++) {
                for(i32 y = 0; y < matrix->size.y; y++) {
                    for(i32 x = 0; x < matrix->size.z; x++) {
                        //file.read((char*)(matrix->data + matrix->getIndex(i32v3(x, y, z))), 4);
                        ui32 data = 0;
                        ok = fread(&data, sizeof(ui32), 1, file) == 1;
                        ui32 r = data & 0x000000ff;
                        ui32 g = (data & 0x0000ff00) >> 8;
                        ui32 b = (data & 0x00ff0000) >> 16;
                        ui32 a = (data & 0xff000000) >> 24;
                        matrix->data[matrix->getIndex(i32v3(x, y, z))] = ColorRGBA8(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                    }
                }
            }
        } else { // RLE compressed
            i32 z = 0;
            while(z < matrix->size.z) {
                z++;
                ui32 index;
                while(true) {
                    ui32 data = 0;
                    //file >> data;
                    ok = fread(&data, sizeof(ui32), 1, file) == 1;
                    if(data == NEXT_SLICE_FLAG) {
                        break;
                    } else if(data == CODE_FLAG) {
                        ui32 count = 0;
                        //file >> count >> data;
                        ok = fread(&count, sizeof(ui32), 1, file) == 1;
                        ok = fread(&data, sizeof(ui32), 1, file) == 1;
                        for(i32 j = 0; j < count; j++) {
                            i32 x = index % matrix->size.x + 1;
                            i32 y = index / matrix->size.x + 1;
                            index++;
                            ui32 r = data & 0x000000ff;
                            ui32 g = (data & 0x0000ff00) >> 8;
                            ui32 b = (data & 0x00ff0000) >> 16;
                            ui32 a = (data & 0xff000000) >> 24;
                            matrix->data[matrix->getIndex(i32v3(x, y, z))] = ColorRGBA8(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                        }
                    } else {
                        i32 x = index % matrix->size.x + 1;
                        i32 y = index / matrix->size.y + 1;
                        index++;
                        ui32 r = data & 0x000000ff;
                        ui32 g = (data & 0x0000ff00) >> 8;
                        ui32 b = (data & 0x00ff0000) >> 16;
                        ui32 a = (data & 0xff000000) >> 24;
                        matrix->data[matrix->getIndex(i32v3(x, y, z))] = ColorRGBA8(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                    }
                }
            }
        }
    }
    return matrices;
}