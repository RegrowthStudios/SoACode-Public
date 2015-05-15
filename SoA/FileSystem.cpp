#include "stdafx.h"
#include "FileSystem.h"

#include <sys/stat.h>

#include <Vorb/io/IOManager.h>
#include <Vorb/io/Keg.h>
#include <Vorb/io/Directory.h>
#include <Vorb/graphics/TextureCache.h>
#include <ZLIB/ioapi.h>
#include <ZLIB/unzip.h>

#include "Animation.h"
#include "BlockPack.h"
#include "Chunk.h"
#include "Errors.h"
#include "FloraGenerator.h"
#include "GameManager.h"
#include "SoaOptions.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticleEngine.h"
#include "Particles.h"
#include "TerrainGenerator.h"

#include "Texture2d.h"

#include "TexturePackLoader.h"
#include "WorldStructs.h"
#include "ZipFile.h"

FileManager fileManager;

FileManager::FileManager() {}

nString FileManager::getSaveFileNameDialog(const nString &prompt, const char *initialDir) {
#ifdef _WIN32
    char pathBuffer[1024];
    const int BUFSIZE = 1024;
    char buffer[BUFSIZE] = { 0 };
    _fullpath(pathBuffer, initialDir, 1024);
    OPENFILENAME ofns = { 0 };
    ofns.lStructSize = sizeof(ofns);
    ofns.lpstrInitialDir = pathBuffer;
    ofns.lpstrFile = buffer;
    ofns.nMaxFile = BUFSIZE;
    ofns.lpstrTitle = prompt.c_str();
    GetSaveFileName(&ofns);
    return buffer;
#elif
    std::cout << "ERROR! FILE DIALOG NOT IMPLEMENTED FOR THIS FILE SYSTEM\n";
    int a;
    std::cin >> a;
#endif
    return "";
}

nString FileManager::loadTexturePackDescription(nString fileName) {
    bool isZip = fileName.substr(fileName.size() - 4) == ".zip";
    if (!isZip) fileName += "/";
    nString rv = "";

    if (isZip) {
        ZipFile zipFile(fileName);
        if (zipFile.isFailure()) exit(1000);
        size_t filesize;
        unsigned char *resData;

        //resolution
        resData = zipFile.readFile("description.txt", filesize);
        if (resData != NULL) {
            rv = (char *)resData;
            delete[] resData;
        }
        while (rv.size() > filesize) {
            rv.pop_back();
        }
        if (rv.size() == 0) return "No description.";
        return rv;
    } else {
        std::ifstream descFile(fileName + "description.txt");
        if (descFile.fail()) {
            perror((fileName + "description.txt").c_str());
            descFile.close();
            return "No description.";
        }
        char buf[1024];
        while (!descFile.eof()) {
            descFile.getline(buf, 1024);
            rv += buf;
        }
        descFile.close();
        if (rv.size() == 0) return "No description.";
        return rv;
    }
}

i32 FileManager::createWorldFile(nString filePath) {
    std::ofstream file(filePath + "world.txt");
    if (file.fail()) {
        perror((filePath + "world.txt").c_str());
        return 1;
    }
    throw 404;
   // file << menuOptions.selectPlanetName << std::endl;
    file.close();
    return 0;
}
nString FileManager::getWorldString(nString filePath) {
    std::ifstream file(filePath + "world.txt");
    if (file.fail()) {
        return "";
    }
    nString rv = "";
    file >> rv;
    file.close();
    return rv;
}

inline void readLine(ui8* buffer, cString dest, i32& size) {

    for (int i = 0; i < size; i++) {
        if (buffer[i] == '\n') {
            size = i;
            dest[i] = '\n';
            return;
        } else if (buffer[i] == '\r') {
            if (buffer[i + 1] == '\n') {
                size = i + 1;
            } else {
                size = i;
            }
            dest[i] = '\n';
            return;
        } else {
            dest[i] = buffer[i];
        }
    }
}

i32 FileManager::getParticleType(nString fileName) {
    /* auto it = _particleTypeMap.find(fileName);
     if (it != _particleTypeMap.end()) {
     return it->second;
     } else {
     Animation *anim = NULL;
     vg::Texture ti = getTexture(fileName, &anim);
     particleTypes.push_back(ParticleType());
     particleTypes.back().texture = ti;
     if (anim == NULL) {
     particleTypes.back().animation = new Animation();
     particleTypes.back().animation->texture = ti;
     particleTypes.back().animation->duration = 100;
     particleTypes.back().animation->xFrames = 1;
     particleTypes.back().animation->yFrames = 1;
     particleTypes.back().animation->repetitions = 1;
     particleTypes.back().animation->fadeOutBegin = INT_MAX;
     } else {
     particleTypes.back().animation = anim;
     particleTypes.back().animation->texture = ti;
     }
     _particleTypeMap.insert(make_pair(fileName, particleTypes.size() - 1));
     return particleTypes.size() - 1;
     }*/
    return -1;
}
