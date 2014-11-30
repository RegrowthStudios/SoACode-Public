#include "stdafx.h"
#include "FileSystem.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <ZLIB/ioapi.c>
#include <ZLIB/unzip.c>

#include <sys/stat.h>

#include "Animation.h"
#include "BlockData.h"
#include "Chunk.h"
#include "Errors.h"
#include "FloraGenerator.h"
#include "GameManager.h"
#include "IOManager.h"
#include "Keg.h"
#include "Options.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticleEngine.h"
#include "Particles.h"
#include "Planet.h"
#include "Player.h"
#include "TerrainGenerator.h"
#include "TextureCache.h"
#include "TexturePackLoader.h"
#include "WorldStructs.h"
#include "ZipFile.h"

FileManager fileManager;

i32 FileManager::deleteDirectory(const nString& rootDirectory) {
    boost::system::error_code error;
    i32 filesRemoved = boost::filesystem::remove_all(rootDirectory, error);
#ifdef DEBUG
    // Print Error Message For Debugging
    if (error.value() != 0) printf("%s\n", error.message().c_str());
#endif // DEBUG
    return error.value();
}

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
    cout << "ERROR! FILE DIALOG NOT IMPLEMENTED FOR THIS FILE SYSTEM\n";
    int a;
    cin >> a;
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
        ifstream descFile(fileName + "description.txt");
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

i32 FileManager::makeSaveDirectories(nString filePath) {
    boost::filesystem::create_directory("Saves");
    boost::filesystem::create_directory(filePath);
    boost::filesystem::create_directory(filePath + "/Players");
    boost::filesystem::create_directory(filePath + "/Data");
    boost::filesystem::create_directory(filePath + "/World");
    boost::filesystem::create_directory(filePath + "/Region");
    boost::filesystem::create_directory(filePath + "/Region/f0");
    boost::filesystem::create_directory(filePath + "/Region/f1");
    boost::filesystem::create_directory(filePath + "/Region/f2");
    boost::filesystem::create_directory(filePath + "/Region/f3");
    boost::filesystem::create_directory(filePath + "/Region/f4");
    boost::filesystem::create_directory(filePath + "/Region/f5");
    return 0;
}
i32 FileManager::createSaveFile(nString filePath) {
    struct stat statbuf;

    if (boost::filesystem::exists(filePath)) {
        return 2;
    }

    if (!boost::filesystem::create_directory(filePath)) {
        perror(filePath.c_str()); pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/Players").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/Data").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/World").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/Region").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/Region/f0").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/Region/f1").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/Region/f2").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/Region/f3").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/Region/f4").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }
    if (!boost::filesystem::create_directory((filePath + "/Region/f5").c_str())) {
        pError("Failed to create directory in CreateSaveFile()"); return 1;
    }

    return setSaveFile(filePath);
}
i32 FileManager::createWorldFile(nString filePath) {
    ofstream file(filePath + "world.txt");
    if (file.fail()) {
        perror((filePath + "world.txt").c_str());
        return 1;
    }
    file << menuOptions.selectPlanetName << endl;
    file.close();
    return 0;
}
nString FileManager::getWorldString(nString filePath) {
    ifstream file(filePath + "world.txt");
    if (file.fail()) {
        return "";
    }
    nString rv = "";
    file >> rv;
    file.close();
    return rv;
}

i32 FileManager::setSaveFile(nString filePath) {
    struct stat statbuf;

    if (stat(filePath.c_str(), &statbuf) != 0) {
        pError("Save file does not exist.");
        return 1;
    }

    GameManager::saveFilePath = filePath;
    return 0;
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
    auto it = _particleTypeMap.find(fileName);
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
    }
    return -1;
}