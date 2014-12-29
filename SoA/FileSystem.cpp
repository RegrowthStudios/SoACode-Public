#include "stdafx.h"
#include "FileSystem.h"

#include <sys/stat.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <Vorb/IOManager.h>
#include <Vorb/Keg.h>
#include <Vorb/TextureCache.h>
#include <ZLIB/ioapi.c>
#include <ZLIB/unzip.c>

#include "Animation.h"
#include "BlockPack.h"
#include "Chunk.h"
#include "Errors.h"
#include "FloraGenerator.h"
#include "GameManager.h"
#include "Options.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticleEngine.h"
#include "Particles.h"
#include "Planet.h"
#include "Player.h"
#include "TerrainGenerator.h"
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

FileManager::FileManager() : isInitialized(false) {}
void FileManager::initialize() {
    isInitialized = 1;

    initializeINIMaps();
    makeBiomeVariableMap();
    makeNoiseVariableMap();
}

void FileManager::makeBiomeVariableMap() {
    Biome tmpBiome;
    biomeVariableMap.clear();

    //biomeVariableMap["VecIndex"] = TreeVariable(0, 4096, 1,((Uint32)&(tmpBiome.vecIndex) - ((Uint32)&tmpBiome)), 0);
    //biomeVariableMap["VecIndex"] = BiomeVariable(0, 4096, 1,((Uint32)&(tmpBiome.vecIndex) - ((Uint32)&tmpBiome)), 0);
    biomeVariableMap["AltColor_Active"] = BiomeVariable(0, 1, 1, ((Uint32)&(tmpBiome.hasAltColor) - ((Uint32)&tmpBiome)), 0);
    biomeVariableMap["isBase"] = BiomeVariable(0, 1, 1, ((Uint32)&(tmpBiome.isBase) - ((Uint32)&tmpBiome)), 0);
    biomeVariableMap["AltColor_R"] = BiomeVariable(0, 255, 1, ((Uint32)&(tmpBiome.r) - ((Uint32)&tmpBiome)), 2);
    biomeVariableMap["AltColor_G"] = BiomeVariable(0, 255, 1, ((Uint32)&(tmpBiome.g) - ((Uint32)&tmpBiome)), 2);
    biomeVariableMap["AltColor_B"] = BiomeVariable(0, 255, 1, ((Uint32)&(tmpBiome.b) - ((Uint32)&tmpBiome)), 2);
    biomeVariableMap["ID_Surface_Block"] = BiomeVariable(0, 4096, 1, ((Uint32)&(tmpBiome.surfaceBlock) - ((Uint32)&tmpBiome)), 3);
    biomeVariableMap["ID_Underwater_Block"] = BiomeVariable(0, 4096, 1, ((Uint32)&(tmpBiome.underwaterBlock) - ((Uint32)&tmpBiome)), 3);
    biomeVariableMap["ID_Beach_Block"] = BiomeVariable(0, 4096, 1, ((Uint32)&(tmpBiome.beachBlock) - ((Uint32)&tmpBiome)), 3);
    biomeVariableMap["Tree_Chance_Base"] = BiomeVariable(0, 0.02, .00001, ((Uint32)&(tmpBiome.treeChance) - ((Uint32)&tmpBiome)), 1);
    biomeVariableMap["Biome_Active_Threshold"] = BiomeVariable(0, 1.0, .01, ((Uint32)&(tmpBiome.applyBiomeAt) - ((Uint32)&tmpBiome)), 1);

    biomeVariableMap["Temp"] = BiomeVariable(-255, 500, 1, ((Uint32)&(tmpBiome.lowTemp) - ((Uint32)&tmpBiome)), 1);
    biomeVariableMap["Temp"].byteOffset2 = ((Uint32)&(tmpBiome.highTemp) - ((Uint32)&tmpBiome));
    biomeVariableMap["Temp_Transition_Length"] = BiomeVariable(0, 100, 1, ((Uint32)&(tmpBiome.tempSlopeLength) - ((Uint32)&tmpBiome)), 1);

    biomeVariableMap["Rain"] = BiomeVariable(-255, 500, 1, ((Uint32)&(tmpBiome.lowRain) - ((Uint32)&tmpBiome)), 1);
    biomeVariableMap["Rain"].byteOffset2 = ((Uint32)&(tmpBiome.highRain) - ((Uint32)&tmpBiome));
    biomeVariableMap["Rain_Transition_Length"] = BiomeVariable(0, 100, 1, ((Uint32)&(tmpBiome.rainSlopeLength) - ((Uint32)&tmpBiome)), 1);

    biomeVariableMap["Height_Max"] = BiomeVariable(0, 4096, 1, ((Uint32)&(tmpBiome.maxHeight) - ((Uint32)&tmpBiome)), 1);
    biomeVariableMap["Terrain_Mult_Min"] = BiomeVariable(0.0, 1.0, 0.01, ((Uint32)&(tmpBiome.minTerrainMult) - ((Uint32)&tmpBiome)), 1);
    biomeVariableMap["Height_Transition_Length"] = BiomeVariable(0, 1000, 1, ((Uint32)&(tmpBiome.maxHeightSlopeLength) - ((Uint32)&tmpBiome)), 0);


}
void FileManager::makeNoiseVariableMap() {
    NoiseInfo noise;
    noiseVariableMap.clear();

    noiseVariableMap["Persistence"] = NoiseVariable(0.0, 1.0, 0.01, ((Uint32)&(noise.persistence) - ((Uint32)&noise)), 1);
    noiseVariableMap["Frequency"] = NoiseVariable(0.00001, 0.01, 0.00001, ((Uint32)&(noise.frequency) - ((Uint32)&noise)), 1);
    noiseVariableMap["Bound_Low"] = NoiseVariable(0, 1000, 1, ((Uint32)&(noise.lowBound) - ((Uint32)&noise)), 1);
    noiseVariableMap["Bound_High"] = NoiseVariable(0, 1000, 1, ((Uint32)&(noise.upBound) - ((Uint32)&noise)), 1);
    noiseVariableMap["Scale"] = NoiseVariable(0, 10000, 1, ((Uint32)&(noise.scale) - ((Uint32)&noise)), 1);
    noiseVariableMap["Octaves"] = NoiseVariable(0, 12, 1, ((Uint32)&(noise.octaves) - ((Uint32)&noise)), 0);

    noiseVariableMap["Composition"] = NoiseVariable(0, 1, 1, ((Uint32)&(noise.composition) - ((Uint32)&noise)), 0);
    noiseVariableMap["Composition"].controlType = 1;
    noiseVariableMap["Composition"].listNames.push_back("Normal");
    noiseVariableMap["Composition"].listNames.push_back("Sand");

    //noiseVariableMap["Type"] = NoiseVariable(0, 100, 1,((Uint32)&(noise.type) - ((Uint32)&noise)), 0);
}

i32 FileManager::loadFloraNoiseFunctions(const cString filename, Planet* planet) {
    double persistence, lowbound, upbound, frequency;
    int octaves, type;
    nString types;
    std::map <nString, int>::iterator fit;

    std::ifstream file;

    char buffer[512];

    file.open(filename);
    if (file.fail()) {
        printf("ERROR: Flora Noise Function File %s could not be found!\nEnter any key to continue: ", filename);
        std::cin >> octaves;
        file.close();
        return 1;
    }

    planet->floraNoiseFunctions.resize(4096, NULL);

    while (file >> types) {
        if (types[0] == '#') {
            file.getline(buffer, 512);
        } else {
            fit = planet->floraLookupMap.find(types);
            if (fit == planet->floraLookupMap.end()) {
                printf("ERROR: (%s) Flora type %s not found in FloraData.txt\n", filename, types.c_str());
                printf("Enter any key to continue... ");
                std::cin >> types;
            } else {
                type = fit->second;
                if (!(file >> persistence >> frequency >> octaves >> lowbound >> upbound)) {
                    printf("    ERROR: Noise Function File %s has a format error!\nEnter any key to continue: ", filename);
                    std::cin >> octaves;
                    file.close();
                    return 1;
                }

                planet->floraNoiseFunctions[type] = new NoiseInfo;
                planet->floraNoiseFunctions[type]->type = type;
                planet->floraNoiseFunctions[type]->persistence = persistence;
                planet->floraNoiseFunctions[type]->frequency = frequency;
                planet->floraNoiseFunctions[type]->octaves = octaves;
                planet->floraNoiseFunctions[type]->lowBound = lowbound;
                planet->floraNoiseFunctions[type]->upBound = upbound;
            }
        }
    }
    file.close();
    return 0;
}
i32 FileManager::loadCloudNoiseFunctions(const cString filename, Planet* planet) {
    ////Per, Freq, Oct, Lo, Up, Sca, Type  
    //double persistence, lowbound, upbound, scale, frequency;
    //int octaves, type;
    //nString types;
    //bool hasTemp = 0, hasRainfall = 0;
    //bool isModifier;

    //ifstream file;

    //char buffer[512];

    //file.open(filename);
    //if (file.fail()){
    //    printf("ERROR: Noise Function File %s could not be found!\nEnter any key to continue: ", filename);
    //    cin >> octaves;
    //    file.close();
    //    return 1;
    //}
    //
    //while (file >> types){
    //    if (types[0] == '#'){
    //        file.getline(buffer, 512);
    //    }else{
    //        type = atoi(types.c_str());

    //        if (!(file >> persistence >> frequency >> octaves >> lowbound)){
    //            printf("    ERROR: Noise Function File %s has a format error!\nEnter any key to continue: ", filename);
    //            cin >> octaves;
    //            file.close();
    //            return 1;
    //        }
    //        if (type == 0){
    //            stormNoiseFunction = new NoiseInfo();
    //            stormNoiseFunction->frequency = frequency;
    //            stormNoiseFunction->persistence = persistence;
    //            stormNoiseFunction->octaves = octaves;
    //            stormNoiseFunction->type = type;
    //            stormNoiseFunction->lowBound = lowbound;
    //        }else if (type == 1){
    //            sunnyCloudyNoiseFunction = new NoiseInfo();
    //            sunnyCloudyNoiseFunction->frequency = frequency;
    //            sunnyCloudyNoiseFunction->persistence = persistence;
    //            sunnyCloudyNoiseFunction->octaves = octaves;
    //            sunnyCloudyNoiseFunction->type = type;
    //            sunnyCloudyNoiseFunction->lowBound = lowbound;
    //        }else if (type == 2){
    //            cumulusNoiseFunction = new NoiseInfo();
    //            cumulusNoiseFunction->frequency = frequency;
    //            cumulusNoiseFunction->persistence = persistence;
    //            cumulusNoiseFunction->octaves = octaves;
    //            cumulusNoiseFunction->type = type;
    //            cumulusNoiseFunction->lowBound = lowbound;
    //        }else{
    //            printf("    ERROR: Noise Function File %s invalid type!\nEnter any key to continue: ", filename);
    //            cin >> octaves;
    //            file.close();
    //            return 1;
    //        }
    //    }
    //}
    //file.close();
    return 0;
}
i32 FileManager::loadNoiseFunctions(const cString filename, bool mandatory, Planet* planet) {
    //Per, Freq, Oct, Lo, Up, Sca, Type  
    double persistence, lowbound, upbound, scale, frequency;
    int octaves, type;
    nString types;
    bool hasTemp = 0, hasRainfall = 0;
    bool isModifier;

    TerrainGenerator* generator = GameManager::terrainGenerator;

    std::ifstream file;

    char buffer[512];

    file.open(filename);
    if (file.fail()) {
        printf("ERROR: Noise Function File %s could not be found!\nEnter any key to continue: ", filename);
        std::cin >> octaves;
        file.close();
        return 1;
    }


    while (file >> types) {
        if (types[0] == '#') {
            file.getline(buffer, 512);
        } else {
            isModifier = 0;
            if (types[0] == '$') {
                file >> types;
                isModifier = 1;
            }
            type = atoi(types.c_str());

            if (!(file >> persistence >> frequency >> octaves >> lowbound >> upbound >> scale)) {
                printf("    ERROR: Noise Function File %s has a format error!\nEnter any key to continue: ", filename);
                std::cin >> octaves;
                file.close();
                return 1;
            }

            if (type == 10000) {
                generator->SetRiverNoiseFunction(persistence, frequency, octaves, lowbound, upbound, scale, type);
            } else if (type == 10001) {
                generator->SetTributaryNoiseFunction(persistence, frequency, octaves, lowbound, upbound, scale, type);
            } else if (type == 10002) {
                generator->SetBiomeOffsetNoiseFunction(persistence, frequency, octaves, lowbound, upbound, scale, type);
            } else if (type == 10003) {
                generator->SetPreturbNoiseFunction(persistence, frequency, octaves, lowbound, upbound, scale, type);
            } else if (type == 10004) {
                hasTemp = 1;
                generator->SetTemperatureNoiseFunction(persistence, frequency, octaves, lowbound, upbound, scale, type);
            } else if (type == 10005) {
                hasRainfall = 1;
                generator->SetRainfallNoiseFunction(persistence, frequency, octaves, lowbound, upbound, scale, type);
            } else {
                generator->AddNoiseFunction(persistence, frequency, octaves, lowbound, upbound, scale, type, isModifier);
            }
        }
    }
    file.close();
    if (mandatory) {
        if (!hasTemp) {
            std::cout << "    ERROR: Temperature noise function missing! Noise type = 30\nEnter any key to continue: ";
            std::cin >> octaves;
            return 1;
        }
        if (!hasRainfall) {
            std::cout << "    ERROR: Rainfall noise function missing! Noise type = 31\nEnter any key to continue: ";
            std::cin >> octaves;
            return 1;
        }
    }
    return 0;
}
i32 FileManager::loadFloraData(Planet *planet, nString worldFilePath) {
    std::ifstream file;
    nString s;
    int blockID, textureID, meshType;
    char buffer[512];

    file.open(worldFilePath + "Flora/FloraData.txt");
    if (file.fail()) {
        pError((worldFilePath + "Flora/FloraData.txt could not be opened for read").c_str());
        perror((worldFilePath + "Flora/FloraData.txt").c_str());
        file.close();
        return 1;
    }
    planet->floraLookupMap.clear();
    while (file >> s) {
        if (s[0] == '#') {
            file.getline(buffer, 512);
            continue;
        }
        sscanf(&(s[0]), "%d", &blockID);
        if (!(file >> textureID >> s >> meshType)) {
            printf("    ERROR: World/Flora/FloraData.txt has a format error!\nEnter any key to continue: ");
            std::cin >> s;
            file.close();
            return 1;
        }
        /*  if (meshType == 1) {
        meshType = MeshType::FLORA;
        } else {
        meshType = MeshType::CROSSFLORA;
        }*/
        if (!(Blocks[blockID].active)) { //for loading
            printf("ERROR: (FloraData.txt) block ID %d not defined!\n", blockID);
            printf("Enter any key to continue... ");
            std::cin >> s;
        } else {
            planet->floraLookupMap.insert(make_pair(s, planet->floraTypeVec.size()));
            PlantType *ft = new PlantType;
            ft->name = s;
            ft->baseBlock = blockID;
            planet->floraTypeVec.push_back(ft);
        }
        //if (Blocks[blockID].active){ //for manual creation
        //    printf("ERROR: (FloraData.txt) block ID %d already in use!\n", blockID);
        //    printf("Enter any key to continue... ");
        //    cin >> s;
        //}else{
        //    planet->floraLookupMap.insert(make_pair(s, planet->floraTypeVec.size()));
        //    FloraType *ft = new FloraType;
        //    ft->name = s;
        //    ft->baseBlock = blockID;
        //    planet->floraTypeVec.push_back(ft);
        //    for (int i = 0; i < s.size(); i++) if (s[i] == '_') s[i] = ' ';
        //    Blocks[blockID].SetName(s).SetMeshType(meshType).SetHealth(100).SetPhysics(P_SOLID).SetColor(255, 255, 255).SetTexture(textureID).SetCollide(0).SetOcclude(0).SetWaveEffect(1).SetNumParticles(2).SetExplosionResistance(0.05).SetWaterBreak(1).SetWeight(0.1f).SetValue(10.0f).SetIsCrushable(1).SetFloatingAction(2).SetIsSupportive(0);
        //}
    }
    file.close();

    if (loadFloraNoiseFunctions((worldFilePath + "/Noise/FloraNoise.SOANOISE").c_str(), planet)) {
        exit(17);
    }


    return 0;
}
i32 FileManager::loadAllTreeData(Planet *planet, nString worldFilePath) {
    printf("Loading TreeData...\n");
    TreeType *tree = NULL;
    std::ifstream file;
    int minSize, r, g, b;
    size_t i;
    nString s, name, fileName;
    char buffer[512];
    std::vector <nString> fileNameVec;

    //********************************************* TREE SPECIFICATIONS ************************************************

    file.open(worldFilePath + "Trees/TreeData.txt");
    if (file.fail()) {
        pError((worldFilePath + "Trees/TreeData.txt" + " could not be opened for read").c_str());
        file.close();
        return 1;
    }
    //make sure constants are ordered from 0->n

    i = 0;
    planet->treeTypeVec.clear();
    planet->treeLookupMap.clear();

    while (file >> s) {
        if (s[0] == '#') {
            file.getline(buffer, 512);
            continue;
        }
        name = s;

        if (!(file >> fileName)) {
            printf("ERROR: (TreeData.txt) Could not read tree fileName for %s!\n", name.c_str());
            int a;
            std::cin >> a;
        }
        fileName = worldFilePath + "Trees/TreeTypes/" + fileName;
        tree = new TreeType;
        tree->name = name;
        planet->treeTypeVec.push_back(tree);
        fileNameVec.push_back(fileName);

        planet->treeLookupMap.insert(make_pair(name, i));
        i++;

    }
    file.close();
    file.clear();

    //******************************************** TREE FILES *********************************
    for (i = 0; i < fileNameVec.size(); i++) {
        if (loadTreeType(fileNameVec[i], planet->treeTypeVec[i])) {
            return 1;
        }
    }
    //********************************************* LEAVES *************************************

    file.open((worldFilePath + "Trees/Leaves.txt").c_str());
    if (file.fail()) {
        perror((worldFilePath + "Trees/Leaves.txt").c_str());
        std::cin >> minSize;
        file.close();
        return 1;
    }

    i = 0;
    int lblock = LEAVES1;
    bool first = 1;
    while (file >> s) {
        if (s[0] == '#') {
            file.getline(buffer, 512);
            continue;
        }
        sscanf(&(s[0]), "%d", &r);
        file >> g >> b;

        if (first) {
            Blocks[lblock].altColors.clear();
            Blocks[lblock].color = ColorRGB8(r, g, b);
            first = 0;
        }
        Blocks[lblock].altColors.emplace_back(r, g, b);
        i++;
        if (i == 15) {
            i = 0;
            first = 1;
            lblock++;
        }
    }
    file.close();
    return 0;
}
i32 FileManager::loadTreeType(nString filePath, TreeType *tree) {
    // TODO: This Should Be An Argument
    IOManager iom;

    nString data;
    iom.readFileToString(filePath.c_str(), data);
    if (data.empty()) return 1;

    return Keg::parse(tree, data.c_str(), "TreeType") == Keg::Error::NONE ? 0 : 1;
}

i32 FileManager::loadBiomeData(Planet *planet, nString worldFilePath) {
    std::ifstream file;
    int mapColor, activeList = 0;
    char buffer[512];
    nString bfname;
    Biome *biome;

    vg::TextureCache* textureCache = GameManager::textureCache;


    nString ts, s;
    file.open(worldFilePath + "Biomes/BiomeDistribution/BiomeList.txt");
    if (file.fail()) {
        pError((worldFilePath + "Biomes/BiomeDistribution/BiomeList.txt" + " could not be opened for read").c_str());
        perror((worldFilePath + "Biomes/BiomeDistribution/BiomeList.txt").c_str());
        file.close();
        return 1;
    }
    while (file >> ts) {

        if (ts[0] == '#') {
            file.getline(buffer, 512);
            continue;
        }
        if (ts == "Biome_Map_Filename:") {
            if (!(file >> s)) {
                printf("    ERROR: World/Biomes/BiomeDistribution/BiomeList.txt trouble reading Biome_Map_Filename!\nEnter any key to continue: ");
                std::cin >> s;
                file.close();
                return 1;
            }
            planet->biomeMapFileName = s;
            planet->biomeMapTexture = textureCache->addTexture(worldFilePath + "Biomes/BiomeDistribution/" + s, &SamplerState::LINEAR_CLAMP_MIPMAP);
        } else if (ts == "Biome_Color_Filename:") {
            if (!(file >> s)) {
                std::cout << "    ERROR: " + worldFilePath + "Biomes/BiomeDistribution/BiomeList.txt trouble reading Biome_Color_Filename!\nEnter any key to continue: ";
                std::cin >> s;
                file.close();
                return 1;
            }
            planet->colorMapFileName = s;
            planet->colorMapTexture = textureCache->addTexture(worldFilePath + "Biomes/BiomeDistribution/" + s, &SamplerState::LINEAR_CLAMP_MIPMAP);
        } else if (ts == "Water_Color_Filename:") {
            if (!(file >> s)) {
                std::cout << "    ERROR: " + worldFilePath + "Biomes/BiomeDistribution/BiomeList.txt trouble reading Water_Color_Filename!\nEnter any key to continue: ";
                std::cin >> s;
                file.close();
                return 1;
            }
            planet->waterColorMapFileName = s;
            planet->waterColorMapTexture = textureCache->addTexture(worldFilePath + "Biomes/BiomeDistribution/" + s, &SamplerState::LINEAR_CLAMP_MIPMAP);
        } else if (ts == "Base_Biomes:") {
            activeList = 1;
        } else if (ts == "Main_Biomes:") {
            activeList = 2;
        } else {
            if (!(file >> bfname)) {
                std::cout << "    ERROR: " + worldFilePath + "Biomes/BiomeDistribution/BiomeList.txt format error!\nEnter any key to continue: ";
                std::cin >> s;
                file.close();
                return 1;
            }

            biome = new Biome;
            biome->name = ts;
            if (activeList == 1) {
                biome->isBase = 1;

                if (!(file >> s)) {
                    printf("    ERROR: %s trouble reading mapColor!\nEnter any key to continue: ", bfname.c_str());
                    std::cin >> s;
                    file.close();
                    return 1;
                }
                sscanf(&(s[0]), "%x", &mapColor);

                if (readBiome(biome, worldFilePath + "Biomes/" + bfname, planet, worldFilePath)) {
                    file.close();
                    return 1;
                }

                planet->addBaseBiome(biome, mapColor);
            } else if (activeList == 2) {
                biome->isBase = 0;

                if (readBiome(biome, worldFilePath + "Biomes/" + bfname, planet, worldFilePath)) {
                    file.close();
                    return 1;
                }
                planet->addMainBiome(biome);
            } else {
                printf("    FORMAT ERROR: %sBiomes/BiomeDistribution/BiomeList.txt) Line: %s \nEnter any key to continue: ", worldFilePath, bfname.c_str());
                std::cin >> s;
                file.close();
                return 1;
            }
        }
    }

    file.close();
    return 0;
}
i32 FileManager::saveBiomeData(Planet *planet, nString worldFilePath) {
    std::vector <std::vector <IniValue> > iniValues;
    std::vector <nString> iniSections;

    iniSections.push_back("");
    iniValues.push_back(std::vector<IniValue>());

    Biome *biome;
    unsigned int colorCode;
    nString hexCode("000000");
    for (auto i = planet->baseBiomesLookupMap.begin(); i != planet->baseBiomesLookupMap.end(); i++) {
        biome = i->second;
        colorCode = i->first;

        iniSections.push_back(biome->name);

        iniValues.push_back(std::vector<IniValue>());
        sprintf(&(hexCode[0]), "%06x", colorCode);
        iniValues.back().push_back(IniValue("colorCode", hexCode));
        iniValues.back().push_back(IniValue("childBiomeMap", "NULL"));
        iniValues.back().push_back(IniValue("fileName", biome->filename));
    }

    if (saveIniFile(worldFilePath + "Biomes/BiomeDistribution/BaseBiomeList.ini", iniValues, iniSections)) return 0;

    return 1;
}

i32 FileManager::readBiome(Biome* biome, nString fileName, Planet* planet, nString worldFilePath) {
    nString ts;
    nString childFileName;
    std::ifstream biomeFile;
    int activeList;
    double d;
    char buffer[512];
    Biome *childBiome;
    std::map<nString, int>::iterator tit;
    std::vector<nString> childFileNames;

    biome->possibleFlora.clear();
    biome->possibleTrees.clear();
    biome->terrainNoiseList.clear();

    biomeFile.open((fileName).c_str());
    if (biomeFile.fail()) {
        pError((fileName + " could not be opened for read").c_str());
        perror((fileName).c_str());
        return 1;
    }
    biome->filename = fileName;
    while (biomeFile >> ts) {
        if (ts[0] == '#') {
            biomeFile.getline(buffer, 512);
            continue;
        }
        if (ts == "Surface_Block_ID:") {
            biomeFile >> biome->surfaceBlock;
        } else if (ts == "Base_Tree_Chance:") {
            biomeFile >> biome->treeChance;
        } else if (ts == "Tree_List:") {
            activeList = 1;
        } else if (ts == "Flora_List:") {
            activeList = 2;
        } else if (ts == "Child_Biomes:") {
            activeList = 3;
        } else if (ts == "Unique_Terrain_Noise:") {
            activeList = 4;
        } else if (ts == "Distribution_Noise:") {
            activeList = 5;
        } else if (ts == "Underwater_Block_ID:") {
            if (!(biomeFile >> biome->underwaterBlock)) {
                printf("    ERROR: %s trouble reading Underwater_Block_ID!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Beach_Block_ID:") {
            if (!(biomeFile >> biome->beachBlock)) {
                printf("    ERROR: %s trouble reading Beach_Block_ID!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Apply_Biome_At:") {
            if (!(biomeFile >> biome->applyBiomeAt)) {
                printf("    ERROR: %s trouble reading Apply_Biome_At!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Min_Terrain_Mult:") {
            if (!(biomeFile >> biome->minTerrainMult)) {
                printf("    ERROR: %s trouble reading Min_Terrain_Mult!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Min_Temp:") {
            if (!(biomeFile >> biome->lowTemp)) {
                printf("    ERROR: %s trouble reading Min_Temp!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Max_Temp:") {
            if (!(biomeFile >> biome->highTemp)) {
                printf("    ERROR: %s trouble reading Max_Temp!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Temp_Slope_Length:") {
            if (!(biomeFile >> biome->tempSlopeLength)) {
                printf("    ERROR: %s trouble reading Temp_Slope_Length!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Min_Rain:") {
            if (!(biomeFile >> biome->lowRain)) {
                printf("    ERROR: %s trouble reading Min_Rain!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Max_Rain:") {
            if (!(biomeFile >> biome->highRain)) {
                printf("    ERROR: %s trouble reading Max_Rain!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Max_Height:") {
            if (!(biomeFile >> biome->maxHeight)) {
                printf("    ERROR: %s trouble reading Max_Height!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Max_Height_Slope_Length:") {
            if (!(biomeFile >> biome->maxHeightSlopeLength)) {
                printf("    ERROR: %s trouble reading Max_Height_Slope_Length!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Rain_Slope_Length:") {
            if (!(biomeFile >> biome->rainSlopeLength)) {
                printf("    ERROR: %s trouble reading Rain_Slope_Length!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
        } else if (ts == "Terrain_Color:") {
            if (!(biomeFile >> biome->hasAltColor)) {
                printf("    ERROR: %s trouble reading active!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
            int a, b, c;
            if (!(biomeFile >> a >> b >> c)) {
                printf("    ERROR: %s trouble reading terrain color!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
            biome->r = a;
            biome->g = b;
            biome->b = c;
        } else if (activeList == 1) { //tree types
            if (!(biomeFile >> d)) {
                printf("    ERROR: %s trouble reading tree probability!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
            tit = planet->treeLookupMap.find(ts);
            if (tit == planet->treeLookupMap.end()) {
                printf("ERROR: (%s) Tree type %s not found in TreeData.txt\n", fileName.c_str(), ts.c_str());
                printf("Enter any key to continue... ");
                std::cin >> ts;
            } else {
                biome->possibleTrees.push_back(BiomeTree(d, tit->second));
            }
        } else if (activeList == 2) { //flora types
            if (!(biomeFile >> d)) {
                printf("    ERROR: %s trouble reading flora probability!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
            tit = planet->floraLookupMap.find(ts);
            if (tit == planet->floraLookupMap.end()) {
                printf("ERROR: (%s) Flora type %s not found in FloraData.txt\n", fileName.c_str(), ts.c_str());
                printf("Enter any key to continue... ");
                std::cin >> ts;
            } else {
                biome->possibleFlora.push_back(BiomeFlora(d, tit->second));
            }
        } else if (activeList == 3) {//child biomes
            childBiome = new Biome;
            childBiome->isBase = 0;
            childBiome->name = ts;
            if (!(biomeFile >> childFileName)) {
                printf("    ERROR: %s trouble reading child filename!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
            childFileName = worldFilePath + "Biomes/" + childFileName;
            childFileNames.push_back(childFileName);
            biome->childBiomes.push_back(childBiome);
            planet->addChildBiome(childBiome);
        } else if (activeList == 4) { //Unique Terrain Noise
            NoiseInfo noisef;
            if (sscanf(&(ts[0]), "%d", &(noisef.type)) == 0 ||
                !(biomeFile >> noisef.persistence >> noisef.frequency >> noisef.octaves >> noisef.lowBound >> noisef.upBound >> noisef.scale >> noisef.composition)) {
                printf("    ERROR: %s trouble reading unique terrain noise!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
            biome->terrainNoiseList.push_back(noisef);
        } else if (activeList == 5) { //distribution noise
            if (sscanf(&(ts[0]), "%lf", &(biome->distributionNoise.persistence)) == 0 ||
                !(biomeFile >> biome->distributionNoise.frequency >> biome->distributionNoise.octaves >>
                biome->distributionNoise.lowBound >> biome->distributionNoise.upBound)) {
                printf("    ERROR: %s trouble reading Distribution_Noise!\nEnter any key to continue: ", fileName.c_str());
                std::cin >> ts;
                biomeFile.close();
                return 1;
            }
            biome->distributionNoise.lowBound = 1.0 - biome->distributionNoise.lowBound;
        }
    }
    biomeFile.close();
    biomeFile.close();

    for (size_t i = 0; i < childFileNames.size(); i++) {
        if (readBiome(biome->childBiomes[i], childFileNames[i], planet, worldFilePath)) {  //recursive biome read
            return 1;
        }
    }
    return 0;
}
i32 FileManager::saveBiome(Biome *biome) {
    std::ofstream file;
    std::map <nString, BiomeVariable>::iterator bit;
    NoiseInfo *np;

    if (biome->filename.size() < 4) return 1;
    nString newFileName = biome->filename.substr(0, biome->filename.size() - 3) + "ini";
    file.open(newFileName.c_str());
    if (file.fail()) {
        perror((newFileName).c_str());
        int a;
        std::cin >> a;
        return 0;
    }

    int byteOff;
    file << "4324F36F\n";
    file << "Name= " << biome->name << "\n";
    for (bit = biomeVariableMap.begin(); bit != biomeVariableMap.end(); bit++) {
        byteOff = bit->second.byteOffset;
        if (bit->second.varType == 0) { //integers
            file << bit->first << "= " << (int)*(GLuint *)((Uint32)biome + (Uint32)byteOff);
            if (bit->second.byteOffset2 != -1) {
                byteOff = bit->second.byteOffset2;
                file << "," << (int)*(GLuint *)((Uint32)biome + (Uint32)byteOff);
            }
        } else if (bit->second.varType == 1) { //floats
            file << bit->first << "= " << (float)*(GLfloat *)((Uint32)biome + (Uint32)byteOff);
            if (bit->second.byteOffset2 != -1) {
                byteOff = bit->second.byteOffset2;
                file << "," << (float)*(GLfloat *)((Uint32)biome + (Uint32)byteOff);
            }
        } else if (bit->second.varType == 2) { //ubytes
            file << bit->first << "= " << (int)*(GLubyte *)((Uint32)biome + (Uint32)byteOff);
            if (bit->second.byteOffset2 != -1) {
                byteOff = bit->second.byteOffset2;
                file << "," << (int)*(GLubyte *)((Uint32)biome + (Uint32)byteOff);
            }
        } else if (bit->second.varType == 3) { //ushorts
            file << bit->first << "= " << (int)*(GLushort *)((Uint32)biome + (Uint32)byteOff);
            if (bit->second.byteOffset2 != -1) {
                byteOff = bit->second.byteOffset2;
                file << "," << (int)*(GLushort *)((Uint32)biome + (Uint32)byteOff);
            }
        }
        file << "\n";
    }
    file << "DistributionNoise= ";
    file << biome->distributionNoise.type << " " << biome->distributionNoise.persistence << " " << biome->distributionNoise.frequency
        << " " << biome->distributionNoise.octaves << " " << biome->distributionNoise.lowBound << " " << biome->distributionNoise.upBound
        << " " << biome->distributionNoise.scale << " " << biome->distributionNoise.composition << " " << biome->distributionNoise.name << "\n";
    file << "[Trees]\n";
    for (size_t i = 0; i < biome->possibleTrees.size(); i++) {
        file << "PossibleTree" << i << "= " << GameManager::planet->treeTypeVec[biome->possibleTrees[i].treeIndex]->name << " "
            << biome->possibleTrees[i].probability << "\n";
    }
    file << "[Flora]\n";
    int findex;
    for (size_t i = 0; i < biome->possibleFlora.size(); i++) {
        findex = biome->possibleFlora[i].floraIndex;
        std::cout << findex << std::endl;
        fflush(stdout);
        file << "PossibleFlora" << i << "= " << GameManager::planet->floraTypeVec[findex]->name << " " << biome->possibleFlora[i].probability;
        if (GameManager::planet->floraNoiseFunctions[findex]) {
            file << " " << GameManager::planet->floraNoiseFunctions[findex]->persistence
                << " " << GameManager::planet->floraNoiseFunctions[findex]->frequency << " " << GameManager::planet->floraNoiseFunctions[findex]->octaves
                << " " << GameManager::planet->floraNoiseFunctions[findex]->lowBound << " " << GameManager::planet->floraNoiseFunctions[findex]->upBound << " " << GameManager::planet->floraNoiseFunctions[findex]->name << "\n";
        }
    }
    file << "[ChildBiomes]\n";
    for (size_t i = 0; i < biome->childBiomes.size(); i++) {
        file << biome->childBiomes[i]->name << " " << biome->childBiomes[i]->filename << "\n";
    }
    file << "[UniqueTerrainNoise]\n";
    for (size_t i = 0; i < biome->terrainNoiseList.size(); i++) {
        np = &(biome->terrainNoiseList[i]);
        file << "TerrainNoise" << i << "= " << np->type << " " << np->persistence << " " << np->frequency
            << " " << np->octaves << " " << np->lowBound << " " << biome->terrainNoiseList[i].upBound
            << " " << np->scale << " " << np->composition << " " << np->name << "\n";

        if (biome->terrainNoiseList[i].modifier != NULL) {
            np = biome->terrainNoiseList[i].modifier;
            file << "Modifier" << i << "= " << np->type << " " << np->persistence << " " << np->frequency
                << " " << np->octaves << " " << np->lowBound << " " << biome->terrainNoiseList[i].upBound
                << " " << np->scale << " " << np->composition << " " << np->name << "\n";
        }
    }

    file.close();
}
void FileManager::saveAllBiomes(Planet *planet) {
    for (size_t i = 0; i < planet->allBiomesLookupVector.size(); i++) {
        saveBiome(planet->allBiomesLookupVector[i]);
    }
}

i32 FileManager::saveTreeData(TreeType *tt) {
#define STRCOMB(a,b) (std::to_string(a) + "," + std::to_string(b))
    std::vector <std::vector <IniValue> > iniValues;
    std::vector <nString> iniSections;

    iniSections.push_back("");
    iniValues.push_back(std::vector<IniValue>());
    std::vector<IniValue> &ivs = iniValues.back();

    ivs.push_back(IniValue("name", tt->name));
    ivs.push_back(IniValue("branchChanceBottom", STRCOMB(tt->branchingPropsBottom.chance.min, tt->branchingPropsBottom.chance.max)));
    ivs.push_back(IniValue("branchChanceTop", STRCOMB(tt->branchingPropsTop.chance.min, tt->branchingPropsTop.chance.max)));
    ivs.push_back(IniValue("branchChanceCapMod", tt->capBranchChanceMod));
    ivs.push_back(IniValue("branchDirBottom", tt->branchingPropsBottom.direction));
    ivs.push_back(IniValue("branchDirTop", tt->branchingPropsTop.direction));
    ivs.push_back(IniValue("branchLengthBottom", STRCOMB(tt->branchingPropsBottom.length.min, tt->branchingPropsBottom.length.max)));
    ivs.push_back(IniValue("branchWidthBottom", STRCOMB(tt->branchingPropsBottom.width.min, tt->branchingPropsBottom.width.max)));
    ivs.push_back(IniValue("branchLeafShape", (i32)tt->branchLeafShape));
    ivs.push_back(IniValue("branchLeafSizeMod", tt->branchLeafSizeMod));
    ivs.push_back(IniValue("branchLeafYMod", tt->branchLeafYMod));
    ivs.push_back(IniValue("branchStart", tt->branchStart));
    ivs.push_back(IniValue("idCore", tt->idCore));
    ivs.push_back(IniValue("droopyLeavesLength", STRCOMB(tt->droopyLength.min, tt->droopyLength.max)));
    ivs.push_back(IniValue("droopyLeavesSlope", tt->droopyLeavesSlope));
    ivs.push_back(IniValue("droopyLeavesDSlope", tt->droopyLeavesDSlope));
    ivs.push_back(IniValue("droopyLeavesActive", tt->hasDroopyLeaves));
    ivs.push_back(IniValue("hasThickCapBranches", tt->hasThickCapBranches));
    ivs.push_back(IniValue("mushroomCapInverted", tt->isMushroomCapInverted));
    ivs.push_back(IniValue("isSlopeRandom", tt->isSlopeRandom));
    ivs.push_back(IniValue("leafCapShape", (i32)tt->leafCapShape));
    ivs.push_back(IniValue("leafCapSize", STRCOMB(tt->leafCapSize.min, tt->leafCapSize.max)));
    ivs.push_back(IniValue("idLeaves", tt->idLeaves));
    ivs.push_back(IniValue("mushroomCapCurlLength", tt->mushroomCapCurlLength));
    ivs.push_back(IniValue("mushroomCapGillThickness", tt->mushroomCapGillThickness));
    ivs.push_back(IniValue("mushroomCapStretchMod", tt->mushroomCapLengthMod));
    ivs.push_back(IniValue("mushroomCapThickness", tt->mushroomCapThickness));
    ivs.push_back(IniValue("idBark", tt->idOuter));
    ivs.push_back(IniValue("idRoot", tt->idRoot));
    ivs.push_back(IniValue("idRoot", tt->idRoot));
    ivs.push_back(IniValue("idSpecialBlock", tt->idSpecial));
    ivs.push_back(IniValue("branchLengthTop", STRCOMB(tt->branchingPropsTop.length.min, tt->branchingPropsTop.length.max)));
    ivs.push_back(IniValue("branchWidthTop", STRCOMB(tt->branchingPropsTop.width.min, tt->branchingPropsTop.width.max)));
    ivs.push_back(IniValue("trunkHeightBase", STRCOMB(tt->trunkBaseHeight.min, tt->trunkBaseHeight.max)));
    ivs.push_back(IniValue("trunkWidthBase", STRCOMB(tt->trunkBaseWidth.min, tt->trunkBaseWidth.max)));
    ivs.push_back(IniValue("trunkChangeDirChance", tt->trunkTopWidth.min));
    ivs.push_back(IniValue("trunkCoreWidth", tt->coreWidth));
    ivs.push_back(IniValue("trunkSlopeEnd", STRCOMB(tt->trunkEndSlope.min, tt->trunkEndSlope.max)));
    ivs.push_back(IniValue("trunkHeight", STRCOMB(tt->trunkHeight.min, tt->trunkHeight.max)));
    ivs.push_back(IniValue("trunkWidthMid", STRCOMB(tt->trunkMidWidth.min, tt->trunkMidWidth.max)));
    ivs.push_back(IniValue("trunkWidthTop", STRCOMB(tt->trunkTopWidth.min, tt->trunkTopWidth.min)));
    ivs.push_back(IniValue("trunkSlopeStart", STRCOMB(tt->trunkStartSlope.min, tt->trunkStartSlope.min)));

    if (fileManager.saveIniFile(tt->fileName.substr(0, tt->fileName.size() - 1), iniValues, iniSections)) return 1;
    return 0;
}
void FileManager::saveAllTreeFiles(Planet *planet) {
    for (size_t i = 0; i < planet->treeTypeVec.size(); i++) {
        if (!saveTreeData(planet->treeTypeVec[i])) exit(4);
    }
}

nString FileManager::getFileNameDialog(const nString &prompt, const char *initialDir) {
    // TODO: Probably Not Necessary, Either Way Bad Code

#if 0
    //#ifdef _WIN32
    //TODO: This sucks
    nString initdir = getFullPath(initialDir); //need to maintain working directory
    char pathBuffer[1024];
    const int BUFSIZE = 1024;
    char buffer[BUFSIZE] = { 0 };
    HRESULT hr = S_OK;


#define MY_BUFSIZE 1024 // Buffer size for console window titles.
    HWND hwndFound;         // This is what is returned to the caller.
    char pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
    // WindowTitle.
    char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
    // WindowTitle.

    // Fetch current window title.

    GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

    // Format a "unique" NewWindowTitle.

    wsprintf(pszNewWindowTitle, "%d/%d",
        GetTickCount(),
        GetCurrentProcessId());

    // Change current window title.

    SetConsoleTitle(pszNewWindowTitle);

    // Ensure window title has been updated.

    Sleep(40);

    // Look for NewWindowTitle.

    hwndFound = FindWindow(NULL, pszNewWindowTitle);

    // Restore original window title.

    SetConsoleTitle(pszOldWindowTitle);

    HWND hwnd = hwndFound;


    // Create a new common open file dialog. 
    IFileDialog *pfd = NULL;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        // Control the default folder of the file dialog. Here we set it as the  
        // Music library known folder.  
        IShellItem *psiMusic = NULL;
        hr = SHCreateItemInKnownFolder(FOLDERID_Music, 0, NULL,
            IID_PPV_ARGS(&psiMusic));
        if (SUCCEEDED(hr)) {
            hr = pfd->SetFolder(psiMusic);
            psiMusic->Release();
        }

        // Set the title of the dialog. 
        if (SUCCEEDED(hr)) {
            hr = pfd->SetTitle(L"Select a File");
        }

        // Specify file types for the file dialog. 
        if (SUCCEEDED(hr)) {
            hr = pfd->SetFileTypes(ARRAYSIZE(c_rgFileTypes), c_rgFileTypes);
            if (SUCCEEDED(hr)) {
                // Set the selected file type index to Word Document. 
                hr = pfd->SetFileTypeIndex(1);
            }
        }

        // Set the default extension to be added to file names as ".docx" 
        if (SUCCEEDED(hr)) {
            hr = pfd->SetDefaultExtension(L"docx");
        }

        // Show the open file dialog. 
        if (SUCCEEDED(hr)) {
            hr = pfd->Show(hwnd);
            if (SUCCEEDED(hr)) {
                // Get the result of the open file dialog. 
                IShellItem *psiResult = NULL;
                hr = pfd->GetResult(&psiResult);
                if (SUCCEEDED(hr)) {
                    PWSTR pszPath = NULL;
                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                    if (SUCCEEDED(hr)) {
                        // MessageBox(NULL, pszPath, L"The selected file is", MB_OK);
                        CoTaskMemFree(pszPath);
                    }
                    psiResult->Release();
                }
            } else {
                if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
                    // User cancelled the dialog... 
                }
            }
        }

        pfd->Release();
    }

    // Report the error.  
    if (FAILED(hr)) {
        // If it's not that the user cancelled the dialog, report the error in a  
        // message box. 
        if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {

        }
    }

    _chdir(initdir.c_str()); //set dir back to initial dir

    return buffer;
    cout << "ERROR! FILE DIALOG NOT IMPLEMENTED FOR THIS FILE SYSTEM\n";
    int a;
    cin >> a;
#endif

    return "";
}

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

void FileManager::loadNoiseDescriptions(const char *filename) {
    std::ifstream file;
    nString s;
    char buffer[1024];
    int i = 0;
    int index = -1;


    file.open(filename);
    if (file.fail()) {
        showMessage("Could not load noise description file.");
        perror(filename);
        return;
    }

    while (file.getline(buffer, 1024)) {
        if (sscanf(buffer, "[%d]", &i) == 1) {
            index = i;
            TerrainFunctionHelps[index] = "";
        } else if (index != -1) {
            TerrainFunctionHelps[index] += buffer;
            TerrainFunctionHelps[index] += "\n";
        }
    }

    file.close();
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
    std::ofstream file(filePath + "world.txt");
    if (file.fail()) {
        perror((filePath + "world.txt").c_str());
        return 1;
    }
    file << menuOptions.selectPlanetName << std::endl;
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

i32 FileManager::setSaveFile(nString filePath) {
    struct stat statbuf;

    if (stat(filePath.c_str(), &statbuf) != 0) {
        pError("Save file does not exist.");
        return 1;
    }

    GameManager::saveFilePath = filePath;
    return 0;
}

i32 FileManager::loadMarkers(Player *player) {


    std::ifstream f(GameManager::saveFilePath + "/Data/" + player->getName() + "_markers.ini");  // New enough C++ library will accept just name
    if (f.is_open() == 0) {
        f.close();
        return 0;
    }
    f.close();

    std::vector <std::vector <IniValue> > iniValues;
    std::vector <nString> iniSections;
    if (loadIniFile(GameManager::saveFilePath + "/Data/" + player->getName() + "_markers.ini", iniValues, iniSections)) return 1;

    int iVal;
    IniValue *iniVal;
    for (size_t i = 1; i < iniSections.size(); i++) {
        glm::dvec3 pos;
        glm::vec3 color;
        for (size_t j = 0; j < iniValues[i].size(); j++) {
            iniVal = &(iniValues[i][j]);

            iVal = getIniVal(iniVal->key);
            switch (iVal) {
            case INI_X:
                pos[0] = iniVal->getDouble();
                break;
            case INI_Y:
                pos[1] = iniVal->getDouble();
                break;
            case INI_Z:
                pos[2] = iniVal->getDouble();
                break;
            case INI_R:
                color[0] = iniVal->getFloat();
                break;
            case INI_G:
                color[1] = iniVal->getFloat();
                break;
            case INI_B:
                color[2] = iniVal->getFloat();
                break;
            }
        }
        GameManager::markers.push_back(Marker(pos, iniSections[i], color));
    }
    return 0;
}
i32 FileManager::loadPlayerFile(Player *player) {
    loadMarkers(player);

    FILE *file = NULL;
    file = fopen((GameManager::saveFilePath + "/Players/" + player->getName() + ".dat").c_str(), "rb");
    if (file == NULL) {
        //file doesnt exist so set spawn to random
        srand(time(NULL));
        int spawnFace = rand() % 4 + 1;
        player->voxelMapData.face = spawnFace;
        return 0;
    }

    GLubyte buffer[2048];
    int n;
    n = fread(buffer, 1, 1024, file);

    int byte = 0;
    player->facePosition.x = BufferUtils::extractFloat(buffer, (byte++) * 4);
    player->facePosition.y = BufferUtils::extractFloat(buffer, (byte++) * 4);
    player->facePosition.z = BufferUtils::extractFloat(buffer, (byte++) * 4);
    player->voxelMapData.face = BufferUtils::extractInt(buffer, (byte++) * 4);
    player->getChunkCamera().setYawAngle(BufferUtils::extractFloat(buffer, (byte++) * 4));
    player->getChunkCamera().setPitchAngle(BufferUtils::extractFloat(buffer, (byte++) * 4));
    player->isFlying = BufferUtils::extractBool(buffer, byte * 4);
    fclose(file);

    player->voxelMapData.ipos = fastFloor(player->facePosition.z / (double)CHUNK_WIDTH);
    player->voxelMapData.jpos = fastFloor(player->facePosition.x / (double)CHUNK_WIDTH);
    return 1;
}
i32 FileManager::saveMarkers(Player *player) {

    std::vector <std::vector <IniValue> > iniValues;
    std::vector <nString> iniSections;
    iniSections.push_back("");
    iniValues.push_back(std::vector<IniValue>());

    Marker *m;
    for (size_t i = 0; i < GameManager::markers.size(); i++) {
        m = &(GameManager::markers[i]);
        iniSections.push_back(m->name);
        iniValues.push_back(std::vector<IniValue>());
        iniValues.back().push_back(IniValue("r", std::to_string(m->color.r)));
        iniValues.back().push_back(IniValue("g", std::to_string(m->color.g)));
        iniValues.back().push_back(IniValue("b", std::to_string(m->color.b)));
        iniValues.back().push_back(IniValue("x", std::to_string(m->pos.x)));
        iniValues.back().push_back(IniValue("y", std::to_string(m->pos.y)));
        iniValues.back().push_back(IniValue("z", std::to_string(m->pos.z)));
    }

    if (saveIniFile(GameManager::saveFilePath + "/Data/" + player->getName() + "_markers.ini", iniValues, iniSections)) return 1;

}
i32 FileManager::savePlayerFile(Player *player) {
    saveMarkers(player);

    FILE *file = NULL;
    file = fopen((GameManager::saveFilePath + "/Players/" + player->getName() + ".dat").c_str(), "wb");
    if (file == NULL) {
        pError("Failed to open player .dat file for writing!");
        return 0; //uhhh idk 
    }

    GLubyte buffer[2048];

    int byte = 0;

    int rot = player->voxelMapData.rotation;
    int face = player->voxelMapData.face;

    float facex = player->facePosition.x;
    float facey = player->facePosition.y;
    float facez = player->facePosition.z;

    if (vvox::FaceSigns[face][rot][0] != vvox::FaceSigns[face][0][0]) {
        facez = -facez;
    }
    if (vvox::FaceSigns[face][rot][1] != vvox::FaceSigns[face][0][1]) {
        facex = -facex;
    }

    if (rot % 2) {
        int tmp = facex;
        facex = facez;
        facez = tmp;
    }

    byte += BufferUtils::setFloat(buffer, byte, facex);
    byte += BufferUtils::setFloat(buffer, byte, facey);
    byte += BufferUtils::setFloat(buffer, byte, facez);
    byte += BufferUtils::setInt(buffer, byte, face);
    byte += BufferUtils::setFloat(buffer, byte, player->getChunkCamera().getYawAngle());
    byte += BufferUtils::setFloat(buffer, byte, player->getChunkCamera().getPitchAngle());
    byte += BufferUtils::setBool(buffer, byte, player->isFlying);

    fwrite(buffer, 1, byte, file);

    fclose(file);
    return 1;
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
i32 FileManager::loadIniFile(nString filePath, std::vector<std::vector<IniValue>> &iniValues, std::vector <nString> &iniSections, ZipFile *zipFile) {
    char buffer[1024];
    int n;
    nString currentName = "";
    nString s, s2;
    std::ifstream file;
    int currSection = 0;

    iniSections.push_back("");
    iniValues.push_back(std::vector <IniValue>());

    if (filePath.length() == 0) return 1;
    size_t zipsize;
    if (zipFile != NULL) {
        unsigned char *data = zipFile->readFile(filePath, zipsize);
        size_t bufferIndex = 0;

        while (bufferIndex < zipsize) {
            int size = 1000;
            readLine(data, buffer, size);

            bufferIndex += size; //number of characters read

            if (buffer[0] == '#' || buffer[0] == ';') continue;
            if (strlen(buffer) <= 2) continue;

            s.clear();
            if (buffer[0] == '[') { //grab the name
                for (n = 1; buffer[n] != ']' && buffer[n] != '\0'; n++) s += buffer[n];
                currentName = s;
                iniSections.push_back(currentName);
                iniValues.push_back(std::vector <IniValue>());
                currSection++;
                continue;
            }

            for (n = 0; buffer[n] == ' ' || buffer[n] == '\t'; n++); //ignore spaces and tabs
            for (n; buffer[n] != '=' && buffer[n] != ' '; n++) s += buffer[n]; //grab the nString name
            for (n; buffer[n] == '=' || buffer[n] == ' ' || buffer[n] == '\t'; n++); //ignore spaces, equals, and tabs


            iniValues[currSection].push_back(IniValue());
            iniValues[currSection].back().key = s;


            s2.clear();
            for (n; buffer[n] != '\n' && buffer[n] != '\0'; n++) s2 += buffer[n]; //grab the nString value
            iniValues[currSection].back().val = s2;
        }

    } else {

        file.open((filePath).c_str());
        if (file.fail()) {
            //    pError((filePath + " could not be opened for read").c_str());
            //    perror((filePath).c_str());
            return 1;
        }

        while (file.getline(buffer, 1024)) {
            if (buffer[0] == '#' || buffer[0] == ';') continue;
            if (strlen(buffer) <= 2) continue;

            s.clear();
            if (buffer[0] == '[') { //grab the name
                for (n = 1; buffer[n] != ']' && buffer[n] != '\0'; n++) s += buffer[n];
                currentName = s;
                iniSections.push_back(currentName);
                iniValues.push_back(std::vector <IniValue>());
                currSection++;
                continue;
            }

            for (n = 0; buffer[n] == ' ' || buffer[n] == '\t'; n++); //ignore spaces and tabs
            for (n; buffer[n] != '=' && buffer[n] != ' '; n++) s += buffer[n]; //grab the nString name
            for (n; buffer[n] == '=' || buffer[n] == ' ' || buffer[n] == '\t'; n++); //ignore spaces, equals, and tabs

            iniValues[currSection].push_back(IniValue());
            iniValues[currSection].back().key = s;

            s2.clear();
            for (n; buffer[n] != '\n' && buffer[n] != '\0'; n++) s2 += buffer[n]; //grab the nString value
            while (s2.back() == ' ') s2.pop_back();
            iniValues[currSection].back().val = s2;
        }
    }

    return 0;
}
i32 FileManager::saveIniFile(nString filePath, std::vector <std::vector <IniValue> > &iniValues, std::vector <nString> &iniSections) {
    std::ofstream file;
    file.open(filePath.c_str());
    if (file.fail()) {
        pError((filePath + " could not be opened for write").c_str());
        perror((filePath).c_str());
        return 1;
    }

    for (size_t i = 0; i < iniSections.size(); i++) {
        if (i != 0) {
            file << "[" + iniSections[i] + "]\n";
        }

        for (size_t j = 0; j < iniValues[i].size(); j++) {
            if (iniValues[i][j].isFloat) {
                file << iniValues[i][j].key << "= " << iniValues[i][j].fval << "\n";
            } else {
                file << iniValues[i][j].key << "= " << iniValues[i][j].val << "\n";
            }
        }
    }
    file.close();
    return 0;
}

ParticleEmitter* FileManager::loadEmitter(nString fileName) {
    auto it = _emitterMap.find(fileName);
    if (it != _emitterMap.end()) {
        return it->second;
    } else {
        std::vector <std::vector <IniValue> > iniValues;
        std::vector <nString> iniSections;
        if (loadIniFile(fileName, iniValues, iniSections)) return NULL;
        ParticleEmitter* e = new ParticleEmitter();
        nString s;
        int iVal;
        IniValue *iniVal;
        for (size_t i = 0; i < iniSections.size(); i++) {
            for (size_t j = 0; j < iniValues[i].size(); j++) {
                iniVal = &(iniValues[i][j]);

                iVal = getIniVal(iniVal->key);
                switch (iVal) {
                case INI_MINDURATION:
                    e->minDuration = iniVal->getInt();
                    break;
                case INI_MAXDURATION:
                    e->maxDuration = iniVal->getInt();
                    break;
                case INI_MINSIZE:
                    s = iniVal->getStr();
                    if (sscanf(&(s[0]), "%d,%d", &(e->minSizeS), &(e->minSizeE)) != 2) {
                        sscanf(&(s[0]), "%d", &(e->minSizeS));
                        e->minSizeE = e->minSizeS;
                    }
                    break;
                case INI_MAXSIZE:
                    s = iniVal->getStr();
                    if (sscanf(&(s[0]), "%d,%d", &(e->maxSizeS), &(e->maxSizeE)) != 2) {
                        sscanf(&(s[0]), "%d", &(e->maxSizeS));
                        e->maxSizeE = e->maxSizeS;
                    }
                    break;
                case INI_SPAWNTIME:
                    s = iniVal->getStr();
                    if (sscanf(&(s[0]), "%d,%d", &(e->spawnTimeS), &(e->spawnTimeE)) != 2) {
                        sscanf(&(s[0]), "%d", &(e->spawnTimeS));
                        e->spawnTimeE = e->spawnTimeS;
                    }
                    break;
                case INI_INITIALVEL:
                    s = iniVal->getStr();
                    sscanf(&(s[0]), "%f,%f,%f", &(e->initialVel.x), &(e->initialVel.y), &(e->initialVel.z));
                    break;
                case INI_TYPE:
                    s = iniVal->getStr();
                    if (s == "STATIC") {
                        e->type = EMITTER_STATIC;
                    } else if (s == "LINEAR") {
                        e->type = EMITTER_LINEAR;
                    } else {
                        e->type = EMITTER_DEFAULT;
                    }
                    break;
                case INI_PARTICLE:
                    e->particleType = getParticleType(iniVal->getStr());
                    break;
                case INI_POSOFFSET:
                    s = iniVal->getStr();
                    sscanf(&(s[0]), "%f,%f,%f", &(e->posOffset.x), &(e->posOffset.y), &(e->posOffset.z));
                    break;
                case INI_RANDPOSOFFSET:
                    s = iniVal->getStr();
                    sscanf(&(s[0]), "%f,%f,%f", &(e->rposOffset.x), &(e->rposOffset.y), &(e->rposOffset.z));
                    break;
                }
            }
        }
        _emitterMap.insert(make_pair(fileName, e));
        if (e->spawnTimeS <= 0) e->spawnTimeS = 1;
        if (e->spawnTimeE <= 0) e->spawnTimeE = 1;
        if (e->minSizeS > 255) e->minSizeS = 255;
        if (e->minSizeE > 255) e->minSizeE = 255;
        if (e->maxSizeS > 255) e->maxSizeS = 255;
        if (e->maxSizeE > 255) e->maxSizeE = 255;
        return e;
    }
    return NULL;
}
Animation *FileManager::loadAnimation(nString fileName, ZipFile *zipFile) {
    auto it = _animationMap.find(fileName);
    if (it != _animationMap.end()) {
        return it->second;
    } else {
        std::vector <std::vector <IniValue> > iniValues;
        std::vector <nString> iniSections;
        if (loadIniFile(fileName, iniValues, iniSections, zipFile)) return NULL;
        Animation *a = new Animation();
        nString s;
        int iVal;
        IniValue *iniVal;
        for (size_t i = 0; i < iniSections.size(); i++) {
            for (size_t j = 0; j < iniValues[i].size(); j++) {
                iniVal = &(iniValues[i][j]);

                iVal = getIniVal(iniVal->key);

                switch (iVal) {
                case INI_DURATION:
                    a->duration = iniVal->getInt();
                    break;
                case INI_REPETITIONS:
                    a->repetitions = iniVal->getInt();
                    break;
                case INI_FADEOUTBEGIN:
                    a->fadeOutBegin = iniVal->getInt();
                    break;
                case INI_TEXTURE:
                    a->texture = getTexture(iniVal->getStr());
                    break;
                case INI_XFRAMES:
                    a->xFrames = iniVal->getInt();
                    break;
                case INI_YFRAMES:
                    a->yFrames = iniVal->getInt();
                    break;
                }
            }
        }
        _animationMap.insert(make_pair(fileName, a));
        a->frames = a->xFrames*a->yFrames;
        return a;
    }
    return NULL;
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

#pragma region TODO: Marked For Removal

std::map <nString, INI_KEYS> IniMap;
std::map <nString, INI_KEYS> BlockIniMap;
std::map <nString, INI_KEYS> BiomeIniMap;

void FileManager::initializeINIMaps() {
    //       planet properties
    IniMap["x"] = INI_X;
    IniMap["y"] = INI_Y;
    IniMap["z"] = INI_Z;
    IniMap["r"] = INI_R;
    IniMap["g"] = INI_G;
    IniMap["b"] = INI_B;
    IniMap["density"] = INI_DENSITY;
    IniMap["radius"] = INI_RADIUS;
    IniMap["gravityConstant"] = INI_GRAVITYCONSTANT;
    IniMap["baseTemperature"] = INI_BASETEMPERATURE;
    IniMap["baseHumidity"] = INI_BASEHUMIDITY;
    IniMap["axialTilt"] = INI_AXIALTILT;
    IniMap["minCelsius"] = INI_MINCELSIUS;
    IniMap["maxCelsius"] = INI_MAXCELSIUS;
    IniMap["minHumidity"] = INI_MINHUMIDITY;
    IniMap["maxHumidity"] = INI_MAXHUMIDITY;

    //       atmosphere properties
    IniMap["m_Kr"] = INI_M_KR;
    IniMap["m_Km"] = INI_M_KM;
    IniMap["m_ESun"] = INI_M_ESUN;
    IniMap["m_g"] = INI_M_G;
    IniMap["m_exp"] = INI_M_EXP;
    IniMap["wavelengthR"] = INI_WAVELENGTHR;
    IniMap["wavelengthG"] = INI_WAVELENGTHG;
    IniMap["wavelengthB"] = INI_WAVELENGTHB;
    IniMap["nSamples"] = INI_NSAMPLES;

    //       options.ini
    IniMap["atmosphereSecColorExposure"] = INI_ATMOSPHERE_SEC_COLOR_EXPOSURE;
    IniMap["effectVolume"] = INI_EFFECT_VOLUME;
    IniMap["isBorderless"] = INI_ISBORDERLESS;
    IniMap["enableParticles"] = INI_ENABLE_PARTICLES;
    IniMap["fov"] = INI_FOV;
    IniMap["gamma"] = INI_GAMMA;
    IniMap["mouseSensitivity"] = INI_MOUSESENSITIVITY;
    IniMap["maxFps"] = INI_MAXFPS;
    IniMap["musicVolume"] = INI_MUSIC_VOLUME;
    IniMap["renderDistance"] = INI_RENDER_DISTANCE;
    IniMap["screenHeight"] = INI_SCREEN_HEIGHT;
    IniMap["screenWidth"] = INI_SCREEN_WIDTH;
    IniMap["terrainQuality"] = INI_TERRAIN_QUALITY;
    IniMap["fullScreen"] = INI_FULLSCREEN;
    IniMap["texturePack"] = INI_TEXTUREPACK;
    IniMap["vSync"] = INI_VSYNC;
    IniMap["motionBlur"] = INI_MOTIONBLUR;
    IniMap["msaa"] = INI_MSAA;

    //emitters
    IniMap["minDuration"] = INI_MINDURATION;
    IniMap["maxDuration"] = INI_MAXDURATION;
    IniMap["minSize"] = INI_MINSIZE;
    IniMap["maxSize"] = INI_MAXSIZE;
    IniMap["initialVel"] = INI_INITIALVEL;
    IniMap["spawnTime"] = INI_SPAWNTIME;
    IniMap["type"] = INI_TYPE;
    IniMap["particle"] = INI_PARTICLE;
    IniMap["posOffset"] = INI_POSOFFSET;
    IniMap["randPosOffset"] = INI_RANDPOSOFFSET;

    //animations
    IniMap["duration"] = INI_DURATION;
    IniMap["repetitions"] = INI_REPETITIONS;
    IniMap["fadeOutBegin"] = INI_FADEOUTBEGIN;
    IniMap["texture"] = INI_TEXTURE;
    IniMap["xFrames"] = INI_XFRAMES;
    IniMap["yFrames"] = INI_YFRAMES;

    //particles
    IniMap["animation"] = INI_ANIMATION;
    IniMap["color"] = INI_COLOR;

    //trees
    IniMap["branchChanceBottom"] = TREE_INI_BRANCHCHANCEBOTTOM;
    IniMap["branchChanceTop"] = TREE_INI_BRANCHCHANCETOP;
    IniMap["branchChanceCapMod"] = TREE_INI_BRANCHCHANCECAPMOD;
    IniMap["branchDirBottom"] = TREE_INI_BRANCHDIRECTIONBOTTOM;
    IniMap["branchDirTop"] = TREE_INI_BRANCHDIRTOP;
    IniMap["branchLengthBottom"] = TREE_INI_BRANCHLENGTHBOTTOM;
    IniMap["branchWidthBottom"] = TREE_INI_BRANCHWIDTHBOTTOM;
    IniMap["branchLeafShape"] = TREE_INI_BRANCHLEAFSHAPE;
    IniMap["branchLeafSizeMod"] = TREE_INI_BRANCHLEAFSIZEMOD;
    IniMap["branchLeafYMod"] = TREE_INI_BRANCHLEAFYMOD;
    IniMap["idCore"] = TREE_INI_IDCORE;
    IniMap["droopyLeavesLength"] = TREE_INI_DROOPYLEAVESLENGTH;
    IniMap["droopyLeavesSlope"] = TREE_INI_DROOPYLEAVESSLOPE;
    IniMap["droopyLeavesDSlope"] = TREE_INI_DROOPYLEAVESDSLOPE;
    IniMap["droopyLeavesActive"] = TREE_INI_DROOPYLEAVESACTIVE;
    IniMap["hasThickCapBranches"] = TREE_INI_HASTHICKCAPBRANCHES;
    IniMap["mushroomCapInverted"] = TREE_INI_MUSHROOMCAPINVERTED;
    IniMap["isSlopeRandom"] = TREE_INI_ISSLOPERANDOM;
    IniMap["leafCapShape"] = TREE_INI_LEAFCAPSHAPE;
    IniMap["leafCapSize"] = TREE_INI_LEAFCAPSIZE;
    IniMap["idLeaves"] = TREE_INI_IDLEAVES;
    IniMap["mushroomCapCurlLength"] = TREE_INI_MUSHROOMCAPCURLLENGTH;
    IniMap["mushroomCapGillThickness"] = TREE_INI_MUSHROOMCAPGILLTHICKNESS;
    IniMap["mushroomCapStretchMod"] = TREE_INI_MUSHROOMCAPSTRETCHMOD;
    IniMap["mushroomCapThickness"] = TREE_INI_MUSHROOMCAPTHICKNESS;
    IniMap["idBark"] = TREE_INI_IDBARK;
    IniMap["idRoot"] = TREE_INI_IDROOT;
    IniMap["idSpecialBlock"] = TREE_INI_IDSPECIALBLOCK;
    IniMap["branchDirectionTop"] = TREE_INI_BRANCHDIRECTIONTOP;
    IniMap["branchLengthTop"] = TREE_INI_BRANCHLENGTHTOP;
    IniMap["branchWidthTop"] = TREE_INI_BRANCHWIDTHTOP;
    IniMap["trunkHeightBase"] = TREE_INI_TRUNKHEIGHTBASE;
    IniMap["trunkWidthBase"] = TREE_INI_TRUNKWIDTHBASE;
    IniMap["trunkChangeDirChance"] = TREE_INI_TRUNKCHANGEDIRCHANCE;
    IniMap["trunkCoreWidth"] = TREE_INI_TRUNKCOREWIDTH;
    IniMap["trunkSlopeEnd"] = TREE_INI_TRUNKSLOPEEND;
    IniMap["trunkHeight"] = TREE_INI_TRUNKHEIGHT;
    IniMap["trunkWidthMid"] = TREE_INI_TRUNKWIDTHMID;
    IniMap["trunkWidthTop"] = TREE_INI_TRUNKWIDTHTOP;
    IniMap["trunkSlopeStart"] = TREE_INI_TRUNKSLOPESTART;
    IniMap["rootDepth"] = TREE_INI_ROOTDEPTH;

    // tex files
    IniMap["method"] = INI_METHOD;
    IniMap["innerSeams"] = INI_INNERSEAMS;
    IniMap["weights"] = INI_WEIGHTS;
    IniMap["symmetry"] = INI_SYMMETRY;
    IniMap["width"] = INI_WIDTH;
    IniMap["height"] = INI_HEIGHT;
    IniMap["pathOverlay"] = INI_PATH_OVERLAY;
    IniMap["pathOverlayTexture"] = INI_PATH_OVERLAY;
    IniMap["blendMode"] = INI_BLEND_MODE;
    IniMap["pathBaseTexture"] = INI_PATH_BASE_TEXTURE;
    IniMap["useMapColor"] = INI_USEMAPCOLOR;

    //        blockData.ini
    BlockIniMap["id"] = BLOCK_INI_ID;
    BlockIniMap["allowsLight"] = BLOCK_INI_ALLOWSLIGHT;
    BlockIniMap["blocksSunRays"] = BLOCK_INI_BLOCKSSUNRAYS;
    BlockIniMap["breaksByWater"] = BLOCK_INI_BREAKSBYWATER;
    BlockIniMap["burnTransformid"] = BLOCK_INI_BURNTRANSFORMID;
    BlockIniMap["collision"] = BLOCK_INI_COLLISION;
    BlockIniMap["crushable"] = BLOCK_INI_CRUSHABLE;
    BlockIniMap["explosionPower"] = BLOCK_INI_EXPLOSIONPOWER;
    BlockIniMap["explosionPowerLoss"] = BLOCK_INI_EXPLOSIONPOWERLOSS;
    BlockIniMap["explosionRays"] = BLOCK_INI_EXPLOSIONRAYS;
    BlockIniMap["explosiveResistance"] = BLOCK_INI_EXPLOSIVERESISTANCE;
    BlockIniMap["flammability"] = BLOCK_INI_FLAMMABILITY;
    BlockIniMap["floatingAction"] = BLOCK_INI_FLOATINGACTION;
    BlockIniMap["health"] = BLOCK_INI_HEALTH;
    BlockIniMap["lightActive"] = BLOCK_INI_LIGHTACTIVE;
    BlockIniMap["lightColor"] = BLOCK_INI_LIGHTCOLOR;
    BlockIniMap["material"] = BLOCK_INI_MATERIAL;
    BlockIniMap["meshType"] = BLOCK_INI_MESHTYPE;
    BlockIniMap["movementMod"] = BLOCK_INI_MOVEMENTMOD;
    BlockIniMap["movesPowder"] = BLOCK_INI_MOVESPOWDER;
    BlockIniMap["physicsProperty"] = BLOCK_INI_PHYSICSPROPERTY;
    BlockIniMap["supportive"] = BLOCK_INI_SUPPORTIVE;
    BlockIniMap["useable"] = BLOCK_INI_USEABLE;
    BlockIniMap["value"] = BLOCK_INI_VALUE;
    BlockIniMap["color"] = BLOCK_INI_COLOR;
    BlockIniMap["colorFilter"] = BLOCK_INI_COLORFILTER;
    BlockIniMap["overlayColor"] = BLOCK_INI_OVERLAYCOLOR;
    BlockIniMap["waterMeshLevel"] = BLOCK_INI_WATERMESHLEVEL;
    BlockIniMap["waveEffect"] = BLOCK_INI_WAVEEFFECT;
    BlockIniMap["weight"] = BLOCK_INI_WEIGHT;
    BlockIniMap["occlude"] = BLOCK_INI_OCCLUDE;
    BlockIniMap["source"] = BLOCK_INI_SOURCE;
    BlockIniMap["sink"] = BLOCK_INI_SINK;
    BlockIniMap["texture"] = BLOCK_INI_TEXTURE;
    BlockIniMap["textureTop"] = BLOCK_INI_TEXTURETOP;
    BlockIniMap["textureSide"] = BLOCK_INI_TEXTURESIDE;
    BlockIniMap["textureBottom"] = BLOCK_INI_TEXTUREBOTTOM;
    BlockIniMap["textureParticle"] = BLOCK_INI_TEXTURE_PARTICLE;
    BlockIniMap["emitter"] = BLOCK_INI_EMITTER;
    BlockIniMap["emitterOnBreak"] = BLOCK_INI_EMITTERONBREAK;
    BlockIniMap["emitterRandom"] = BLOCK_INI_EMITTERRANDOM;

    // biomes
    BiomeIniMap["none"] = BIOME_INI_NONE;
    BiomeIniMap["name"] = BIOME_INI_NAME;
    BiomeIniMap["isAltColorActive"] = BIOME_INI_ALTCOLORACTIVE;
    BiomeIniMap["altColor"] = BIOME_INI_ALTCOLOR;
    BiomeIniMap["maxHeight"] = BIOME_INI_MAXHEIGHT;
    BiomeIniMap["maxHeightTransitionLength"] = BIOME_INI_MAXHEIGHTTRANSITIONLENGTH;
    BiomeIniMap["beachBlock"] = BIOME_INI_BEACHBLOCK;
    BiomeIniMap["surfaceBlock"] = BIOME_INI_SURFACEBLOCK;
    BiomeIniMap["underwaterBlock"] = BIOME_INI_UNDERWATERBLOCK;
    BiomeIniMap["baseTreeChance"] = BIOME_INI_TREECHANCEBASE;
    BiomeIniMap["distributionNoise"] = BIOME_INI_DISTRIBUTIONNOISE;
}

INI_KEYS FileManager::getIniVal(nString &s) {
    auto it = IniMap.find(s);
    if (it == IniMap.end()) {
        return INI_NONE;
    } else {
        return it->second;
    }
}

INI_KEYS FileManager::getBlockIniVal(nString &s) {
    auto it = BlockIniMap.find(s);
    if (it == BlockIniMap.end()) {
        return BLOCK_INI_NONE;
    } else {
        return it->second;
    }
}

IniValue::IniValue(const nString &k, const nString &v) : isFloat(0) {
    key = k;
    val = v;
}
IniValue::IniValue(const nString &k, const double f) : isFloat(1) {
    key = k;
    fval = f;
}
bool IniValue::getBool() {
    int i;
    if (sscanf(val.c_str(), "%d", &i) != 1) {
        pError(("Error reading ini file. " + key + " expects bool value type. Value: " + val).c_str());
    }
    return (i != 0);
}
int IniValue::getInt() {
    int rv;
    if (sscanf(val.c_str(), "%d", &rv) != 1) {
        pError(("Error reading ini file. " + key + " expects integer value type. Value: " + val).c_str());
    }
    return rv;
}
float IniValue::getFloat() {
    float rv;
    if (sscanf(val.c_str(), "%f", &rv) != 1) {
        pError(("Error reading ini file. " + key + " expects float value type. Value: " + val).c_str());
    }
    return rv;
}
double IniValue::getDouble() {
    double rv;
    if (sscanf(val.c_str(), "%lf", &rv) != 1) {
        pError(("Error reading ini file. " + key + " expects double value type. Value: " + val).c_str());
    }
    return rv;
}
nString IniValue::getStr() {
    return val;
}
#pragma endregion

