#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "ImageLoading.h"

#include "Constants.h"
#include "IOManager.h"
#include "utils.h"

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint loadDDS(const char * imagepath, int smoothType) {
    printf("Reading image %s\n", imagepath);
    unsigned char header[124];

    FILE *fp;

    /* try to open the file */
    fp = fopen(imagepath, "rb");
    if (fp == NULL)
        return 0;

    /* verify the type of file */
    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        fclose(fp);
        return 0;
    }

    /* get the surface desc */
    fread(&header, 124, 1, fp);

    unsigned int height = *(unsigned int*)&(header[8]);
    unsigned int width = *(unsigned int*)&(header[12]);
    unsigned int linearSize = *(unsigned int*)&(header[16]);
    unsigned int mipMapCount = *(unsigned int*)&(header[24]);
    unsigned int fourCC = *(unsigned int*)&(header[80]);


    unsigned char * buffer;
    unsigned int bufsize;
    /* how big is it going to be including all mipmaps? */
    bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
    fread(buffer, 1, bufsize, fp);
    /* close the file pointer */
    fclose(fp);

    unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int format;
    switch (fourCC) {
    case FOURCC_DXT1:
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        free(buffer);
        return 0;
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    unsigned int offset = 0;

    /* load the mipmaps */
    for (unsigned int level = 0; level < mipMapCount && (width || height); ++level) {
        unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
            0, size, buffer + offset);
        offset += size;
        width /= 2;
        height /= 2;
    }
    if (smoothType == 1) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else if (smoothType == 2) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else if (smoothType == 3) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_EXT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_EXT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    if (mipMapCount == 1) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    free(buffer);

    return textureID;
}

ui32v2 readImageSize(IOManager* iom, const cString imagePath) {
    FILE* file = iom->openFile(imagePath, "rb");
    ui32v2 imageSize;
    fseek(file, 16, SEEK_SET);
    fread(&imageSize, sizeof(ui32), 2, file);
    fclose(file);
    imageSize.x = changeEndian(imageSize.x);
    imageSize.y = changeEndian(imageSize.y);
    return imageSize;
}

//if madeTexture == 1, it constructs an opengl texture and returns NULL. otherwise it returns the pixel data
ui8* loadPNG(TextureInfo& texInfo, const cString imagepath, PNGLoadInfo texParams, bool makeTexture) {
    texInfo.freeTexture();
    FILE * file = fopen(imagepath, "rb");
    vector <unsigned char> fileData;
    unsigned char * fileData2;
    unsigned int imageWidth, imageHeight;
    size_t fileSize;
    //printf("Reading image %s\n", imagepath);

    if (!file) {
        perror(imagepath); return NULL;
    }

    struct stat filestatus;
    stat(imagepath, &filestatus);
    fileSize = filestatus.st_size;
    vector <unsigned char> imgData(fileSize);

    fread(&(imgData[0]), 1, fileSize, file);
    fclose(file);

    unsigned error = lodepng::decode(fileData, imageWidth, imageHeight, imgData);


    //if there's an error, display it
    if (error) {
        std::cout << "png decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
        return NULL;
    }

    fileSize = fileData.size();

    texInfo.width = imageWidth;
    texInfo.height = imageHeight;

    // Determine The Maximum Number Of Mipmap Levels Available
    i32 maxMipmapLevels = 0;
    i32 size = min(imageWidth, imageHeight);
    while (size > 1) {
        maxMipmapLevels++;
        size >>= 1;
    }

    //flip it
    fileData2 = new unsigned char[fileData.size()];
    int sz = fileData.size();
    int rowOffset;
    for (size_t r = 0; r < imageHeight; r++) {
        rowOffset = r*imageWidth * 4;
        for (size_t i = 0; i < imageWidth * 4; i += 4) {
            fileData2[sz + i - rowOffset - imageWidth * 4] = fileData[i + rowOffset];
            fileData2[sz + i + 1 - rowOffset - imageWidth * 4] = fileData[i + 1 + rowOffset];
            fileData2[sz + i + 2 - rowOffset - imageWidth * 4] = fileData[i + 2 + rowOffset];
            fileData2[sz + i + 3 - rowOffset - imageWidth * 4] = fileData[i + 3 + rowOffset];
        }
    }

    if (makeTexture == 0) {
        return fileData2;
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(fileData2[0]));
    delete[] fileData2;

    // Setup Texture Sampling Parameters
    texParams.samplingParameters->set(GL_TEXTURE_2D);

    // Create Mipmaps If Necessary
    if (texParams.mipmapLevels > 0) {
        glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, min(texParams.mipmapLevels, maxMipmapLevels));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, min(texParams.mipmapLevels, maxMipmapLevels));
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    texInfo.ID = textureID;
    return NULL;
}
ui8* loadPNG(TextureInfo& texInfo, ui8* pngdata, size_t fileSize, PNGLoadInfo texParams, bool makeTexture) {
    texInfo.freeTexture();
    if (pngdata == NULL) return NULL;

    vector <unsigned char> fileData;
    unsigned char * fileData2;
    unsigned int imageWidth, imageHeight;
    vector <unsigned char> imgdata(fileSize);
    memcpy(&(imgdata[0]), pngdata, fileSize); //should switch to vector by default

    unsigned error = lodepng::decode(fileData, imageWidth, imageHeight, imgdata);

    //if there's an error, display it
    if (error) {
        std::cout << "png decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
        return NULL;
    }


    fileSize = fileData.size();
    delete[] pngdata;

    texInfo.width = imageWidth;
    texInfo.height = imageHeight;

    // Determine The Maximum Number Of Mipmap Levels Available
    i32 maxMipmapLevels = 0;
    i32 size = min(imageWidth, imageHeight);
    while (size > 1) {
        maxMipmapLevels++;
        size >>= 1;
    }

    //flip it
    fileData2 = new unsigned char[fileData.size()];
    int sz = fileData.size();
    int rowOffset;
    for (size_t r = 0; r < imageHeight; r++) {
        rowOffset = r*imageWidth * 4;
        for (size_t i = 0; i < imageWidth * 4; i += 4) {
            fileData2[sz + i - rowOffset - imageWidth * 4] = fileData[i + rowOffset];
            fileData2[sz + i + 1 - rowOffset - imageWidth * 4] = fileData[i + 1 + rowOffset];
            fileData2[sz + i + 2 - rowOffset - imageWidth * 4] = fileData[i + 2 + rowOffset];
            fileData2[sz + i + 3 - rowOffset - imageWidth * 4] = fileData[i + 3 + rowOffset];
        }
    }

    if (makeTexture == 0) {
        return fileData2;
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(fileData2[0]));
    delete[] fileData2;

    // Setup Texture Sampling Parameters
    texParams.samplingParameters->set(GL_TEXTURE_2D);

    // Create Mipmaps If Necessary
    if (texParams.mipmapLevels > 0) {
        glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, min(texParams.mipmapLevels, maxMipmapLevels));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, min(texParams.mipmapLevels, maxMipmapLevels));
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    texInfo.ID = textureID;
    return NULL;
}

SamplerState textureSamplers[6] = {
    SamplerState(TextureMinFilter::NEAREST_MIPMAP_LINEAR, TextureMagFilter::NEAREST,
    TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE),
    SamplerState(TextureMinFilter::LINEAR_MIPMAP_LINEAR, TextureMagFilter::LINEAR,
    TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureWrapMode::REPEAT),
    SamplerState(TextureMinFilter::LINEAR_MIPMAP_LINEAR, TextureMagFilter::NEAREST,
    TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureWrapMode::REPEAT),
    SamplerState(TextureMinFilter::LINEAR_MIPMAP_LINEAR, TextureMagFilter::LINEAR,
    TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE),
    SamplerState(TextureMinFilter::NEAREST_MIPMAP_LINEAR, TextureMagFilter::NEAREST,
    TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE),
    SamplerState(TextureMinFilter::NEAREST_MIPMAP_LINEAR, TextureMagFilter::NEAREST,
    TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE)
};