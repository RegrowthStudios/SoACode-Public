#pragma once
#include "EditorVariables.h"

struct Animation;
class ZipFile;
class TreeType;
class Biome;
class ParticleEmitter;

class FileManager {
public:
    FileManager();
    
    i32 readZipFile(nString fileName);

    i32 deleteDirectory(const nString& refcstrRootDirectory);
  
    nString loadTexturePackDescription(nString fileName);

    i32 loadBlocks(nString filePath);
    i32 saveBlocks(nString filePath);

    nString getSaveFileNameDialog(const nString& prompt, const cString initialDir);

    i32 makeSaveDirectories(nString filePath);
    i32 createSaveFile(nString filePath);
    i32 createWorldFile(nString filePath);
    nString getWorldString(nString filePath);
    i32 getSaveListSize();

    i32 getParticleType(nString fileName);

    std::map<nString, BiomeVariable> biomeVariableMap;
    std::map<nString, NoiseVariable> noiseVariableMap;
private:
    void loadGameData();

    std::map<nString, i32> _particleTypeMap;
    std::map<nString, ParticleEmitter*> _emitterMap;
    std::map<nString, Animation*> _animationMap;
};

extern FileManager fileManager;
