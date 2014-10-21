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