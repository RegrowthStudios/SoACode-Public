#include "stdafx.h"
#include "VoxelModelLoader.h"

#include <iostream>
#include <fstream>

#include "VoxelMatrix.h"

VoxelModelLoader::VoxelModelLoader() {
    //Empty
}

VoxelMatrix VoxelModelLoader::loadModel(const nString& filePath) {
    FILE* file = NULL;
    fopen_s(&file, filePath.c_str(), "rb");

    bool ok = true;
    ui8* version = new ui8[4];
    ok = fread(&version[0], sizeof(char)*4, 1, file) == 1;
    ui32 colorFormat;
    ok = fread(&colorFormat, sizeof(ui32), 1, file) == 1;
    ui32 zAxisOrientation;
    ok = fread(&zAxisOrientation, sizeof(ui32), 1, file) == 1;
    ui32 compressed;
    ok = fread(&compressed, sizeof(ui32), 1, file) == 1;
    ui32 visibilityMaskEncoded;
    ok = fread(&visibilityMaskEncoded, sizeof(ui32), 1, file) == 1;
    ui32 numMatrices;
    ok = fread(&numMatrices, sizeof(ui32), 1, file) == 1;

    // Even though .qb can have multiple matrices, we assume a single matrix.
    VoxelMatrix matrix;

    char nameLength = 0;
    ok = fread((char*)&nameLength, sizeof(char), 1, file) == 1;
    char* name = new char[nameLength + 1];
    ok = fread(name, sizeof(char)*nameLength, 1, file) == 1;
    name[nameLength] = 0;
    printf(name);

    ok = fread(&matrix.size.x, sizeof(ui32), 1, file) == 1;
    ok = fread(&matrix.size.y, sizeof(ui32), 1, file) == 1;
    ok = fread(&matrix.size.z, sizeof(ui32), 1, file) == 1;

    ok = fread(&matrix.position.x, sizeof(ui32), 1, file) == 1;
    ok = fread(&matrix.position.y, sizeof(ui32), 1, file) == 1;
    ok = fread(&matrix.position.z, sizeof(ui32), 1, file) == 1;

    matrix.data = new ColorRGBA8[matrix.size.x * matrix.size.y * matrix.size.z];

    if(compressed == 0) { // Uncompressed Data
        for(i32 z = 0; z < matrix.size.z; z++) {
            for(i32 y = 0; y < matrix.size.y; y++) {
                for(i32 x = 0; x < matrix.size.x; x++) {
                    ui32 data = 0;
                    ok = fread(&data, sizeof(ui32), 1, file) == 1;
                    ui32 r = data & 0x000000ff;
                    ui32 g = (data & 0x0000ff00) >> 8;
                    ui32 b = (data & 0x00ff0000) >> 16;
                    ui32 a = (data & 0xff000000) >> 24;
                    int location = matrix.getIndex(x, y, z);
                    matrix.data[location] = ColorRGBA8(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                }
            }
        }
    } else { // RLE compressed
        i32 z = 0;
        while(z < matrix.size.z) {
            ui32 index = 0;
            while(true) {
                ui32 data = 0;
                ok = fread(&data, sizeof(ui32), 1, file) == 1;
                if(data == NEXT_SLICE_FLAG) {
                    break;
                } else if(data == CODE_FLAG) {
                    ui32 count = 0;
                    ok = fread(&count, sizeof(ui32), 1, file) == 1;
                    ok = fread(&data, sizeof(ui32), 1, file) == 1;
                    for(i32 j = 0; j < count; j++) {
                        i32 x = index % matrix.size.x;
                        i32 y = index / matrix.size.x;
                        index++;
                        ui32 r = data & 0x000000ff;
                        ui32 g = (data & 0x0000ff00) >> 8;
                        ui32 b = (data & 0x00ff0000) >> 16;
                        ui32 a = (data & 0xff000000) >> 24;
                        int location = matrix.getIndex(x, y, z);
                        matrix.data[location] = ColorRGBA8(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                    }
                } else {
                    i32 x = index % matrix.size.x;
                    i32 y = index / matrix.size.x;
                    index++;
                    ui32 r = data & 0x000000ff;
                    ui32 g = (data & 0x0000ff00) >> 8;
                    ui32 b = (data & 0x00ff0000) >> 16;
                    ui32 a = (data & 0xff000000) >> 24;
                    int location = matrix.getIndex(x, y, z);
                    matrix.data[location] = ColorRGBA8(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                }
            }
            z++;
        }
    }
    
    return matrix;
}